#include "LightingPass.h"

#include "D3D11Manager.h"
#include "Light.h"
#include "RenderManager.h"
#include "RenderTarget.h"

struct CB_DirLight
{
	
};

HRESULT engine::LightingPass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	D3D11Manager* d3d11Manager = &D3D11Manager::GetInstance();
	d3d11Manager->CreateShader(L"..\\GameEngine\\resource\\Shader\\DirectionalLight.hlsl", m_DirLight);
	d3d11Manager->CreateShader(L"..\\GameEngine\\resource\\Shader\\DirectionalLight.hlsl", m_PointLight);

	return S_OK;
}

HRESULT engine::LightingPass::Render(ID3D11DeviceContext* context, void* data)
{
	LightPassData* passData = static_cast<LightPassData*>(data);

	RenderManager* renderManager = &RenderManager::GetInstance();
	const auto& diffuseMap = renderManager->FindRenderTarget("Target_Diffuse")->GetSRV();
	const auto& normalMap = renderManager->FindRenderTarget("Target_Normal")->GetSRV();
	const auto& depthMap = renderManager->FindRenderTarget("Target_Depth")->GetSRV();

	renderManager->BeginMRT("Light-Pass");

	for (const auto& light : *passData->Lights)
	{
		const auto& desc = light->GetLightDesc();
		
		switch (desc.Type)
		{
		case LightType_Directional:
		{
			m_DirLight->SetTexture("g_DiffuseMap", diffuseMap);
			m_DirLight->SetTexture("g_NormalMap", normalMap);
			m_DirLight->SetTexture("g_DepthMap", depthMap);

			m_DirLight->SetMatrix("g_ViewMatrix", passData->ViewMat);
			m_DirLight->SetMatrix("g_ProjMatrix", passData->ProjMat);
			m_DirLight->SetMatrix("g_ViewMatrixInverse", passData->InvViewMat);
			m_DirLight->SetMatrix("g_ProjMatrixInverse", passData->InvProjMat);

			light->Render(context, m_DirLight);
		}
		break;

		case LightType_Point:
		{
			m_PointLight->SetTexture("g_DiffuseMap", diffuseMap);
			m_PointLight->SetTexture("g_NormalMap", normalMap);
			m_PointLight->SetTexture("g_DepthMap", depthMap);

			m_PointLight->SetMatrix("g_ViewMatrix", passData->ViewMat);
			m_PointLight->SetMatrix("g_ProjMatrix", passData->ProjMat);
			m_PointLight->SetMatrix("g_ViewMatrixInverse", passData->InvViewMat);
			m_PointLight->SetMatrix("g_ProjMatrixInverse", passData->InvProjMat);

			light->Render(context, m_PointLight);
		}
		break;

		default:
			break;
		}
	}

	renderManager->EndMRT();


	return S_OK;
}

void engine::LightingPass::Release()
{

}
