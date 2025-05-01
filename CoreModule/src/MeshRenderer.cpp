#include "MeshRenderer.h"

#include "D3D11Manager.h"
#include "EditorComponentManager.h"
#include "LoadManager.h"
#include "Material.h"
#include "Mesh.h"

DEFINE_REGISTER_COMPONENT(MeshRenderer)

engine::MeshRenderer::MeshRenderer(const SharedPtr<GameObject>& owner, const _string& name)
	: Renderer(owner, name)
{

}

engine::MeshRenderer::~MeshRenderer() = default;

engine::MeshRenderer::MeshRenderer(const MeshRenderer& rhs)
	: Renderer(rhs), m_Mesh(rhs.m_Mesh)
{

}

void engine::MeshRenderer::Bind(const ComPtr<ID3D11DeviceContext>& context)
{
	if (m_Mesh)
	{
		m_Mesh->Bind(context);
	}
}

void engine::MeshRenderer::Render(const ComPtr<ID3D11DeviceContext>& context)
{
	{
		// TODO : 아래는 임시코드 임 지워야함.
		//=============================================================
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;           // Anisotropic 필터링 사용
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;         // U 좌표 랩 모드
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;         // V 좌표 랩 모드
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;         // W 좌표 랩 모드
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 16;                            // 최대 이방성 정도 (품질에 따라 조정)
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;      // 비교 함수 설정
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		ComPtr<ID3D11SamplerState> sampler;

		D3D11Manager::GetInstance().CreateSampler(samplerDesc, sampler);

		for (auto material : m_Material)
		{
			if (material.second->GetShader())
			{
				material.second->SetSampler("Sampler", sampler);
				//material.second->SetFloat4("DirLight_Dir", _float4(1.f, -1.f, 1.f, 0.f));
				//material.second->SetColor("DirLight_Diffuse", _float4(1.f, 1.f, 1.f, 1.f));
				material.second->SetColor("DirLight_Ambient", _float4(1.f, 1.f, 1.f, 1.f));
				material.second->SetColor("DirLight_Specular", _float4(1.f, 1.f, 1.f, 1.f));

				//material.second->SetColor("Ambient", _float4(0.8f, 0.8f, 0.8f, 0.8f));
				//material.second->SetColor("Specular", _float4(1.f, 1.f, 1.f, 1.f));

				material.second->SetMatrix("g_WorldMatrix", GetTransform()->GetWorldMatrix());
			}
		}
		//=============================================================

		Bind(context);

		auto& subMeshes = m_Mesh->GetSubMeshes();

		for (auto i = 0; i < subMeshes.size(); ++i)
		{
			if (m_Material[subMeshes[i].MaterialIndex]->GetShader())
			{
				m_Material[subMeshes[i].MaterialIndex]->Bind(context.Get());

				context->DrawIndexed(subMeshes[i].IndexCount, subMeshes[i].IndexOffset, 0);
			}
		}
	}
}

void engine::MeshRenderer::to_json(nlohmann::ordered_json& j)
{
	std::string type = "MeshRenderer";
	j = nlohmann::ordered_json{
		{"type", type},
		{"enable", m_bEnabled},
		{"materials", nlohmann::ordered_json::array() }
	};

	for (const auto& pair : m_Material)
	{
		nlohmann::ordered_json matJson;

		int index = pair.first;
		std::string path = WStringToString(pair.second->GetPath());
		j["materials"].push_back({ {"index", index}, {"path", path} });
	}
}

void engine::MeshRenderer::from_json(const nlohmann::ordered_json& j)
{
	if (j.contains("enable"))
	{
		j.at("enable").get_to(m_bEnabled);
	}

	for (const auto& matJson : j.at("materials"))
	{
		int index = matJson.at("index").get<_int>();
		_wstring path = StringToWString(matJson.at("path").get<_string>());

		SharedPtr<Material> material = Material::Create(shared_from_this());
		LoadManager::GetInstance().LoadMaterialData(material, path);

		SetMaterial(material, index);
	}
}

void engine::MeshRenderer::Destroy()
{
	m_bDestroyed = true;
}

void engine::MeshRenderer::registerComponent(ApplicationMode mode)
{
	Renderer::registerComponent(mode);

	//m_Material = Material::Create(shared_from_this());
}
