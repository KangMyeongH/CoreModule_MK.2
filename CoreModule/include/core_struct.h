#pragma once
#include <iostream>

#include "core_enum.h"

namespace engine
{
	class Transform;
}

namespace engine
{
	class Material;
}

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
						D3D11_MAPPED_SUBRESOURCE mappedResource;
						if (SUCCEEDED(context->Map(cbr->Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
						{
							memcpy(mappedResource.pData, cbr->LocalData.data(), cbr->Size);
							context->Unmap(cbr->Buffer.Get(), 0);
							cbr->DirtyFlag = false;
						}
						//context->UpdateSubresource(cbr->Buffer.Get(), 0, nullptr, cbr->LocalData.data(), 0, 0);
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

	struct AnimationClip
	{
		_string Name;
		double Duration;
		std::vector<BoneKeyFrames> Tracks;
	};

	//======================================//
	//				  binary				//
	//======================================//
#pragma pack(push,1)
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
		std::string name;
	};

	struct SubMeshData
	{
		std::string name;

		int indexOffset;
		int indexCount;
		int materialIndex;
	};

	struct MeshInfo
	{
		uint32_t nameLength;

		uint32_t vertexCount;
		uint32_t indexCount;
		uint32_t materialCount;
		uint32_t subMeshCount;
	};

	struct MeshData
	{
		std::string Name;

		std::vector<VTX_MESH> Vertices;
		std::vector<_uint> Indices;

		std::vector<MaterialData> Materials;
		std::vector<SubMeshData> SubMeshes;
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

	//struct FileHeader
	//{
	//	char magic[4];
	//	uint32_t version;

	//	uint32_t meshCount;
	//	uint32_t skeletonCount;
	//	uint32_t animationCount;
	//};

	//struct NodeData
	//{
	//	uint32_t nameLength;
	//	std::string nodeName;

	//	int nodeType;
	//	float localPos[3];
	//	float localRot[4];
	//	float localScale[3];

	//	int parentIndex;
	//	uint32_t childCount;
	//};

	//struct VertexData
	//{
	//	float px, py, pz;	// 
	//	float nx, ny, nz;	// normal
	//	float u, v;			// texCoord(UV)
	//	float tx, ty, tz; 	// tangent

	//	uint8_t boneIndices[4];
	//	uint8_t boneWeights[4];
	//};

	//struct MeshData
	//{
	//	uint32_t vertexCount;
	//	uint32_t indexCount;

	//	VertexData vertexes[];
	//};

	//struct MeshInfo
	//{
	//	uint32_t vertexCount;
	//	uint32_t indexCount;
	//};

	//struct BoneInfo
	//{
	//	int parentIndex;

	//	float tx, ty, tz;
	//	float rx, ry, rz, rw;
	//	float sx, sy, sz;

	//	float offsetMatrix[16];
	//};
#pragma pack(pop)
}
