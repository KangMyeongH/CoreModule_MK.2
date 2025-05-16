#include "LightingPass.h"

#include "D3D11Manager.h"
#include "EditorCore.h"
#include "Light.h"
#include "RenderManager.h"
#include "RenderTarget.h"

HRESULT engine::LightingPass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	D3D11Manager* d3d11Manager = &D3D11Manager::GetInstance();
	d3d11Manager->CreateShader(L"..\\GameEngine\\resource\\Shader\\DirectionalLight.hlsl", m_DirLight);
	d3d11Manager->CreateShader(L"..\\GameEngine\\resource\\Shader\\DirectionalLight.hlsl", m_PointLight);

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = FALSE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	device->CreateDepthStencilState(&dsDesc, m_LightDSState.ReleaseAndGetAddressOf());

	D3D11_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	for (auto& i : blendDesc.RenderTarget)
	{
		i.BlendEnable = TRUE;
		i.SrcBlend = D3D11_BLEND_ONE;
		i.DestBlend = D3D11_BLEND_ONE;
		i.BlendOp = D3D11_BLEND_OP_ADD;
		i.SrcBlendAlpha = D3D11_BLEND_ONE;
		i.DestBlendAlpha = D3D11_BLEND_ZERO;
		i.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		i.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	device->CreateBlendState(&blendDesc, m_BlendState.ReleaseAndGetAddressOf());

	return S_OK;
}

HRESULT engine::LightingPass::Render(ID3D11DeviceContext* context, void* data)
{
	LightPassData* passData = static_cast<LightPassData*>(data);

	RenderManager* renderManager = &RenderManager::GetInstance();

	renderManager->BeginMRT("Light-Pass");

	ComPtr<ID3D11DepthStencilState> prevState;
	UINT ref;
	ComPtr<ID3D11BlendState> prevBlendState;
	_float prevBlendFactor[4];
	UINT prevSampleMask;

	_float blendFactor[4] = { 0.f, 0.f,0.f,0.f };
	UINT sampleMask = 0xFFFFFFFF;


	context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);
	context->OMGetBlendState(prevBlendState.ReleaseAndGetAddressOf(), prevBlendFactor, &prevSampleMask);

	context->OMSetDepthStencilState(m_LightDSState.Get(), 0);
	context->OMSetBlendState(m_BlendState.Get(), blendFactor, sampleMask);

	const auto& worldPosMap = renderManager->FindRenderTarget("Target_Position")->GetSRV();
	const auto& diffuseMap = renderManager->FindRenderTarget("Target_Diffuse")->GetSRV();
	const auto& normalMap = renderManager->FindRenderTarget("Target_Normal")->GetSRV();
	const auto& depthMap = renderManager->FindRenderTarget("Target_Depth")->GetSRV();

	for (const auto& light : *passData->Lights)
	{
		const auto& desc = light->GetLightDesc();
		
		switch (desc.Type)
		{
		case LightType_Directional:
		{
			m_DirLight->SetTexture("g_WorldPositionMap", worldPosMap);
			m_DirLight->SetTexture("g_DiffuseMap", diffuseMap);
			m_DirLight->SetTexture("g_NormalMap", normalMap);
			m_DirLight->SetTexture("g_DepthMap", depthMap);

			m_DirLight->SetMatrix("g_ViewMatrix", passData->ViewMat);
			m_DirLight->SetMatrix("g_ProjMatrix", passData->ProjMat);
			m_DirLight->SetMatrix("g_ViewMatrixInverse", passData->InvViewMat);
			m_DirLight->SetMatrix("g_ProjMatrixInverse", passData->InvProjMat);
			m_DirLight->SetFloat4("CameraPosition", passData->CameraPosition);

			light->Render(context, m_DirLight);
		}
		break;

		case LightType_Point:
		{
			m_PointLight->SetTexture("g_WorldPositionMap", worldPosMap);
			m_PointLight->SetTexture("g_DiffuseMap", diffuseMap);
			m_PointLight->SetTexture("g_NormalMap", normalMap);
			m_PointLight->SetTexture("g_DepthMap", depthMap);

			m_PointLight->SetMatrix("g_ViewMatrix", passData->ViewMat);
			m_PointLight->SetMatrix("g_ProjMatrix", passData->ProjMat);
			m_PointLight->SetMatrix("g_ViewMatrixInverse", passData->InvViewMat);
			m_PointLight->SetMatrix("g_ProjMatrixInverse", passData->InvProjMat);
			m_PointLight->SetFloat4("CameraPosition", passData->CameraPosition);
			
			light->Render(context, m_PointLight);
		}
		break;

		default:
			break;
		}
	}

	renderManager->EndMRT();

	context->OMSetDepthStencilState(prevState.Get(), ref);
	context->OMSetBlendState(prevBlendState.Get(), prevBlendFactor, prevSampleMask);

	return S_OK;
}

