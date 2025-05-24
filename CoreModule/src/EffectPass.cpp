#include "EffectPass.h"

#include "D3D11Manager.h"
#include "EditorCore.h"
#include "Effect.h"
#include "Material.h"
#include "RenderManager.h"

HRESULT engine::EffectPass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	// TODO : Effect Pass 용 Shader Loading


	// Create Depth Stencil State
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	device->CreateDepthStencilState(&dsDesc, m_PassDSState.ReleaseAndGetAddressOf());

	// Create Blend State
	D3D11_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	for (auto& i : blendDesc.RenderTarget)
	{
		i.BlendEnable = TRUE;
		i.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		i.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		i.BlendOp = D3D11_BLEND_OP_ADD;
		i.SrcBlendAlpha = D3D11_BLEND_ZERO;
		i.DestBlendAlpha = D3D11_BLEND_ONE;
		i.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		i.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	device->CreateBlendState(&blendDesc, m_PassBlendState.ReleaseAndGetAddressOf());

	// Create Rasterizer State
	D3D11_RASTERIZER_DESC rDesc = {};
	rDesc.FillMode = D3D11_FILL_SOLID;
	rDesc.CullMode = D3D11_CULL_NONE;
	rDesc.FrontCounterClockwise = FALSE;
	rDesc.DepthClipEnable = TRUE;

	device->CreateRasterizerState(&rDesc, m_RasterState.ReleaseAndGetAddressOf());

	// Create Sampler State
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP; // 가로 반복
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP; // 세로 반복
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, m_ClampSamplerState.ReleaseAndGetAddressOf());

	samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // 가로 반복
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; // 세로 반복
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, m_WrapSamplerState.ReleaseAndGetAddressOf());

	return S_OK;
}

HRESULT engine::EffectPass::Render(ID3D11DeviceContext* context, void* data)
{
	if (!data)
	{
		return E_INVALIDARG;
	}

	EffectPassData* passData = static_cast<EffectPassData*>(data);

	const auto& viewMat = passData->ViewMat;
	const auto& projMat = passData->ProjMat;
	_float4 vCamPos = _float4(passData->CamPos.x, passData->CamPos.y, passData->CamPos.z, 1.f);

	RenderManager* renderManager = &RenderManager::GetInstance();

	renderManager->BeginMRT("Effect-Pass");

	// 0. 이전 상태 보존 -----------------------------------------------
	ComPtr<ID3D11DepthStencilState> prevState;
	UINT ref;
	ComPtr<ID3D11BlendState> prevBlendState;
	_float prevBF[4];
	UINT prevSampleMask;
	context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);
	context->OMGetBlendState(prevBlendState.ReleaseAndGetAddressOf(), prevBF, &prevSampleMask);

	// 1. 상태 세팅 ----------------------------------------------------
	_float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
	UINT sampleMask = 0xffffffff;

	context->OMSetDepthStencilState(m_PassDSState.Get(), 0);
	context->OMSetBlendState(m_PassBlendState.Get(), blendFactor, sampleMask);
	context->RSSetState(m_RasterState.Get());

	// 2. 소팅 and Render! --------------------------------------------
	std::vector<EffectPassItem> queue;
	Vector3 camPos = Vector3(passData->CamPos);

	for (const auto& effect : *passData->Effects)
	{
		_float depth = calcDepth(effect->GetTransform()->Position(), camPos);
		queue.push_back({ effect, depth });
	}

	std::sort(queue.begin(), queue.end(), [](const EffectPassItem& a, const EffectPassItem& b) {return a.Depth > b.Depth; });

	for (const auto& item : queue)
	{
		if (item.Effect->IsEnabled())
		{
			if (const auto& material = item.Effect->GetMaterial())
			{
				if (item.Effect->IsClamp())
				{
					material->SetSampler("Sampler", m_ClampSamplerState);
				}

				else
				{
					material->SetSampler("Sampler", m_WrapSamplerState);
				}
				material->SetMatrix("g_ViewMatrix", viewMat);
				material->SetMatrix("g_ProjMatrix", projMat);

				item.Effect->InputAssembler(context);
				item.Effect->Bind(context);
				item.Effect->Render(context);
			}
		}
	}

	renderManager->EndMRT();

	// 3. 상태 복원 ------------------------------------------------------
	context->OMSetDepthStencilState(prevState.Get(), ref);
	context->OMSetBlendState(prevBlendState.Get(), prevBF, prevSampleMask);
	D3D11Manager::GetInstance().SetCW();

	return S_OK;
}

