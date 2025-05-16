#include "OutLinePass.h"

#include "D3D11Manager.h"
#include "RenderManager.h"
#include "RenderTarget.h"

HRESULT engine::OutLinePass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	D3D11Manager::GetInstance().CreateShader(L"..\\GameEngine\\resource\\Shader\\Outline.hlsl", m_Shader);

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = FALSE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	device->CreateDepthStencilState(&dsDesc, m_DepthStencilState.ReleaseAndGetAddressOf());

	D3D11_VIEWPORT vpDesc{};
	_uint numViewports = 1;

	context->RSGetViewports(&numViewports, &vpDesc);

	m_InvScreen = { 1 / vpDesc.Width, 1 / vpDesc.Height };
	_float depthScale = 0.1f;
	_float normalScale = 0.1f;
	_float threshold = 0.2f;
	m_Shader->SetFloat2("InvScreen", m_InvScreen);
	m_Shader->SetFloat("DepthScale", depthScale);
	m_Shader->SetFloat("NormalScale", normalScale);
	m_Shader->SetFloat("Threshold", threshold);

	return S_OK;
}

HRESULT engine::OutLinePass::Render(ID3D11DeviceContext* context, void* data)
{
	RenderManager* renderManager = &RenderManager::GetInstance();

	renderManager->BeginMRT("Outline-Pass");

	ComPtr<ID3D11DepthStencilState> prevState;
	UINT prevRef;

	context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &prevRef);
	context->OMSetDepthStencilState(m_DepthStencilState.Get(), 0);

	const auto& depthMap = renderManager->FindRenderTarget("Target_Depth")->GetSRV();
	const auto& normalMap = renderManager->FindRenderTarget("Target_Normal")->GetSRV();

	m_Shader->SetTexture("g_DepthMap", depthMap);
	m_Shader->SetTexture("g_NormalMap", normalMap);

	m_Shader->Bind(context);
	context->Draw(3, 0);

	renderManager->EndMRT();

	context->OMSetDepthStencilState(prevState.Get(), prevRef);
	return S_OK;
}

void engine::OutLinePass::Release()
{
}
