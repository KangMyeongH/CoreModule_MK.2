#pragma once
#include <iostream>

#include "core_enum.h"

namespace engine
{
	struct Ray
	{
		Vector3 Origin;			// Ray의 시작점 		(월드 좌표)
		Vector3 Direction;		// Ray의 방향 벡터	(정규화된 월드 벡터)

		Ray() : Origin(0, 0, 0), Direction(0, 0, 1) {}
		Ray(const Vector3& origin, const Vector3& direction) : Origin(origin), Direction(direction) {}

		Vector3 GetPoint(const _float distance) const
		{
			return Origin + Direction * distance;
		}
	};

	struct RayHit
	{
		Vector3 Point;			// 충돌 지점
		Vector3 Normal;		// 충돌 법선 ( 지금은 미 구현 )
		float Distance;		// ray origin으로부터 거리. 즉, ray 시작점으로 부터 거리
		//Collider* Collider; // 충돌된 collider

		RayHit() : Point(0,0,0), Normal(0,0,0), Distance(0.f) {}
	};

	struct VS_ConstantBuffer
	{
		_float4X4 WorldMat;
		_float4X4 ViewMat;
		_float4X4 ProjMat;
	};

	struct ShaderVarDesc
	{
		_string		Name;			// hlsl에 선언 된 변수 이름
		_uint		StartOffset;	// CBuffer 내 byte offset
		_uint		Size;			// byte size
	};

	struct ConstantBufferDesc
	{
		_string		Name;
		_uint		BindPoint;
		_uint		BufferSize;

		std::unordered_map<_string, ShaderVarDesc> Variables;
	};

	struct TextureInfo
	{
		_string		Name;
		_uint		BindPoint;
	};

	struct SamplerInfo
	{
		_string		Name;
		_uint		BindPoint;
	};

	struct ReflectResult
	{
		std::unordered_map<std::string, ConstantBufferDesc> CBuffers;
		std::unordered_map<std::string, TextureInfo>        Textures;
		std::unordered_map<std::string, SamplerInfo>        Samplers;
	};

	struct CBufferRuntime
	{
		ComPtr<ID3D11Buffer>	Buffer;
		std::vector<uint8_t>	LocalData;		// 매개 변수들
		UINT					BindPoint = 0;	// register
		UINT					Size = 0;		// Buffer Size
		_bool					DirtyFlag = true;
	};

	struct TextureRuntime
	{
		ComPtr<ID3D11ShaderResourceView> 	Texture;
		UINT								BindPoint = 0;
	};

	struct SamplerRuntime
	{
		ComPtr<ID3D11SamplerState>			Sampler;
		UINT								BindPoint = 0;
	};

	// CBufferRuntime의 LocalData. TextureRuntime, SamplerRuntime은
	// 각각의 객체가 고유한 값을 가지고 있어야함.
	// 그 외의 정보는 다 공유해서 가지고 있음.

	struct Shader
	{
		_wstring						Path;

		ComPtr<ID3D11VertexShader>		VertexShader;
		ComPtr<ID3D11PixelShader>		PixelShader;
		ComPtr<ID3D11DomainShader>		DomainShader;
		ComPtr<ID3D11ComputeShader>		ComputeShader;
		ComPtr<ID3D11GeometryShader>	GeometryShader;
		ComPtr<ID3D11HullShader>		HullShader;

		ComPtr<ID3D11InputLayout>		InputLayout;

		ReflectResult Reflects[ShaderTypeEnd];

		std::unordered_map<_string, SharedPtr<CBufferRuntime>> CBuffers[ShaderTypeEnd];
		std::unordered_map<_string, SharedPtr<TextureRuntime>> Textures[ShaderTypeEnd];
		std::unordered_map<_string, SharedPtr<SamplerRuntime>> Samplers[ShaderTypeEnd];

		Shader() = default;
		~Shader() = default;
		Shader(const Shader& rhs)
			: Path(rhs.Path),
			VertexShader(rhs.VertexShader),
			PixelShader(rhs.PixelShader),
			DomainShader(rhs.DomainShader),
			ComputeShader(rhs.ComputeShader),
			GeometryShader(rhs.GeometryShader),
			HullShader(rhs.HullShader),
			InputLayout(rhs.InputLayout)
		{
			if (this != &rhs)
			{
				std::copy(std::begin(rhs.Reflects), std::end(rhs.Reflects), std::begin(Reflects));
			}
		}

		void UploadConstantBuffers(ID3D11DeviceContext* context)
		{
			for (auto& cbMap : CBuffers)
			{
				for (auto& pair : cbMap)
				{
					const auto& cbr = pair.second;

					if (cbr->DirtyFlag)
					{
						context->UpdateSubresource(cbr->Buffer.Get(), 0, nullptr, cbr->LocalData.data(), 0, 0);
					}
				}
			}
		}

