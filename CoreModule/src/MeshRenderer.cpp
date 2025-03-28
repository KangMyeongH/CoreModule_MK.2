#include "MeshRenderer.h"

#include "D3D11Manager.h"
#include "Material.h"
#include "Mesh.h"

DEFINE_REGISTER_COMPONENT(MeshRenderer)

engine::MeshRenderer::MeshRenderer(const SharedPtr<GameObject>& owner, const _string& name)
	: Renderer(owner, name)
{

}

engine::MeshRenderer::~MeshRenderer()
{
}

engine::MeshRenderer::MeshRenderer(const MeshRenderer& rhs)
	: Renderer(rhs)
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
			if (material->GetShader())
			{
				material->SetSampler("Sampler", sampler);
				material->SetFloat4("DirLight_Dir", _float4(1.f, -1.f, 1.f, 0.f));
				material->SetColor("DirLight_Diffuse", _float4(1.f, 1.f, 1.f, 1.f));
				material->SetColor("DirLight_Ambient", _float4(1.f, 1.f, 1.f, 1.f));
				material->SetColor("DirLight_Specular", _float4(1.f, 1.f, 1.f, 1.f));
				material->SetFloat4("CameraPosition", _float4(0.f, 10.f, -6.f, 1.f));

				material->SetColor("Ambient", _float4(0.3f, 0.3f, 0.3f, 0.3f));
				material->SetColor("Specular", _float4(1.f, 1.f, 1.f, 1.f));
				_float4X4 worldMat;
				XMStoreFloat4x4(&worldMat, XMMatrixTranspose(GetTransform()->GetWorldMatrix()));

				material->SetMatrix("g_WorldMatrix", worldMat);
			}
		}
		//=============================================================

		_float4X4 worldMat;
		XMStoreFloat4x4(&worldMat, XMMatrixTranspose(GetTransform()->GetWorldMatrix()));

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
}

void engine::MeshRenderer::from_json(const nlohmann::ordered_json& j)
{
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
