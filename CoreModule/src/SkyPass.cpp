#include "SkyPass.h"

#include "D3D11Manager.h"

HRESULT engine::SkyPass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	m_SkySphere = SkySphere::Create();
	m_SkySphere->Initialize(device, context);

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	device->CreateDepthStencilState(&dsDesc, m_SkyPassDSState.ReleaseAndGetAddressOf());

	return S_OK;
}

HRESULT engine::SkyPass::Render(ID3D11DeviceContext* context, void* data)
{
	auto passData = static_cast<SkyPassData*>(data);

	ComPtr<ID3D11DepthStencilState> prevState;
	UINT ref;

	ID3D11RenderTargetView* mainRTV = D3D11Manager::GetInstance().GetMainRTV().Get();

	context->OMSetRenderTargets(1, &mainRTV, D3D11Manager::GetInstance().GetDepthStencilView().Get());
	context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);
	context->OMSetDepthStencilState(m_SkyPassDSState.Get(), 0);
	
	m_SkySphere->Render(context, passData->ViewMat, passData->ProjMat, passData->CameraPosition, passData->SunDir);

	context->OMSetDepthStencilState(prevState.Get(), ref);

	return S_OK;
}

void engine::SkyPass::Release()
{
	m_SkySphere.reset();
}