		void BindConstantBuffers(ID3D11DeviceContext* context) const
		{
			for (const auto& cb : CBuffers[VS])
			{
				context->VSSetConstantBuffers(cb.second->BindPoint, 1, cb.second->Buffer.GetAddressOf());
			}

			for (const auto& cb : CBuffers[HS])
			{
				context->HSSetConstantBuffers(cb.second->BindPoint, 1, cb.second->Buffer.GetAddressOf());
			}

			for (const auto& cb : CBuffers[DS])
			{
				context->DSSetConstantBuffers(cb.second->BindPoint, 1, cb.second->Buffer.GetAddressOf());
			}

			for (const auto& cb : CBuffers[GS])
			{
				context->GSSetConstantBuffers(cb.second->BindPoint, 1, cb.second->Buffer.GetAddressOf());
			}

			for (const auto& cb : CBuffers[PS])
			{
				context->PSSetConstantBuffers(cb.second->BindPoint, 1, cb.second->Buffer.GetAddressOf());
			}

			for (const auto& cb : CBuffers[CS])
			{
				context->CSSetConstantBuffers(cb.second->BindPoint, 1, cb.second->Buffer.GetAddressOf());
			}
		}

		void BindTextures(ID3D11DeviceContext* context) const
		{
			for (const auto& pair : Textures[VS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto srv = pair.second->Texture;

				context->VSSetShaderResources(bindPoint, 1, srv.GetAddressOf());
			}

			for (const auto& pair : Textures[HS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto srv = pair.second->Texture;

				context->HSSetShaderResources(bindPoint, 1, srv.GetAddressOf());
			}

			for (const auto& pair : Textures[DS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto srv = pair.second->Texture;

				context->DSSetShaderResources(bindPoint, 1, srv.GetAddressOf());
			}

			for (const auto& pair : Textures[GS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto srv = pair.second->Texture;

				context->GSSetShaderResources(bindPoint, 1, srv.GetAddressOf());
			}

			for (const auto& pair : Textures[PS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto srv = pair.second->Texture;

				context->PSSetShaderResources(bindPoint, 1, srv.GetAddressOf());
			}

			for (const auto& pair : Textures[CS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto srv = pair.second->Texture;

				context->CSSetShaderResources(bindPoint, 1, srv.GetAddressOf());
			}
		}

		void BindSamplers(ID3D11DeviceContext* context) const
		{
			for (const auto& pair : Samplers[VS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto sampler = pair.second->Sampler;

				context->VSSetSamplers(bindPoint, 1, sampler.GetAddressOf());
			}

			for (const auto& pair : Samplers[HS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto sampler = pair.second->Sampler;

				context->HSSetSamplers(bindPoint, 1, sampler.GetAddressOf());
			}

			for (const auto& pair : Samplers[DS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto sampler = pair.second->Sampler;

				context->DSSetSamplers(bindPoint, 1, sampler.GetAddressOf());
			}

			for (const auto& pair : Samplers[GS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto sampler = pair.second->Sampler;

				context->GSSetSamplers(bindPoint, 1, sampler.GetAddressOf());
			}

			for (const auto& pair : Samplers[PS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto sampler = pair.second->Sampler;

				context->PSSetSamplers(bindPoint, 1, sampler.GetAddressOf());
			}

			for (const auto& pair : Samplers[CS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto sampler = pair.second->Sampler;

				context->CSSetSamplers(bindPoint, 1, sampler.GetAddressOf());
			}
		}

		void Bind(ID3D11DeviceContext* context)
		{
			UploadConstantBuffers(context);

			context->IASetInputLayout(InputLayout.Get());

			context->VSSetShader(VertexShader.Get(), nullptr, 0);
			context->HSSetShader(HullShader.Get(), nullptr, 0);
			context->DSSetShader(DomainShader.Get(), nullptr, 0);
			context->GSSetShader(GeometryShader.Get(), nullptr,0);
			context->PSSetShader(PixelShader.Get(), nullptr, 0);
			context->CSSetShader(ComputeShader.Get(), nullptr, 0);

			BindConstantBuffers(context);
			BindTextures(context);
			BindSamplers(context);
		}

		SharedPtr<Shader> Clone(ID3D11Device* device)
		{
			SharedPtr<Shader> clone = std::make_shared<Shader>(*this);

			auto& cBuffers = clone->CBuffers;

			for (_uint i = 0; i < ShaderTypeEnd; ++i)
			{
				auto& reflectResult = Reflects[i];

				for (auto& pair : reflectResult.CBuffers)
				{
					const auto& cbName = pair.first;
					const auto& cbDesc = pair.second;

					SharedPtr<CBufferRuntime> cBufferRuntime = std::make_shared<CBufferRuntime>();
					cBufferRuntime->BindPoint = cbDesc.BindPoint;
					cBufferRuntime->Size = cbDesc.BufferSize;
					cBufferRuntime->LocalData.resize(cbDesc.BufferSize, 0);

					D3D11_BUFFER_DESC bd{};
					bd.ByteWidth = cbDesc.BufferSize;
					bd.Usage = D3D11_USAGE_DEFAULT; // 예: DEFAULT
					bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
					bd.CPUAccessFlags = 0;                   // D3D11_USAGE_DYNAMIC이면 D3D11_CPU_ACCESS_WRITE
					bd.MiscFlags = 0;
					bd.StructureByteStride = 0;

					HRESULT hr = device->CreateBuffer(&bd, nullptr, cBufferRuntime->Buffer.GetAddressOf());

					if (FAILED(hr))
					{
						std::cerr << "Failed to create Constant Buffer : " << cbName << "\n";
						return nullptr;
					}

					cBuffers[i].emplace(cbDesc.Name, cBufferRuntime);
				}
			}

			return clone;
		}
	};

	struct VTX_MESH
	{
		_float3 Position;
		_float3 Normal;
		_float2 TexCoord0;
	};
}
