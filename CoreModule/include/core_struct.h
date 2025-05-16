#pragma once
#include <iostream>

#include "core_enum.h"

namespace engine
{
	class Light;
	class Renderer;
	class Collider;
	class Transform;
}

namespace engine
{
	class Material;
}

namespace engine
{
	struct TriangleAABB;
	struct Contact;
	struct BVHNode;
	struct BVHNodeData;

	struct Contact
	{
		_bool IsHit = false;		// 충돌 여부
		Vector3 Normal{};				// A -> B 방향 노멀 (단위 벡터
		_float Penetration = 0.f;	// 침투 깊이 (>0) MTV = Normal * penetration;
		Vector3 PointA{};				// A 접촉점
		Vector3 PointB{};				// B 접촉점
	};

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

	struct ShaderVarDesc
	{
		_string		Name;			// hlsl에 선언 된 변수 이름
		_uint		StartOffset;	// CBuffer 내 byte offset
		_uint		Size;			// byte size
	};

	struct ConstantBufferDesc
	{
		_string		Name;			// CBuffer 이름
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
		std::wstring						TexturePath;
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
	// TODO : 최적화 해야함

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

		void SetFloat(const _string& name, const _float value)
		{
			for (_uint i = 0; i < ShaderTypeEnd; ++i)
			{
				for (const auto& pair : Reflects[i].CBuffers)
				{
					const auto& cbDesc = pair.second;
					auto varIt = cbDesc.Variables.find(name);

					if (varIt != cbDesc.Variables.end())
					{
						const auto& varInfo = varIt->second;
						const auto& cbr = CBuffers[i][cbDesc.Name];
						auto& dataVec = cbr->LocalData;

						if (varInfo.Size != sizeof(_float))
						{
							std::cerr << "ERROR : Type mismatch in SetValue." << "Property : " << name.c_str() << '\n';

							return;
						}

						if (dataVec.size() < cbDesc.BufferSize)
						{
							dataVec.resize(cbDesc.BufferSize, 0);
						}

						memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float));

						cbr->DirtyFlag = true;

						break;
					}
				}
			}
		}
		void SetFloat2(const _string& name, const _float2 value)
		{
			for (_uint i = 0; i < ShaderTypeEnd; ++i)
			{
				for (const auto& pair : Reflects[i].CBuffers)
				{
					const auto& cbDesc = pair.second;
					auto varIt = cbDesc.Variables.find(name);

					if (varIt != cbDesc.Variables.end())
					{
						const auto& varInfo = varIt->second;
						const auto& cbr = CBuffers[i][cbDesc.Name];
						auto& dataVec = cbr->LocalData;

						if (varInfo.Size != sizeof(_float2))
						{
							std::cerr << "ERROR : Type mismatch in SetValue." << "Property : " << name.c_str() << '\n';

							return;
						}

						if (dataVec.size() < cbDesc.BufferSize)
						{
							dataVec.resize(cbDesc.BufferSize, 0);
						}

						memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float2));

						cbr->DirtyFlag = true;

						break;
					}
				}
			}
		}
		void SetFloat3(const _string& name, const _float3 value)
		{
			for (_uint i = 0; i < ShaderTypeEnd; ++i)
			{
				for (const auto& pair : Reflects[i].CBuffers)
				{
					const auto& cbDesc = pair.second;
					auto varIt = cbDesc.Variables.find(name);

					if (varIt != cbDesc.Variables.end())
					{
						const auto& varInfo = varIt->second;
						const auto& cbr = CBuffers[i][cbDesc.Name];
						auto& dataVec = cbr->LocalData;

						if (varInfo.Size != sizeof(_float3))
						{
							std::cerr << "ERROR : Type mismatch in SetValue." << "Property : " << name.c_str() << '\n';

							return;
						}

						if (dataVec.size() < cbDesc.BufferSize)
						{
							dataVec.resize(cbDesc.BufferSize, 0);
						}

						memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float3));

						cbr->DirtyFlag = true;

						break;
					}
				}
			}
		}
		void SetFloat4(const _string& name, const _float4 value)
		{
			for (_uint i = 0; i < ShaderTypeEnd; ++i)
			{
				for (const auto& pair : Reflects[i].CBuffers)
				{
					auto& cbDesc = pair.second;
					auto varIt = cbDesc.Variables.find(name);

					if (varIt != cbDesc.Variables.end())
					{
						const auto& varInfo = varIt->second;
						const auto& cbr = CBuffers[i][cbDesc.Name];
						auto& dataVec = cbr->LocalData;

						if (varInfo.Size != sizeof(_float4))
						{
							std::cerr << "ERROR : Type mismatch in SetValue." << "Property : " << name.c_str() << '\n';

							return;
						}

						if (dataVec.size() < cbDesc.BufferSize)
						{
							dataVec.resize(cbDesc.BufferSize, 0);
						}

						memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float4));

						cbr->DirtyFlag = true;

						break;
					}
				}
			}
		}
		void SetMatrix(const _string& name, const _float4X4& value)
		{
			for (_uint i = 0; i < ShaderTypeEnd; ++i)
			{
				for (const auto& pair : Reflects[i].CBuffers)
				{
					const auto& cbDesc = pair.second;
					auto varIt = cbDesc.Variables.find(name);

					if (varIt != cbDesc.Variables.end())
					{
						const auto& varInfo = varIt->second;
						const auto& cbr = CBuffers[i][cbDesc.Name];
						auto& dataVec = cbr->LocalData;

						if (varInfo.Size != sizeof(_float4X4))
						{
							std::cerr << "ERROR : Type mismatch in SetValue." << "Property : " << name.c_str() << '\n';

							return;
						}


						if (dataVec.size() < cbDesc.BufferSize)
						{
							dataVec.resize(cbDesc.BufferSize, 0);
						}

						memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float4X4));

						cbr->DirtyFlag = true;

						break;
					}
				}
			}
		}
		void SetMatrix(const _string& name, const _matrix& matrix)
		{
			_float4X4 mat;
			DirectX::XMStoreFloat4x4(&mat, matrix);
			SetMatrix(name, mat);
		}
		void SetTexture(const _string& name, const ComPtr<ID3D11ShaderResourceView>& texture)
		{
			for (_uint i = 0; i < ShaderTypeEnd; ++i)
			{
				auto& texMap = Reflects[i].Textures;
				auto texIt = texMap.find(name);
				if (texIt != texMap.end())
				{
					SharedPtr<TextureRuntime> textureRuntime = std::make_shared<TextureRuntime>();

					textureRuntime->Texture = texture;
					textureRuntime->BindPoint = texIt->second.BindPoint;

					Textures[i][name] = textureRuntime;

					break;
				}
			}
		}
		void SetSampler(const _string& name, const ComPtr<ID3D11SamplerState>& sampler)
		{
			for (_uint i = 0; i < ShaderTypeEnd; ++i)
			{
				auto& texMap = Reflects[i].Samplers;
				auto texIt = texMap.find(name);
				if (texIt != texMap.end())
				{
					SharedPtr<SamplerRuntime> samplerRuntime(new SamplerRuntime);

					samplerRuntime->Sampler = sampler;
					samplerRuntime->BindPoint = texIt->second.BindPoint;

					Samplers[i][name] = samplerRuntime;

					break;
				}
			}
		}
		void SetValue(const std::vector<_float4X4>& value)
		{
			auto& cbr = CBuffers[VS]["Bones"];
			auto& dataVec = cbr->LocalData;

			if (dataVec.size() < sizeof(_float4X4) * value.size())
			{
				dataVec.resize(sizeof(_float4X4) * value.size(), 0);
			}

			memcpy(dataVec.data(), value.data(), sizeof(_float4X4) * value.size());

			cbr->DirtyFlag = true;
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
						D3D11_MAPPED_SUBRESOURCE mappedResource;
						if (SUCCEEDED(context->Map(cbr->Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
						{
							memcpy(mappedResource.pData, cbr->LocalData.data(), cbr->Size);
							context->Unmap(cbr->Buffer.Get(), 0);
							cbr->DirtyFlag = false;
						}
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

			//for (const auto& cb : CBuffers[HS])
			//{
			//	context->HSSetConstantBuffers(cb.second->BindPoint, 1, cb.second->Buffer.GetAddressOf());
			//}

			//for (const auto& cb : CBuffers[DS])
			//{
			//	context->DSSetConstantBuffers(cb.second->BindPoint, 1, cb.second->Buffer.GetAddressOf());
			//}

			//for (const auto& cb : CBuffers[GS])
			//{
			//	context->GSSetConstantBuffers(cb.second->BindPoint, 1, cb.second->Buffer.GetAddressOf());
			//}

			for (const auto& cb : CBuffers[PS])
			{
				context->PSSetConstantBuffers(cb.second->BindPoint, 1, cb.second->Buffer.GetAddressOf());
			}

			//for (const auto& cb : CBuffers[CS])
			//{
			//	context->CSSetConstantBuffers(cb.second->BindPoint, 1, cb.second->Buffer.GetAddressOf());
			//}
		}

		void BindTextures(ID3D11DeviceContext* context) const
		{
			for (const auto& pair : Textures[VS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto srv = pair.second->Texture;

				context->VSSetShaderResources(bindPoint, 1, srv.GetAddressOf());
			}

			//for (const auto& pair : Textures[HS])
			//{
			//	const auto& bindPoint = pair.second->BindPoint;
			//	auto srv = pair.second->Texture;

			//	context->HSSetShaderResources(bindPoint, 1, srv.GetAddressOf());
			//}

			//for (const auto& pair : Textures[DS])
			//{
			//	const auto& bindPoint = pair.second->BindPoint;
			//	auto srv = pair.second->Texture;

			//	context->DSSetShaderResources(bindPoint, 1, srv.GetAddressOf());
			//}

			//for (const auto& pair : Textures[GS])
			//{
			//	const auto& bindPoint = pair.second->BindPoint;
			//	auto srv = pair.second->Texture;

			//	context->GSSetShaderResources(bindPoint, 1, srv.GetAddressOf());
			//}

			for (const auto& pair : Textures[PS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto srv = pair.second->Texture;

				context->PSSetShaderResources(bindPoint, 1, srv.GetAddressOf());
			}

			//for (const auto& pair : Textures[CS])
			//{
			//	const auto& bindPoint = pair.second->BindPoint;
			//	auto srv = pair.second->Texture;

			//	context->CSSetShaderResources(bindPoint, 1, srv.GetAddressOf());
			//}
		}

		void BindSamplers(ID3D11DeviceContext* context) const
		{
			for (const auto& pair : Samplers[VS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto sampler = pair.second->Sampler;

				context->VSSetSamplers(bindPoint, 1, sampler.GetAddressOf());
			}

			//for (const auto& pair : Samplers[HS])
			//{
			//	const auto& bindPoint = pair.second->BindPoint;
			//	auto sampler = pair.second->Sampler;

			//	context->HSSetSamplers(bindPoint, 1, sampler.GetAddressOf());
			//}

			//for (const auto& pair : Samplers[DS])
			//{
			//	const auto& bindPoint = pair.second->BindPoint;
			//	auto sampler = pair.second->Sampler;

			//	context->DSSetSamplers(bindPoint, 1, sampler.GetAddressOf());
			//}

			//for (const auto& pair : Samplers[GS])
			//{
			//	const auto& bindPoint = pair.second->BindPoint;
			//	auto sampler = pair.second->Sampler;

			//	context->GSSetSamplers(bindPoint, 1, sampler.GetAddressOf());
			//}

			for (const auto& pair : Samplers[PS])
			{
				const auto& bindPoint = pair.second->BindPoint;
				auto sampler = pair.second->Sampler;

				context->PSSetSamplers(bindPoint, 1, sampler.GetAddressOf());
			}

			//for (const auto& pair : Samplers[CS])
			//{
			//	const auto& bindPoint = pair.second->BindPoint;
			//	auto sampler = pair.second->Sampler;

			//	context->CSSetSamplers(bindPoint, 1, sampler.GetAddressOf());
			//}
		}

		void Bind(ID3D11DeviceContext* context)
		{
			UploadConstantBuffers(context);

			context->IASetInputLayout(InputLayout.Get());

			context->VSSetShader(VertexShader.Get(), nullptr, 0);
			//context->HSSetShader(HullShader.Get(), nullptr, 0);
			//context->DSSetShader(DomainShader.Get(), nullptr, 0);
			//context->GSSetShader(GeometryShader.Get(), nullptr,0);
			context->PSSetShader(PixelShader.Get(), nullptr, 0);
			//context->CSSetShader(ComputeShader.Get(), nullptr, 0);

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
					bd.Usage = D3D11_USAGE_DYNAMIC; // 예: DEFAULT
					bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
					bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;                   // D3D11_USAGE_DYNAMIC이면 D3D11_CPU_ACCESS_WRITE
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

	struct SubMesh
	{
		_uint IndexOffset;	// 시작 인덱스
		_uint IndexCount;	// 인덱스 개수
		_uint MaterialIndex;
	};

	struct VIBuffer
	{
		ComPtr<ID3D11Buffer> VertexBuffer;
		ComPtr<ID3D11Buffer> IndexBuffer;
		
		_uint NumVertexBuffers;					// 사용되는 정점 버퍼의 개수
		_uint VertexStride;						// 하나의 정점 구조체 크기
		_uint NumVertices;						// 정점 개수
		_uint IndexStride;						// 인덱스 데이터의 크기 (바이트 단위, 16비트 = 2, 32비트 = 4)
		_uint NumIndices;						// 인덱스 개수
		DXGI_FORMAT IndexFormat;				// 인덱스 버퍼의 데이터 형식
		D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;	// 그리기 방식 ( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST )

		VIBuffer(): NumVertexBuffers(0), VertexStride(0), NumVertices(0), IndexStride(0), NumIndices(0), IndexFormat(),
		            PrimitiveTopology()
		{
		}
	};

	struct VTX_GRID
	{
		_float3 Position;
		_float4 Color;
	};

	struct VTX_TEXTURE_UI
	{
		_float3 Position;
		_float2 TexCoord0;
	};

	struct VTX_CUBE
	{
		_float3 Position;
		_float4 Color;
	};

	struct VTX_MESH
	{
		_float3 Position;
		_float3 Normal;
		_float2 TexCoord0;
		_float3 Tangent;
	};

	struct VTX_MESH_HASH
	{
		size_t operator()(const engine::VTX_MESH& v) const
		{
			using std::hash;

			size_t result = 0;
			auto h_float3 = [&](const DirectX::XMFLOAT3& f3)
				{
					size_t hx = std::hash<float>()(f3.x);
					size_t hy = std::hash<float>()(f3.y);
					size_t hz = std::hash<float>()(f3.z);
					return (hx ^ (hy + 0x9e3779b97f4a7c15ULL + (hx << 6) + (hx >> 2))) ^ hz;
				};

			result ^= h_float3(v.Position);
			result ^= h_float3(v.Normal);
			result ^= h_float3(v.Tangent);

			size_t huvx = std::hash<float>()(v.TexCoord0.x);
			size_t huvy = std::hash<float>()(v.TexCoord0.y);
			result ^= (huvx ^ (huvy + 0x9e3779b97f4a7c15ULL + (huvx << 6) + (huvx >> 2)));

			return result;
		}
	};

	struct VTX_SKY_SPHERE
	{
		_float3 Position;
		_float2 TexCoord;
	};

	struct VTX_MESH_EQUAL
	{
		bool operator()(const engine::VTX_MESH& a, const engine::VTX_MESH& b) const
		{
			if (a.Position.x != b.Position.x ||
				a.Position.y != b.Position.y ||
				a.Position.z != b.Position.z) return false;

			if (a.Normal.x != b.Normal.x ||
				a.Normal.y != b.Normal.y ||
				a.Normal.z != b.Normal.z)   return false;

			if (a.Tangent.x != b.Tangent.x ||
				a.Tangent.y != b.Tangent.y ||
				a.Tangent.z != b.Tangent.z)  return false;

			if (a.TexCoord0.x != b.TexCoord0.x ||
				a.TexCoord0.y != b.TexCoord0.y) return false;

			return true;
		}
	};

	struct VTX_SKINNED_MESH
	{
		_float3 			Position;
		_float3 			Normal;
		_float2 			TexCoord0;
		_float3 			Tangent;

		DirectX::XMUINT4 	BoneIndices;
		_float4 			BoneWeight;
	};

	struct VTX_SKINNED_MESH_HASH
	{
		size_t operator()(const VTX_SKINNED_MESH& v) const
		{
			using std::hash;

			auto hashCombine = [](size_t seed, size_t value) {
				return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
				};

			size_t h = 0;
			std::hash<float>   hFloat;
			std::hash<uint32_t> hUint;

			h = hashCombine(h, hFloat(v.Position.x));
			h = hashCombine(h, hFloat(v.Position.y));
			h = hashCombine(h, hFloat(v.Position.z));

			h = hashCombine(h, hFloat(v.Normal.x));
			h = hashCombine(h, hFloat(v.Normal.y));
			h = hashCombine(h, hFloat(v.Normal.z));

			h = hashCombine(h, hFloat(v.TexCoord0.x));
			h = hashCombine(h, hFloat(v.TexCoord0.y));

			h = hashCombine(h, hFloat(v.Tangent.x));
			h = hashCombine(h, hFloat(v.Tangent.y));
			h = hashCombine(h, hFloat(v.Tangent.z));

			h = hashCombine(h, hUint(v.BoneIndices.x));
			h = hashCombine(h, hUint(v.BoneIndices.y));
			h = hashCombine(h, hUint(v.BoneIndices.z));
			h = hashCombine(h, hUint(v.BoneIndices.w));

			h = hashCombine(h, hFloat(v.BoneWeight.x));
			h = hashCombine(h, hFloat(v.BoneWeight.y));
			h = hashCombine(h, hFloat(v.BoneWeight.z));
			h = hashCombine(h, hFloat(v.BoneWeight.w));

			return h;
		}
	};

	struct VTX_SKINNED_MESH_EQUAL
	{
		bool operator()(const VTX_SKINNED_MESH& a, const VTX_SKINNED_MESH& b) const
		{
			if (a.Position.x != b.Position.x ||
				a.Position.y != b.Position.y ||
				a.Position.z != b.Position.z) return false;

			if (a.Normal.x != b.Normal.x ||
				a.Normal.y != b.Normal.y ||
				a.Normal.z != b.Normal.z)   return false;

			if (a.Tangent.x != b.Tangent.x ||
				a.Tangent.y != b.Tangent.y ||
				a.Tangent.z != b.Tangent.z)  return false;

			if (a.TexCoord0.x != b.TexCoord0.x ||
				a.TexCoord0.y != b.TexCoord0.y) return false;

			if (a.BoneIndices.x != b.BoneIndices.x ||
				a.BoneIndices.y != b.BoneIndices.y ||
				a.BoneIndices.z != b.BoneIndices.z ||
				a.BoneIndices.w != b.BoneIndices.w) return false;

			if (a.BoneWeight.x != b.BoneWeight.x ||
				a.BoneWeight.y != b.BoneWeight.y ||
				a.BoneWeight.z != b.BoneWeight.z ||
				a.BoneWeight.w != b.BoneWeight.w) return false;

			return true;
		}
	};

	struct Bone
	{
		_string Name;
		_int 	ParentIndex;
		SharedPtr<Transform> Transform;
		_float4X4 Offset;
	};

	struct Skeleton
	{
		std::vector<Bone> Bones;

		SharedPtr<Transform> RootBone;

		std::unordered_map<_string, _int> BoneIndexMap;
	};

	struct Keyframe
	{
		double Time;

		Vector3 Translation;
		Quaternion Rotation;
		Vector3 Scale;
	};

	struct BoneKeyFrames
	{
		int BoneIndex;
		std::vector<Keyframe> Frames;
	};

	struct AnimationEvent
	{
		float Time;
		_string EventName;
		_bool	IsActive;
	};

	struct AnimationClip
	{
		_string Name;	// 애니메이션 이름
		_wstring Path;	// 애니메이션 경로
		double 	Duration; // 애니메이션 총 길이
		std::vector<BoneKeyFrames> Tracks;
		std::vector<AnimationEvent> Events;
	};

	struct AnimationState
	{
		std::string CurrentClip;
		_float CurrentTime = 0.0f;

		std::string NextClip;
		_float NextFadeDuration = 0.0f;
		_float NextIsLoop = false;

		_bool IsCrossFading = false;
		_bool IsLoop = false;
		_bool IsFinish = true;

		_string OldClip;
		_float OldClipTime = 0.0f;

		_float FadeTimer = 0.0f;
		_float FadeDuration = 0.0f;

		_string EventString = "";
	};

	//======================================//
	//				  binary				//
	//======================================//
	struct FileHeader
	{
		char magic[4];
		uint32_t version;

		uint32_t meshCount;
		uint32_t skeletonCount;
		uint32_t animationCount;
	};

	struct MaterialData
	{
		std::string MaterialName;
		int MaterialIndex;

		MaterialData() = default;
		MaterialData(const MaterialData& rhs) = default;
		MaterialData& operator=(const MaterialData& rhs) = default;
	};

	struct SubMeshData
	{
		std::string SubMeshName;

		int IndexOffset;
		int IndexCount;
		int MaterialIndex;

		SubMeshData() = default;
		SubMeshData(const SubMeshData& rhs) = default;
		SubMeshData& operator=(const SubMeshData& rhs) = default;
	};

	struct MeshInfo
	{
		uint32_t nameLength;

		uint32_t vertexCount;
		uint32_t indexCount;
		uint32_t materialCount;
		uint32_t subMeshCount;
	};

	struct SkinnedData
	{
		uint32_t BoneIndices[4];
		float BoneWeight[4];

		SkinnedData() = default;
		SkinnedData(const SkinnedData& rhs) = default;
		SkinnedData& operator=(const SkinnedData& rhs) = default;
	};

	struct BoneData
	{
		std::string BoneName;

		int Index;
		int parentIndex;

		float tx, ty, tz;
		float rx, ry, rz, rw;
		float sx, sy, sz;

		float offsetMatrix[16];

		BoneData() = default;
		BoneData(const BoneData& rhs) = default;
		BoneData& operator=(const BoneData& rhs) = default;
	};

	struct MeshData
	{
		std::string MeshName;
		bool IsSkinned;

		float tx, ty, tz;
		float rx, ry, rz, rw;
		float sx, sy, sz;

		std::vector<VTX_MESH> Vertices;
		std::vector<_uint> Indices;

		std::vector<MaterialData> Materials;
		std::vector<SubMeshData> SubMeshes;

		std::vector<SkinnedData> SkinnedData;
		std::vector<BoneData> Bones;

		std::vector<BVHNodeData> 	BVHNodes;
		std::vector<_int> 			BVHTriangles;

		// 아래의 데이터는 바이너리화 할 때 들어가면 안됨.
		std::unordered_map<_string, int> BoneMap;

		SharedPtr<VIBuffer> VIBuffer;

		MeshData() = default;
		MeshData(const MeshData& rhs) = default;
		MeshData& operator=(const MeshData& rhs) = default;
	};

	struct ModelData
	{
		std::string ModelName;

		std::vector<MeshData> Meshes;

		ModelData() = default;
		ModelData(const ModelData& rhs) = default;
		ModelData& operator=(const ModelData& rhs) = default;
	};

	struct MaterialInfo
	{
		uint32_t NameLength;
	};

	struct SubMeshInfo
	{
		uint32_t NameLength;

		uint32_t IndexOffset;
		uint32_t IndexCount;
		uint32_t MaterialIndex;
	};

	struct StaticVertex
	{
		float px, py, pz;	// position
		float nx, ny, nz;	// normal
		float u, v;			// texCoord(UV)
		float tx, ty, tz; 	// tangent
	};

	struct AABB
	{
		Vector3 Min;
		Vector3 Max;

		AABB()
		{
			const float inf = std::numeric_limits<float>::infinity();
			Min = Vector3{ inf,  inf,  inf };
			Max = Vector3{ -inf, -inf, -inf };
		}
		explicit AABB(const _float3& mn, const _float3& mx) : Min(mn), Max(mx)
		{
			
		}

		_bool Intersects(const AABB& other) const
		{
			return 	(Min.Value.x <= other.Max.Value.x && Max.Value.x >= other.Min.Value.x) &&
					(Min.Value.y <= other.Max.Value.y && Max.Value.y >= other.Min.Value.y) &&
					(Min.Value.z <= other.Max.Value.z && Max.Value.z >= other.Min.Value.z);
		}

		_float SurfaceArea() const
		{
			_float3 d{ Max.Value.x - Min.Value.x, Max.Value.y - Min.Value.y, Max.Value.z - Min.Value.z };
			return 2.f * (d.x * d.y + d.y * d.z + d.z * d.x);
		}

		AABB Union(const AABB& rhs) const
		{
			return AABB{
				{ (std::min)(Min.Value.x, rhs.Min.Value.x), (std::min)(Min.Value.y, rhs.Min.Value.y), (std::min)(Min.Value.z, rhs.Min.Value.z) },
				{ (std::max)(Max.Value.x, rhs.Max.Value.x), (std::max)(Max.Value.y, rhs.Max.Value.y), (std::max)(Max.Value.z, rhs.Max.Value.z) }
			};
		}

		void Expand(const _float3& p)
		{
			Min.Value.x = (p.x < Min.Value.x) ? p.x : Min.Value.x;  Max.Value.x = (p.x > Max.Value.x) ? p.x : Max.Value.x;
			Min.Value.y = (p.y < Min.Value.y) ? p.y : Min.Value.y;  Max.Value.y = (p.y > Max.Value.y) ? p.y : Max.Value.y;
			Min.Value.z = (p.z < Min.Value.z) ? p.z : Min.Value.z;  Max.Value.z = (p.z > Max.Value.z) ? p.z : Max.Value.z;
		}

		void Expand(const AABB& other)
		{
			Min.Value.x = std::min(Min.Value.x, other.Min.Value.x);
			Min.Value.y = std::min(Min.Value.y, other.Min.Value.y);
			Min.Value.z = std::min(Min.Value.z, other.Min.Value.z);

			Max.Value.x = std::max(Max.Value.x, other.Max.Value.x);
			Max.Value.y = std::max(Max.Value.y, other.Max.Value.y);
			Max.Value.z = std::max(Max.Value.z, other.Max.Value.z);
		}
	};

	struct OBB
	{
		Vector3 Center;
		Vector3 AxisX;
		Vector3 AxisY;
		Vector3 AxisZ;
		Vector3 Extents;
	};
	
	struct Sphere
	{
		Vector3 Center;
		_float	Radius = 0.5f;
	};

	struct Capsule
	{
		Vector3 CenterW;
		Vector3 AxisW;
		Vector3 P0W, P1W;
		_float HalfHeight;
		_float Radius;
	};

	//struct BVHNode
	//{
	//	AABB 		Box;			
	//	uint32_t 	First;   	// leaf : primitive 시작 인덱스		
	//	uint16_t 	Count;   	// leaf : 개수 + interior: 0
	//	int32_t		Right;   	// interior: 우측 자식 인덱스
	//};

	struct TriangleAABB
	{
		AABB 		Box;
		_float3 	Center;		// (v0 + v1 + v2) * 1/3
		_float3 	V0, V1, V2;
		uint32_t    TriIndex;
	};

	struct HitResult
	{
		float    T = std::numeric_limits<float>::max();
		uint32_t Tri = 0;
	};

	struct DebugVertex
	{
		_float3 Position;
	};

	struct CB_ViewProjMat
	{
		_float4X4 View;
		_float4X4 Proj;
	};

	struct CB_World
	{
		_float4X4 World;
	};

	struct CB_Color
	{
		_float4 Color;
	};

	struct LightDesc
	{
		LightType 	Type;
		_float4		Position;
		_float4		Dir;
		_float4 	Color;
		_float		Intensity;
		_float     	Range;
		_float     	SpotAngle;
	};

	struct BVHNode
	{
		AABB Bounds;
		_int Left;
		_int Right;
	};

	struct AABBData
	{
		_float3 Min;
		_float3 Max;

		void Expand(const _float3& p)
		{
			Min.x = (p.x < Min.x) ? p.x : Min.x;  Max.x = (p.x > Max.x) ? p.x : Max.x;
			Min.y = (p.y < Min.y) ? p.y : Min.y;  Max.y = (p.y > Max.y) ? p.y : Max.y;
			Min.z = (p.z < Min.z) ? p.z : Min.z;  Max.z = (p.z > Max.z) ? p.z : Max.z;
		}

		void Expand(const AABBData& other)
		{
			Min.x = std::min(Min.x, other.Min.x);
			Min.y = std::min(Min.y, other.Min.y);
			Min.z = std::min(Min.z, other.Min.z);

			Max.x = std::max(Max.x, other.Max.x);
			Max.y = std::max(Max.y, other.Max.y);
			Max.z = std::max(Max.z, other.Max.z);
		}
	};

	struct BVHNodeData
	{
		AABBData Bounds;
		int32_t Left;
		int32_t Right;
	};

	struct SkyPassData
	{
		_float4X4 ViewMat;
		_float4X4 ProjMat;
		Vector3 CameraPosition;
		_float3 SunDir;
	};

	struct PrePassData
	{
		std::vector<SharedPtr<Renderer>>* Renderers;
		_float4X4 ViewMat;
		_float4X4 ProjMat;
	};

	struct BasePassData
	{
		std::vector<SharedPtr<Renderer>>* Renderers;
		_float4X4 ViewMat;
		_float4X4 ProjMat;
		_float4 CameraPosition;
		_float4 NearFarPlane;
	};

	struct LightPassData
	{
		std::vector<SharedPtr<Light>>* Lights;

		_float4X4 	ViewMat;
		_float4X4 	ProjMat;
		_float4X4 	InvViewMat;
		_float4X4 	InvProjMat;
		_float4 	CameraPosition;
		_float4 	NearFarPlane;
	};

	struct ShadowPassData
	{
		_vector LightDir;
		_float3 CamPos;
		_float NearZ, FarZ;
		_matrix View;
		_matrix Proj;

	};

	struct CamData
	{
		_float4X4 ViewMat;
		_float4X4 ProjMat;
		_float4 NearFarPlane;
		_float4 Position;
	};
}