HRESULT engine::LightingPass::RenderEditor(ID3D11DeviceContext* context, void* data, _bool isGame)
{
	if (!isGame)
	{
		LightPassData* passData = static_cast<LightPassData*>(data);

		editor::EditorCore* editorCore = &editor::EditorCore::GetInstance();

		editorCore->BeginMRT("Light-Pass", isGame);

		ComPtr<ID3D11DepthStencilState> prevState;
		UINT ref;
		ComPtr<ID3D11BlendState> prevBlendState;
		_float prevBlendFactor[4];
		UINT prevSampleMask;

		_float blendFactor[4] = { 0.f, 0.f,0.f,0.f };
		UINT sampleMask = 0xFFFFFFFF;


		context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);
		context->OMGetBlendState(prevBlendState.ReleaseAndGetAddressOf(), prevBlendFactor, &prevSampleMask);

		context->OMSetDepthStencilState(m_LightDSState.Get(), 0);
		context->OMSetBlendState(m_BlendState.Get(), blendFactor, sampleMask);

		const auto& worldPosMap = editorCore->FindRenderTarget("Target_Position", isGame)->GetSRV();
		const auto& diffuseMap = editorCore->FindRenderTarget("Target_Diffuse", isGame)->GetSRV();
		const auto& normalMap = editorCore->FindRenderTarget("Target_Normal", isGame)->GetSRV();
		const auto& depthMap = editorCore->FindRenderTarget("Target_Depth", isGame)->GetSRV();

		for (const auto& light : *passData->Lights)
		{
			const auto& desc = light->GetLightDesc();

			switch (desc.Type)
			{
			case LightType_Directional:
			{
				m_DirLight->SetTexture("g_WorldPositionMap", worldPosMap);
				m_DirLight->SetTexture("g_DiffuseMap", diffuseMap);
				m_DirLight->SetTexture("g_NormalMap", normalMap);
				m_DirLight->SetTexture("g_DepthMap", depthMap);

				m_DirLight->SetMatrix("g_ViewMatrix", passData->ViewMat);
				m_DirLight->SetMatrix("g_ProjMatrix", passData->ProjMat);
				m_DirLight->SetMatrix("g_ViewMatrixInverse", passData->InvViewMat);
				m_DirLight->SetMatrix("g_ProjMatrixInverse", passData->InvProjMat);
				m_DirLight->SetFloat4("CameraPosition", passData->CameraPosition);

				light->Render(context, m_DirLight);
			}
			break;

			case LightType_Point:
			{
				m_PointLight->SetTexture("g_WorldPositionMap", worldPosMap);
				m_PointLight->SetTexture("g_DiffuseMap", diffuseMap);
				m_PointLight->SetTexture("g_NormalMap", normalMap);
				m_PointLight->SetTexture("g_DepthMap", depthMap);

				m_PointLight->SetMatrix("g_ViewMatrix", passData->ViewMat);
				m_PointLight->SetMatrix("g_ProjMatrix", passData->ProjMat);
				m_PointLight->SetMatrix("g_ViewMatrixInverse", passData->InvViewMat);
				m_PointLight->SetMatrix("g_ProjMatrixInverse", passData->InvProjMat);
				m_PointLight->SetFloat4("CameraPosition", passData->CameraPosition);

				light->Render(context, m_PointLight);
			}
			break;

			default:
				break;
			}
		}

		editorCore->EndMRT(isGame);

		context->OMSetDepthStencilState(prevState.Get(), ref);
		context->OMSetBlendState(prevBlendState.Get(), prevBlendFactor, prevSampleMask);

		return S_OK;
	}

	else
	{
		LightPassData* passData = static_cast<LightPassData*>(data);

		editor::EditorCore* editorCore = &editor::EditorCore::GetInstance();

		editorCore->BeginMRT("Light-Pass", isGame);

		ComPtr<ID3D11DepthStencilState> prevState;
		UINT ref;
		ComPtr<ID3D11BlendState> prevBlendState;
		_float prevBlendFactor[4];
		UINT prevSampleMask;

		_float blendFactor[4] = { 0.f, 0.f,0.f,0.f };
		UINT sampleMask = 0xFFFFFFFF;


		context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);
		context->OMGetBlendState(prevBlendState.ReleaseAndGetAddressOf(), prevBlendFactor, &prevSampleMask);

		context->OMSetDepthStencilState(m_LightDSState.Get(), 0);
		context->OMSetBlendState(m_BlendState.Get(), blendFactor, sampleMask);

		const auto& worldPosMap = editorCore->FindRenderTarget("Target_Position", isGame)->GetSRV();
		const auto& diffuseMap = editorCore->FindRenderTarget("Target_Diffuse", isGame)->GetSRV();
		const auto& normalMap = editorCore->FindRenderTarget("Target_Normal", isGame)->GetSRV();
		const auto& depthMap = editorCore->FindRenderTarget("Target_Depth", isGame)->GetSRV();

		for (const auto& light : *passData->Lights)
		{
			const auto& desc = light->GetLightDesc();

			switch (desc.Type)
			{
			case LightType_Directional:
			{
				m_DirLight->SetTexture("g_WorldPositionMap", worldPosMap);
				m_DirLight->SetTexture("g_DiffuseMap", diffuseMap);
				m_DirLight->SetTexture("g_NormalMap", normalMap);
				m_DirLight->SetTexture("g_DepthMap", depthMap);

				m_DirLight->SetMatrix("g_ViewMatrix", passData->ViewMat);
				m_DirLight->SetMatrix("g_ProjMatrix", passData->ProjMat);
				m_DirLight->SetMatrix("g_ViewMatrixInverse", passData->InvViewMat);
				m_DirLight->SetMatrix("g_ProjMatrixInverse", passData->InvProjMat);
				m_DirLight->SetFloat4("CameraPosition", passData->CameraPosition);

				light->Render(context, m_DirLight);
			}
			break;

			case LightType_Point:
			{
				m_PointLight->SetTexture("g_WorldPositionMap", worldPosMap);
				m_PointLight->SetTexture("g_DiffuseMap", diffuseMap);
				m_PointLight->SetTexture("g_NormalMap", normalMap);
				m_PointLight->SetTexture("g_DepthMap", depthMap);

				m_PointLight->SetMatrix("g_ViewMatrix", passData->ViewMat);
				m_PointLight->SetMatrix("g_ProjMatrix", passData->ProjMat);
				m_PointLight->SetMatrix("g_ViewMatrixInverse", passData->InvViewMat);
				m_PointLight->SetMatrix("g_ProjMatrixInverse", passData->InvProjMat);
				m_PointLight->SetFloat4("CameraPosition", passData->CameraPosition);

				light->Render(context, m_PointLight);
			}
			break;

			default:
				break;
			}
		}

		editorCore->EndMRT(isGame);

		context->OMSetDepthStencilState(prevState.Get(), ref);
		context->OMSetBlendState(prevBlendState.Get(), prevBlendFactor, prevSampleMask);

		return S_OK;
	}
}

void engine::LightingPass::Release()
{

}
