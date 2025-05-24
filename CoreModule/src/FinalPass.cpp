#include "FinalPass.h"

#include "D3D11Manager.h"
#include "EditorCore.h"
#include "RenderManager.h"
#include "RenderTarget.h"

HRESULT engine::FinalPass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	D3D11Manager::GetInstance().CreateShader(L"..\\GameEngine\\resource\\Shader\\FinalPass.hlsl", m_Shader);

	D3D11_DEPTH_STENCIL_DESC dsDesc{};
	dsDesc.DepthEnable = FALSE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	device->CreateDepthStencilState(&dsDesc, m_DSState.ReleaseAndGetAddressOf());

	return S_OK;
}

HRESULT engine::FinalPass::Render(ID3D11DeviceContext* context, void* data)
{
	ComPtr<ID3D11DepthStencilState> prevState;
	UINT ref;
	context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);

	context->OMSetDepthStencilState(m_DSState.Get(), 0);

	const auto& finalMap = RenderManager::GetInstance().FindRenderTarget("Target_FinalScene")->GetSRV();
	m_Shader->SetTexture("g_FinalMap", finalMap);
	m_Shader->Bind(context);
	context->Draw(3, 0);

	context->OMSetDepthStencilState(prevState.Get(), ref);

	return S_OK;
}

HRESULT engine::FinalPass::RenderEditor(ID3D11DeviceContext* context, void* data, _bool isGame)
{
	editor::EditorCore* editorCore = &editor::EditorCore::GetInstance();

	if (!isGame)
	{
		ComPtr<ID3D11DepthStencilState> prevState;
		UINT ref;
		context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);

		context->OMSetDepthStencilState(m_DSState.Get(), 0);
		context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		const auto& finalMap = editorCore->FindRenderTarget("Target_FinalScene", isGame)->GetSRV();
		m_Shader->SetTexture("g_FinalMap", finalMap);
		m_Shader->Bind(context);
		context->Draw(3, 0);

		context->OMSetDepthStencilState(prevState.Get(), ref);
	}

	else
	{
		ComPtr<ID3D11DepthStencilState> prevState;
		UINT ref;
		context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);

		context->OMSetDepthStencilState(m_DSState.Get(), 0);
		context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		const auto& finalMap = editorCore->FindRenderTarget("Target_FinalScene", isGame)->GetSRV();
		m_Shader->SetTexture("g_FinalMap", finalMap);
		m_Shader->Bind(context);
		context->Draw(3, 0);

		context->OMSetDepthStencilState(prevState.Get(), ref);
	}

	return S_OK;
}

void engine::FinalPass::Release()
{
}