HRESULT engine::EffectPass::RenderEditor(ID3D11DeviceContext* context, void* data, _bool isGame)
{
	if (!data)
	{
		return E_INVALIDARG;
	}

	editor::EditorCore* editorCore = &editor::EditorCore::GetInstance();

	EffectPassData* passData = static_cast<EffectPassData*>(data);

	// 0. 이전 상태 보존 -----------------------------------------------
	ComPtr<ID3D11DepthStencilState> prevState;
	UINT ref;
	ComPtr<ID3D11BlendState> prevBlendState;
	_float prevBF[4];
	UINT prevSampleMask;
	context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);
	context->OMGetBlendState(prevBlendState.ReleaseAndGetAddressOf(), prevBF, &prevSampleMask);

	if (!isGame)
	{
		editorCore->BeginMRT("Effect-Pass", isGame);

		// 1. 상태 세팅 ----------------------------------------------------
		_float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		UINT sampleMask = 0xffffffff;

		context->OMSetDepthStencilState(m_PassDSState.Get(), 0);
		context->OMSetBlendState(m_PassBlendState.Get(), blendFactor, sampleMask);
		context->RSSetState(m_RasterState.Get());

		// 2. 소팅 and Render! --------------------------------------------
		std::vector<EffectPassItem> queue;
		Vector3 camPos = Vector3(passData->CamPos);

		for (const auto& effect : *passData->Effects)
		{
			_float depth = calcDepth(effect->GetTransform()->Position(), camPos);
			queue.push_back({ effect, depth });
		}

		std::sort(queue.begin(), queue.end(), [](const EffectPassItem& a, const EffectPassItem& b) {return a.Depth > b.Depth; });

		for (const auto& item : queue)
		{
			if (item.Effect->IsEnabled())
			{
				if (const auto& material = item.Effect->GetMaterial())
				{
					if (material->GetShader())
					{
						if (item.Effect->IsClamp())
						{
							material->SetSampler("Sampler", m_ClampSamplerState);
						}

						else
						{
							material->SetSampler("Sampler", m_WrapSamplerState);
						}
						material->SetMatrix("g_ViewMatrix", passData->ViewMat);
						material->SetMatrix("g_ProjMatrix", passData->ProjMat);
					}
				}
				item.Effect->InputAssembler(context);
				item.Effect->Bind(context);
				item.Effect->Render(context);
			}
		}

		editorCore->EndMRT(isGame);

		// 3. 상태 복원 ------------------------------------------------------
		context->OMSetDepthStencilState(prevState.Get(), ref);
		context->OMSetBlendState(prevBlendState.Get(), prevBF, prevSampleMask);
		D3D11Manager::GetInstance().SetCW();
	}

	else
	{
		editorCore->BeginMRT("Effect-Pass", isGame);
		
		// 1. 상태 세팅 ----------------------------------------------------
		_float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		UINT sampleMask = 0xffffffff;

		context->OMSetDepthStencilState(m_PassDSState.Get(), 0);
		context->OMSetBlendState(m_PassBlendState.Get(), blendFactor, sampleMask);
		context->RSSetState(m_RasterState.Get());

		// 2. 소팅 and Render! --------------------------------------------
		std::vector<EffectPassItem> queue;
		Vector3 camPos = Vector3(passData->CamPos);

		for (const auto& effect : *passData->Effects)
		{
			_float depth = calcDepth(effect->GetTransform()->Position(), camPos);
			queue.push_back({ effect, depth });
		}

		std::sort(queue.begin(), queue.end(), [](const EffectPassItem& a, const EffectPassItem& b) {return a.Depth > b.Depth; });

		for (const auto& item : queue)
		{
			if (item.Effect->IsEnabled())
			{
				if (const auto& material = item.Effect->GetMaterial())
				{
					if (material->GetShader())
					{
						if (item.Effect->IsClamp())
						{
							material->SetSampler("Sampler", m_ClampSamplerState);
						}

						else
						{
							material->SetSampler("Sampler", m_WrapSamplerState);
						}
						material->SetMatrix("g_ViewMatrix", passData->ViewMat);
						material->SetMatrix("g_ProjMatrix", passData->ProjMat);
					}
				}
				item.Effect->InputAssembler(context);
				item.Effect->Bind(context);
				item.Effect->Render(context);
			}
		}

		// 3. 상태 복원 ------------------------------------------------------
		editorCore->EndMRT(isGame);

		context->OMSetDepthStencilState(prevState.Get(), ref);
		context->OMSetBlendState(prevBlendState.Get(), prevBF, prevSampleMask);
		D3D11Manager::GetInstance().SetCW();
	}

	return S_OK;
}

void engine::EffectPass::Release()
{
}

engine::_float engine::EffectPass::calcDepth(const Vector3& worldPos, const Vector3& cameraPos)
{
	return (worldPos - cameraPos).SqrMagnitude();
}
