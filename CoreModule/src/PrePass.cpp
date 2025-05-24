#include "PrePass.h"

#include "D3D11Manager.h"
#include "EditorCore.h"
#include "Renderer.h"

HRESULT engine::PrePass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	D3D11Manager::GetInstance().CreateShader(L"..\\GameEngine\\resource\\Shader\\DepthOnly.hlsl", m_PrePassShader);
	D3D11Manager::GetInstance().CreateShader(L"..\\GameEngine\\resource\\Shader\\SkinningDepthOnly.hlsl", m_PrePassSkinnedShader);

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	device->CreateDepthStencilState(&dsDesc, m_PrePassDSState.ReleaseAndGetAddressOf());

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0;
	device->CreateBlendState(&blendDesc, m_PrePassBlendState.ReleaseAndGetAddressOf());

	DXGI_SWAP_CHAIN_DESC scDesc;
	D3D11Manager::GetInstance().GetSwapChain()->GetDesc(&scDesc);
	UINT w = scDesc.BufferDesc.Width;
	UINT h = scDesc.BufferDesc.Height;

	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = w;
	texDesc.Height = h;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;

	ComPtr<ID3D11Texture2D> dummyTex;
	HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, dummyTex.GetAddressOf());
	if (FAILED(hr))
	{
		return hr;
	}

	hr = device->CreateRenderTargetView(dummyTex.Get(), nullptr, m_DummyRTV.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

HRESULT engine::PrePass::Render(ID3D11DeviceContext* context, void* data)
{
	//if (!data)
	//{
	//	return E_INVALIDARG;
	//}

	//const PrePassData* passData = static_cast<PrePassData*>(data);

	//// 0. 이전 상태 보존 -----------------------------------------------
	//ComPtr<ID3D11DepthStencilState> prevState;
	//UINT ref;
	//ComPtr<ID3D11BlendState> prevBlendState;
	//_float prevBF[4];
	//UINT prevSampleMask;
	//context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);
	//context->OMGetBlendState(prevBlendState.ReleaseAndGetAddressOf(), prevBF, &prevSampleMask);

	//// 1. 상태 세팅 ----------------------------------------------------
	//ID3D11RenderTargetView* dummyRTV = m_DummyRTV.Get();
	//ID3D11DepthStencilView* dsv = D3D11Manager::GetInstance().GetDepthStencilView().Get();

	//context->OMSetRenderTargets(1, &dummyRTV, dsv);
	//context->OMSetDepthStencilState(m_PrePassDSState.Get(), 0);
	//context->OMSetBlendState(m_PrePassBlendState.Get(), nullptr, 0xffffffff);
	////D3D11Manager::GetInstance().SetCW();

	//// 2. PrePass Draw ----------------------------------------------
	//for (const auto& renderer : *passData->Renderers)
	//{
	//	if (const auto& owner = renderer->GetGameObject().lock())
	//	{
	//		if (owner->IsActive())
	//		{
	//			renderer->PreRender(context, passData->ViewMat, passData->ProjMat);
	//		}
	//	}
	//}

	//// 3. 상태 복원 --------------------------------------------------
	//ID3D11RenderTargetView* mainRTV = D3D11Manager::GetInstance().GetMainRTV().Get();
	//context->OMSetRenderTargets(1, &mainRTV, dsv);
	//context->OMSetDepthStencilState(prevState.Get(), ref);
	//context->OMSetBlendState(prevBlendState.Get(), prevBF, prevSampleMask);

	return S_OK;
}

HRESULT engine::PrePass::RenderEditor(ID3D11DeviceContext* context, void* data, _bool isGame)
{
	//if (!data)
	//{
	//	return E_INVALIDARG;
	//}

	//const PrePassData* passData = static_cast<PrePassData*>(data);  

	//// 0. 이전 상태 보존 -----------------------------------------------
	//ComPtr<ID3D11DepthStencilState> prevState;
	//UINT ref;
	//ComPtr<ID3D11BlendState> prevBlendState;
	//_float prevBF[4];
	//UINT prevSampleMask;
	//context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);
	//context->OMGetBlendState(prevBlendState.ReleaseAndGetAddressOf(), prevBF, &prevSampleMask);

	//if (!isGame)
	//{
	//	// 1. 상태 세팅 ----------------------------------------------------
	//	ID3D11RenderTargetView* dummyRTV = nullptr;
	//	ID3D11DepthStencilView* dsv = editor::EditorCore::GetInstance().GetSceneDSV().Get();

	//	context->OMSetRenderTargets(0, &dummyRTV, dsv);
	//	context->OMSetDepthStencilState(m_PrePassDSState.Get(), 0);
	//	context->OMSetBlendState(m_PrePassBlendState.Get(), nullptr, 0xffffffff);

	//	// 2. PrePass Draw ----------------------------------------------
	//	for (const auto& renderer : *passData->Renderers)
	//	{
	//		if (const auto& owner = renderer->GetGameObject().lock())
	//		{
	//			if (owner->IsActive())
	//			{
	//				renderer->PreRender(context, passData->ViewMat, passData->ProjMat);
	//			}
	//		}
	//	}

	//	// 3. 상태 복원 --------------------------------------------------
	//	ID3D11RenderTargetView* mainRTV = editor::EditorCore::GetInstance().GetSceneRTV().Get();
	//	context->OMSetRenderTargets(1, &mainRTV, dsv);
	//	context->OMSetDepthStencilState(prevState.Get(), ref);
	//	context->OMSetBlendState(prevBlendState.Get(), prevBF, prevSampleMask);
	//}

	//else
	//{
	//	// 1. 상태 세팅 ----------------------------------------------------
	//	ID3D11RenderTargetView* dummyRTV = nullptr;
	//	ID3D11DepthStencilView* dsv = editor::EditorCore::GetInstance().GetGameDSV().Get();

	//	context->OMSetRenderTargets(0, &dummyRTV, dsv);
	//	context->OMSetDepthStencilState(m_PrePassDSState.Get(), 0);
	//	context->OMSetBlendState(m_PrePassBlendState.Get(), nullptr, 0xffffffff);
	//	//D3D11Manager::GetInstance().SetCW();

	//	// 2. PrePass Draw ----------------------------------------------
	//	for (const auto& renderer : *passData->Renderers)
	//	{
	//		if (const auto& owner = renderer->GetGameObject().lock())
	//		{
	//			if (owner->IsActive())
	//			{
	//				renderer->PreRender(context, passData->ViewMat, passData->ProjMat);
	//			}
	//		}
	//	}

	//	// 3. 상태 복원 --------------------------------------------------
	//	ID3D11RenderTargetView* mainRTV = editor::EditorCore::GetInstance().GetGameRTV().Get();
	//	context->OMSetRenderTargets(1, &mainRTV, dsv);
	//	context->OMSetDepthStencilState(prevState.Get(), ref);
	//	context->OMSetBlendState(prevBlendState.Get(), prevBF, prevSampleMask);
	//}

	return S_OK;
}

void engine::PrePass::Release()
{
	m_PrePassShader.reset();
	m_PrePassSkinnedShader.reset();
	m_PrePassDSState.Reset();
	m_PrePassBlendState.Reset();
	m_DummyRTV.Reset();
}

void engine::PrePass::setMatrix(const _string& name, const _float4X4& value)
{
	for (_uint i = 0; i < ShaderTypeEnd; ++i)
	{
		for (auto& pair : m_PrePassShader->Reflects[i].CBuffers)
		{
			auto& cbDesc = pair.second;
			auto varIt = cbDesc.Variables.find(name);

			if (varIt != cbDesc.Variables.end())
			{
				auto& varInfo = varIt->second;
				auto& cbr = m_PrePassShader->CBuffers[i][cbDesc.Name];
				auto& dataVec = cbr->LocalData;

				if (varInfo.Size != sizeof(_float4X4))
				{
					std::cerr << "ERROR : Type mismatch in SetValue. (Class Material) " <<
						"Property : " << name.c_str() << ", Method : SetFloat4X4() \n";

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

		for (auto& pair : m_PrePassSkinnedShader->Reflects[i].CBuffers)
		{
			auto& cbDesc = pair.second;
			auto varIt = cbDesc.Variables.find(name);

			if (varIt != cbDesc.Variables.end())
			{
				auto& varInfo = varIt->second;
				auto& cbr = m_PrePassSkinnedShader->CBuffers[i][cbDesc.Name];
				auto& dataVec = cbr->LocalData;

				if (varInfo.Size != sizeof(_float4X4))
				{
					std::cerr << "ERROR : Type mismatch in SetValue. (Class Material) " <<
						"Property : " << name.c_str() << ", Method : SetFloat4X4() \n";

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
