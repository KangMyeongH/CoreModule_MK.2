#include "DeferredPass.h"

#include "D3D11Manager.h"
#include "RenderManager.h"
#include "RenderTarget.h"

HRESULT engine::DeferredPass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	
	D3D11Manager::GetInstance().CreateShader(L"..\\GameEngine\\resource\\Shader\\Deferred.hlsl", m_DeferredShader);
	
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	device->CreateDepthStencilState(&dsDesc, m_DeferredDSState.ReleaseAndGetAddressOf());

	return S_OK;
}

HRESULT engine::DeferredPass::Render(ID3D11DeviceContext* context, void* data)
{
	RenderManager* renderManager = &RenderManager::GetInstance();

	ComPtr<ID3D11DepthStencilState> prevState;
	UINT ref;
	context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);

	context->OMSetDepthStencilState(m_DeferredDSState.Get(), 0);

	const auto& shadeMap = renderManager->FindRenderTarget("Target_Shade")->GetSRV();
	const auto& specularMap = renderManager->FindRenderTarget("Target_Specular")->GetSRV();
	const auto& diffuseMap = renderManager->FindRenderTarget("Target_Diffuse")->GetSRV();
	const auto& outlineMap = renderManager->FindRenderTarget("Target_Outline")->GetSRV();

	m_DeferredShader->SetTexture("g_DiffuseMap", diffuseMap);
	m_DeferredShader->SetTexture("g_ShadeMap", shadeMap);
	m_DeferredShader->SetTexture("g_SpecularMap", specularMap);
	m_DeferredShader->SetTexture("g_OutlineMap", outlineMap);
	m_DeferredShader->Bind(context);
	context->Draw(3, 0);

	context->OMSetDepthStencilState(prevState.Get(), ref);

	return S_OK;
}

void engine::DeferredPass::Release()
{
}
