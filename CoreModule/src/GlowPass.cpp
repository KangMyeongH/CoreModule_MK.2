#include "GlowPass.h"

#include "D3D11Manager.h"
#include "EditorCore.h"
#include "RenderManager.h"
#include "RenderTarget.h"

HRESULT engine::GlowPass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	D3D11Manager* d3d11Manager = &D3D11Manager::GetInstance();

	// depth stencil state
	D3D11_DEPTH_STENCIL_DESC dsDesc{};
	dsDesc.DepthEnable = FALSE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	device->CreateDepthStencilState(&dsDesc, m_DSState.ReleaseAndGetAddressOf());

	// additive blendState
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;   // RTV 0번만 씀

	auto& rt = blendDesc.RenderTarget[0];
	rt.BlendEnable = TRUE;

	// -------- 색(RGB) 합산 --------
	rt.SrcBlend = D3D11_BLEND_ONE;   // 원본 * 1
	rt.DestBlend = D3D11_BLEND_ONE;   // 대상 * 1
	rt.BlendOp = D3D11_BLEND_OP_ADD;

	// -------- 알파 채널 --------
	rt.SrcBlendAlpha = D3D11_BLEND_ZERO;  // 선택 ①: 알파 유지
	rt.DestBlendAlpha = D3D11_BLEND_ONE;   //   (원본 알파 무시)
	rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;

	rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	device->CreateBlendState(&blendDesc, m_AddBlend.ReleaseAndGetAddressOf());

	// linear clamp sampler
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;      // Linear filtering
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;         // U 좌표 Clamp
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;         // V 좌표 Clamp
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;         // W 좌표 Clamp
	samplerDesc.MipLODBias = 0.0f;                                // LOD bias 없음
	samplerDesc.MaxAnisotropy = 1;                                   // 비(非) 이방성 필터링
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;             // 비교 함수 없음
	samplerDesc.BorderColor[0] = 0;                                   // Border 색 (Clamp이므로 사용 안 됨)
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;                                   // LOD 최소
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;                  // LOD 최대

	device->CreateSamplerState(&samplerDesc, m_LinearClamp.ReleaseAndGetAddressOf());

	// 4x4 down shader
	d3d11Manager->CreateShader(L"..\\GameEngine\\resource\\Shader\\Down4x4.hlsl", m_4x4Down);

	// 6x6 down shader
	d3d11Manager->CreateShader(L"..\\GameEngine\\resource\\Shader\\Down6x6.hlsl", m_6x6Down);

	// 6x6 up shader
	d3d11Manager->CreateShader(L"..\\GameEngine\\resource\\Shader\\Down6x6.hlsl", m_6x6Up);

	// blurH, V Shader
	d3d11Manager->CreateShader(L"..\\GameEngine\\resource\\Shader\\BlurH.hlsl", m_BlurH);
	d3d11Manager->CreateShader(L"..\\GameEngine\\resource\\Shader\\BlurV.hlsl", m_BlurV);

	// Composite
	d3d11Manager->CreateShader(L"..\\GameEngine\\resource\\Shader\\Composite.hlsl", m_Composite);

	// Final
	d3d11Manager->CreateShader(L"..\\GameEngine\\resource\\Shader\\BloomFinal.hlsl", m_Final);

	return S_OK;
}

HRESULT engine::GlowPass::Render(ID3D11DeviceContext* context, void* data)
{
	RenderManager* renderManager = &RenderManager::GetInstance();

	const auto& l1_4X4RT = renderManager->FindRenderTarget("Target_L1_4X4"); // /4
	const auto& l2_6X6RT = renderManager->FindRenderTarget("Target_L2_6X6"); // /24
	const auto& l3_6X6RT = renderManager->FindRenderTarget("Target_L3_6X6"); // /144
	const auto& l3_tmp = renderManager->FindRenderTarget("Target_L3_Tmp"); // /144
	const auto& l2_tmp = renderManager->FindRenderTarget("Target_L2_Tmp"); // /24
	const auto& l1_tmp = renderManager->FindRenderTarget("Target_L1_Tmp");
	const auto& bloom = renderManager->FindRenderTarget("Target_Bloom");

	const auto& glowMap = renderManager->FindRenderTarget("Target_GlowMap")->GetSRV();

	const auto& finalMap = renderManager->FindRenderTarget("Target_FinalScene");

	D3D11_TEXTURE2D_DESC texture2DDesc{};
	finalMap->GetTexture2D()->GetDesc(&texture2DDesc);
	initVP(texture2DDesc.Width, texture2DDesc.Height);

	const _float blendFactor[4] = { 0.f,0.f,0.f,0.f };

	_float2 l1TexelSize = { 1.f / m_VP[L1].Width, 1.f / m_VP[L1].Height };
	_float2 l2TexelSize = { 1.f / m_VP[L2].Width, 1.f / m_VP[L2].Height };
	_float2 l3TexelSize = { 1.f / m_VP[L3].Width, 1.f / m_VP[L3].Height };

	// 1. 4x4 down sample ( 4x4 )
	context->OMSetRenderTargets(1, l1_4X4RT->GetRTV().GetAddressOf(), nullptr);
	m_4x4Down->SetSampler("Sampler", m_LinearClamp);
	m_4x4Down->SetTexture("SrcTexture", glowMap);
	m_4x4Down->Bind(context);
	context->RSSetViewports(1, &m_VP[L1]);
	context->Draw(3, 0);

	// 2. 6x6 down sample ( 24x24 )
	context->OMSetRenderTargets(1, l2_6X6RT->GetRTV().GetAddressOf(), nullptr);
	m_6x6Down->SetSampler("Sampler", m_LinearClamp);
	m_6x6Down->SetTexture("SrcTexture", l1_4X4RT->GetSRV());
	m_6x6Down->Bind(context);
	context->RSSetViewports(1, &m_VP[L2]);
	context->Draw(3, 0);

	// 3. 6x6 down sample ( 144x144 )
	context->OMSetRenderTargets(1, l3_6X6RT->GetRTV().GetAddressOf(), nullptr);
	m_6x6Down->SetSampler("Sampler", m_LinearClamp);
	m_6x6Down->SetTexture("SrcTexture", l2_6X6RT->GetSRV());
	m_6x6Down->Bind(context);
	context->RSSetViewports(1, &m_VP[L3]);
	context->Draw(3, 0);

	// 4. gaussian blur ( 144x144 )
	// blur h, blur v
	context->OMSetRenderTargets(1, l3_tmp->GetRTV().GetAddressOf(), nullptr);
	m_BlurH->SetSampler("Sampler", m_LinearClamp);
	m_BlurH->SetTexture("SrcTexture", l3_6X6RT->GetSRV());
	m_BlurH->SetFloat2("TexelSize", l3TexelSize);
	m_BlurH->Bind(context);
	context->RSSetViewports(1, &m_VP[L3]);
	context->Draw(3, 0);

	context->OMSetRenderTargets(1, l3_6X6RT->GetRTV().GetAddressOf(), nullptr);
	m_BlurV->SetSampler("Sampler", m_LinearClamp);
	m_BlurV->SetTexture("SrcTexture", l3_tmp->GetSRV());
	m_BlurV->SetFloat2("TexelSize", l3TexelSize);
	m_BlurV->Bind(context);
	context->RSSetViewports(1, &m_VP[L3]);
	context->Draw(3, 0);

	// 5. 6x6 up + add ( 24x24 + blur )
	// EnableAddBlend(); TODO : 함수, BlendState 추가 해야됨
	context->OMSetBlendState(m_AddBlend.Get(), blendFactor, 0xffffffff);
	context->OMSetRenderTargets(1, l2_6X6RT->GetRTV().GetAddressOf(), nullptr);
	m_6x6Up->SetSampler("Sampler", m_LinearClamp);
	m_6x6Up->SetTexture("SrcTexture", l3_6X6RT->GetSRV());
	m_6x6Up->Bind(context);
	context->RSSetViewports(1, &m_VP[L2]);
	context->Draw(3, 0);
	context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);

	// 6. gaussian blur ( 24x24 )
	// blur h, blur v
	context->OMSetRenderTargets(1, l2_tmp->GetRTV().GetAddressOf(), nullptr);
	m_BlurH->SetSampler("Sampler", m_LinearClamp);
	m_BlurH->SetTexture("SrcTexture", l2_6X6RT->GetSRV());
	m_BlurH->SetFloat2("TexelSize", l2TexelSize);
	m_BlurH->Bind(context);
	context->RSSetViewports(1, &m_VP[L2]);
	context->Draw(3, 0);

	context->OMSetRenderTargets(1, l2_6X6RT->GetRTV().GetAddressOf(), nullptr);
	m_BlurV->SetSampler("Sampler", m_LinearClamp);
	m_BlurV->SetTexture("SrcTexture", l2_tmp->GetSRV());
	m_BlurV->SetFloat2("TexelSize", l2TexelSize);
	m_BlurV->Bind(context);
	context->RSSetViewports(1, &m_VP[L2]);
	context->Draw(3, 0);

	// 7. 6x6 up + add
	context->OMSetBlendState(m_AddBlend.Get(), blendFactor, 0xffffffff);
	context->OMSetRenderTargets(1, l1_4X4RT->GetRTV().GetAddressOf(), nullptr);
	m_6x6Up->SetSampler("Sampler", m_LinearClamp);
	m_6x6Up->SetTexture("SrcTexture", l2_6X6RT->GetSRV());
	m_6x6Up->Bind(context);
	context->RSSetViewports(1, &m_VP[L1]);
	context->Draw(3, 0);
	context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);

	// 8. gaussian blur
		// blur h, blur v
	context->OMSetRenderTargets(1, l1_tmp->GetRTV().GetAddressOf(), nullptr);
	m_BlurH->SetSampler("Sampler", m_LinearClamp);
	m_BlurH->SetTexture("SrcTexture", l1_4X4RT->GetSRV());
	m_BlurH->SetFloat2("TexelSize", l1TexelSize);
	m_BlurH->Bind(context);
	context->RSSetViewports(1, &m_VP[L1]);
	context->Draw(3, 0);

	context->OMSetRenderTargets(1, l1_4X4RT->GetRTV().GetAddressOf(), nullptr);
	m_BlurV->SetSampler("Sampler", m_LinearClamp);
	m_BlurV->SetTexture("SrcTexture", l1_tmp->GetSRV());
	m_BlurV->SetFloat2("TexelSize", l1TexelSize);
	m_BlurV->Bind(context);
	context->RSSetViewports(1, &m_VP[L1]);
	context->Draw(3, 0);

	// 9. Composite ( scene +  4x4 up scale bloom * intensity
	context->OMSetRenderTargets(1, bloom->GetRTV().GetAddressOf(), nullptr);
	m_Composite->SetSampler("Sampler", m_LinearClamp);
	m_Composite->SetTexture("SceneTexture", finalMap->GetSRV());
	m_Composite->SetTexture("BloomTexture", l1_4X4RT->GetSRV());
	m_Composite->Bind(context);
	context->RSSetViewports(1, &m_VP[L0]);
	context->Draw(3, 0);

	renderManager->BeginMRT("Final-Scene");

	ComPtr<ID3D11DepthStencilState> prevState;
	UINT ref;
	context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);

	context->OMSetDepthStencilState(m_DSState.Get(), 0);
	m_Final->SetSampler("Sampler", m_LinearClamp);
	m_Final->SetTexture("SrcTexture", bloom->GetSRV());
	m_Final->Bind(context);
	context->Draw(3, 0);

	renderManager->EndMRT();

	context->OMSetDepthStencilState(prevState.Get(), ref);

	return S_OK;
}

HRESULT engine::GlowPass::RenderEditor(ID3D11DeviceContext* context, void* data, _bool isGame)
{
	editor::EditorCore* editorCore = &editor::EditorCore::GetInstance();

	if (!isGame)
	{
		const auto& l1_4X4RT = editorCore->FindRenderTarget("Target_L1_4X4", isGame); // /4
		const auto& l2_6X6RT = editorCore->FindRenderTarget("Target_L2_6X6", isGame); // /24
		const auto& l3_6X6RT = editorCore->FindRenderTarget("Target_L3_6X6", isGame); // /144
		const auto& l3_tmp = editorCore->FindRenderTarget("Target_L3_Tmp", isGame); // /144
		const auto& l2_tmp = editorCore->FindRenderTarget("Target_L2_Tmp", isGame); // /24
		const auto& l1_tmp = editorCore->FindRenderTarget("Target_L1_Tmp", isGame);
		const auto& bloom = editorCore->FindRenderTarget("Target_Bloom", isGame);

		const auto& glowMap = editorCore->FindRenderTarget("Target_GlowMap", isGame)->GetSRV();

		const auto& finalMap = editorCore->FindRenderTarget("Target_FinalScene", isGame);

		D3D11_TEXTURE2D_DESC texture2DDesc{};
		finalMap->GetTexture2D()->GetDesc(&texture2DDesc);
		initVP(texture2DDesc.Width, texture2DDesc.Height);

		const _float blendFactor[4] = { 0.f,0.f,0.f,0.f };

		_float2 l1TexelSize = { 1.f / m_VP[L1].Width, 1.f / m_VP[L1].Height };
		_float2 l2TexelSize = { 1.f / m_VP[L2].Width, 1.f / m_VP[L2].Height };
		_float2 l3TexelSize = { 1.f / m_VP[L3].Width, 1.f / m_VP[L3].Height };

		// 1. 4x4 down sample ( 4x4 )
		context->OMSetRenderTargets(1, l1_4X4RT->GetRTV().GetAddressOf(), nullptr);
		m_4x4Down->SetSampler("Sampler", m_LinearClamp);
		m_4x4Down->SetTexture("SrcTexture", glowMap);
		m_4x4Down->Bind(context);
		context->RSSetViewports(1, &m_VP[L1]);
		context->Draw(3, 0);

		// 2. 6x6 down sample ( 24x24 )
		context->OMSetRenderTargets(1, l2_6X6RT->GetRTV().GetAddressOf(), nullptr);
		m_6x6Down->SetSampler("Sampler", m_LinearClamp);
		m_6x6Down->SetTexture("SrcTexture", l1_4X4RT->GetSRV());
		m_6x6Down->Bind(context);
		context->RSSetViewports(1, &m_VP[L2]);
		context->Draw(3, 0);

		// 3. 6x6 down sample ( 144x144 )
		context->OMSetRenderTargets(1, l3_6X6RT->GetRTV().GetAddressOf(), nullptr);
		m_6x6Down->SetSampler("Sampler", m_LinearClamp);
		m_6x6Down->SetTexture("SrcTexture", l2_6X6RT->GetSRV());
		m_6x6Down->Bind(context);
		context->RSSetViewports(1, &m_VP[L3]);
		context->Draw(3, 0);

		// 4. gaussian blur ( 144x144 )
		// blur h, blur v
		context->OMSetRenderTargets(1, l3_tmp->GetRTV().GetAddressOf(), nullptr);
		m_BlurH->SetSampler("Sampler", m_LinearClamp);
		m_BlurH->SetTexture("SrcTexture", l3_6X6RT->GetSRV());
		m_BlurH->SetFloat2("TexelSize", l3TexelSize);
		m_BlurH->Bind(context);
		context->RSSetViewports(1, &m_VP[L3]);
		context->Draw(3, 0);

		context->OMSetRenderTargets(1, l3_6X6RT->GetRTV().GetAddressOf(), nullptr);
		m_BlurV->SetSampler("Sampler", m_LinearClamp);
		m_BlurV->SetTexture("SrcTexture", l3_tmp->GetSRV());
		m_BlurV->SetFloat2("TexelSize", l3TexelSize);
		m_BlurV->Bind(context);
		context->RSSetViewports(1, &m_VP[L3]);
		context->Draw(3, 0);

		// 5. 6x6 up + add ( 24x24 + blur )
		context->OMSetBlendState(m_AddBlend.Get(), blendFactor, 0xffffffff);
		context->OMSetRenderTargets(1, l2_6X6RT->GetRTV().GetAddressOf(), nullptr);
		m_6x6Up->SetSampler("Sampler", m_LinearClamp);
		m_6x6Up->SetTexture("SrcTexture", l3_6X6RT->GetSRV());
		m_6x6Up->Bind(context);
		context->RSSetViewports(1, &m_VP[L2]);
		context->Draw(3, 0);
		context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);

		// 6. gaussian blur ( 24x24 )
		// blur h, blur v
		context->OMSetRenderTargets(1, l2_tmp->GetRTV().GetAddressOf(), nullptr);
		m_BlurH->SetSampler("Sampler", m_LinearClamp);
		m_BlurH->SetTexture("SrcTexture", l2_6X6RT->GetSRV());
		m_BlurH->SetFloat2("TexelSize", l2TexelSize);
		m_BlurH->Bind(context);
		context->RSSetViewports(1, &m_VP[L2]);
		context->Draw(3, 0);

		context->OMSetRenderTargets(1, l2_6X6RT->GetRTV().GetAddressOf(), nullptr);
		m_BlurV->SetSampler("Sampler", m_LinearClamp);
		m_BlurV->SetTexture("SrcTexture", l2_tmp->GetSRV());
		m_BlurV->SetFloat2("TexelSize", l2TexelSize);
		m_BlurV->Bind(context);
		context->RSSetViewports(1, &m_VP[L2]);
		context->Draw(3, 0);

		// 7. 6x6 up + add
		context->OMSetBlendState(m_AddBlend.Get(), blendFactor, 0xffffffff);
		context->OMSetRenderTargets(1, l1_4X4RT->GetRTV().GetAddressOf(), nullptr);
		m_6x6Up->SetSampler("Sampler", m_LinearClamp);
		m_6x6Up->SetTexture("SrcTexture", l2_6X6RT->GetSRV());
		m_6x6Up->Bind(context);
		context->RSSetViewports(1, &m_VP[L1]);
		context->Draw(3, 0);
		context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);

		// 8. gaussian blur
			// blur h, blur v
		context->OMSetRenderTargets(1, l1_tmp->GetRTV().GetAddressOf(), nullptr);
		m_BlurH->SetSampler("Sampler", m_LinearClamp);
		m_BlurH->SetTexture("SrcTexture", l1_4X4RT->GetSRV());
		m_BlurH->SetFloat2("TexelSize", l1TexelSize);
		m_BlurH->Bind(context);
		context->RSSetViewports(1, &m_VP[L1]);
		context->Draw(3, 0);

		context->OMSetRenderTargets(1, l1_4X4RT->GetRTV().GetAddressOf(), nullptr);
		m_BlurV->SetSampler("Sampler", m_LinearClamp);
		m_BlurV->SetTexture("SrcTexture", l1_tmp->GetSRV());
		m_BlurV->SetFloat2("TexelSize", l1TexelSize);
		m_BlurV->Bind(context);
		context->RSSetViewports(1, &m_VP[L1]);
		context->Draw(3, 0);

		// 9. Composite ( scene +  4x4 up scale bloom * intensity
		context->OMSetRenderTargets(1, bloom->GetRTV().GetAddressOf(), nullptr);
		m_Composite->SetSampler("Sampler", m_LinearClamp);
		m_Composite->SetTexture("SceneTexture", finalMap->GetSRV());
		m_Composite->SetTexture("BloomTexture", l1_4X4RT->GetSRV());
		m_Composite->Bind(context);
		context->RSSetViewports(1, &m_VP[L0]);
		context->Draw(3, 0);

		editorCore->BeginMRT("Final-Scene", isGame);

		ComPtr<ID3D11DepthStencilState> prevState;
		UINT ref;
		context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);

		context->OMSetDepthStencilState(m_DSState.Get(), 0);
		m_Final->SetSampler("Sampler", m_LinearClamp);
		m_Final->SetTexture("SrcTexture", bloom->GetSRV());
		m_Final->Bind(context);
		context->Draw(3, 0);

		editorCore->EndMRT(isGame);

		context->OMSetDepthStencilState(prevState.Get(), ref);

		return S_OK;
	}

	else
	{
		const auto& l1_4X4RT = editorCore->FindRenderTarget("Target_L1_4X4", isGame); // /4
		const auto& l2_6X6RT = editorCore->FindRenderTarget("Target_L2_6X6", isGame); // /24
		const auto& l3_6X6RT = editorCore->FindRenderTarget("Target_L3_6X6", isGame); // /144
		const auto& l3_tmp = editorCore->FindRenderTarget("Target_L3_Tmp", isGame); // /144
		const auto& l2_tmp = editorCore->FindRenderTarget("Target_L2_Tmp", isGame); // /24
		const auto& l1_tmp = editorCore->FindRenderTarget("Target_L1_Tmp", isGame);
		const auto& bloom = editorCore->FindRenderTarget("Target_Bloom", isGame);

		const auto& glowMap = editorCore->FindRenderTarget("Target_GlowMap", isGame)->GetSRV();

		const auto& finalMap = editorCore->FindRenderTarget("Target_FinalScene", isGame);

		D3D11_TEXTURE2D_DESC texture2DDesc{};
		finalMap->GetTexture2D()->GetDesc(&texture2DDesc);
		initVP(texture2DDesc.Width, texture2DDesc.Height);

		const _float blendFactor[4] = { 0.f,0.f,0.f,0.f };

		_float2 l1TexelSize = { 1.f / m_VP[L1].Width, 1.f / m_VP[L1].Height };
		_float2 l2TexelSize = { 1.f / m_VP[L2].Width, 1.f / m_VP[L2].Height };
		_float2 l3TexelSize = { 1.f / m_VP[L3].Width, 1.f / m_VP[L3].Height };

		// 1. 4x4 down sample ( 4x4 )
		context->OMSetRenderTargets(1, l1_4X4RT->GetRTV().GetAddressOf(), nullptr);
		m_4x4Down->SetSampler("Sampler", m_LinearClamp);
		m_4x4Down->SetTexture("SrcTexture", glowMap);
		m_4x4Down->Bind(context);
		context->RSSetViewports(1, &m_VP[L1]);
		context->Draw(3, 0);

		// 2. 6x6 down sample ( 24x24 )
		context->OMSetRenderTargets(1, l2_6X6RT->GetRTV().GetAddressOf(), nullptr);
		m_6x6Down->SetSampler("Sampler", m_LinearClamp);
		m_6x6Down->SetTexture("SrcTexture", l1_4X4RT->GetSRV());
		m_6x6Down->Bind(context);
		context->RSSetViewports(1, &m_VP[L2]);
		context->Draw(3, 0);

		// 3. 6x6 down sample ( 144x144 )
		context->OMSetRenderTargets(1, l3_6X6RT->GetRTV().GetAddressOf(), nullptr);
		m_6x6Down->SetSampler("Sampler", m_LinearClamp);
		m_6x6Down->SetTexture("SrcTexture", l2_6X6RT->GetSRV());
		m_6x6Down->Bind(context);
		context->RSSetViewports(1, &m_VP[L3]);
		context->Draw(3, 0);

		// 4. gaussian blur ( 144x144 )
		// blur h, blur v
		context->OMSetRenderTargets(1, l3_tmp->GetRTV().GetAddressOf(), nullptr);
		m_BlurH->SetSampler("Sampler", m_LinearClamp);
		m_BlurH->SetTexture("SrcTexture", l3_6X6RT->GetSRV());
		m_BlurH->SetFloat2("TexelSize", l3TexelSize);
		m_BlurH->Bind(context);
		context->RSSetViewports(1, &m_VP[L3]);
		context->Draw(3, 0);

		context->OMSetRenderTargets(1, l3_6X6RT->GetRTV().GetAddressOf(), nullptr);
		m_BlurV->SetSampler("Sampler", m_LinearClamp);
		m_BlurV->SetTexture("SrcTexture", l3_tmp->GetSRV());
		m_BlurV->SetFloat2("TexelSize", l3TexelSize);
		m_BlurV->Bind(context);
		context->RSSetViewports(1, &m_VP[L3]);
		context->Draw(3, 0);

		// 5. 6x6 up + add ( 24x24 + blur )
		context->OMSetBlendState(m_AddBlend.Get(), blendFactor, 0xffffffff);
		context->OMSetRenderTargets(1, l2_6X6RT->GetRTV().GetAddressOf(), nullptr);
		m_6x6Up->SetSampler("Sampler", m_LinearClamp);
		m_6x6Up->SetTexture("SrcTexture", l3_6X6RT->GetSRV());
		m_6x6Up->Bind(context);
		context->RSSetViewports(1, &m_VP[L2]);
		context->Draw(3, 0);
		context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);

		// 6. gaussian blur ( 24x24 )
		// blur h, blur v
		context->OMSetRenderTargets(1, l2_tmp->GetRTV().GetAddressOf(), nullptr);
		m_BlurH->SetSampler("Sampler", m_LinearClamp);
		m_BlurH->SetTexture("SrcTexture", l2_6X6RT->GetSRV());
		m_BlurH->SetFloat2("TexelSize", l2TexelSize);
		m_BlurH->Bind(context);
		context->RSSetViewports(1, &m_VP[L2]);
		context->Draw(3, 0);

		context->OMSetRenderTargets(1, l2_6X6RT->GetRTV().GetAddressOf(), nullptr);
		m_BlurV->SetSampler("Sampler", m_LinearClamp);
		m_BlurV->SetTexture("SrcTexture", l2_tmp->GetSRV());
		m_BlurV->SetFloat2("TexelSize", l2TexelSize);
		m_BlurV->Bind(context);
		context->RSSetViewports(1, &m_VP[L2]);
		context->Draw(3, 0);

		// 7. 6x6 up + add
		context->OMSetBlendState(m_AddBlend.Get(), blendFactor, 0xffffffff);
		context->OMSetRenderTargets(1, l1_4X4RT->GetRTV().GetAddressOf(), nullptr);
		m_6x6Up->SetSampler("Sampler", m_LinearClamp);
		m_6x6Up->SetTexture("SrcTexture", l2_6X6RT->GetSRV());
		m_6x6Up->Bind(context);
		context->RSSetViewports(1, &m_VP[L1]);
		context->Draw(3, 0);
		context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);

		// 8. gaussian blur
			// blur h, blur v
		context->OMSetRenderTargets(1, l1_tmp->GetRTV().GetAddressOf(), nullptr);
		m_BlurH->SetSampler("Sampler", m_LinearClamp);
		m_BlurH->SetTexture("SrcTexture", l1_4X4RT->GetSRV());
		m_BlurH->SetFloat2("TexelSize", l1TexelSize);
		m_BlurH->Bind(context);
		context->RSSetViewports(1, &m_VP[L1]);
		context->Draw(3, 0);

		context->OMSetRenderTargets(1, l1_4X4RT->GetRTV().GetAddressOf(), nullptr);
		m_BlurV->SetSampler("Sampler", m_LinearClamp);
		m_BlurV->SetTexture("SrcTexture", l1_tmp->GetSRV());
		m_BlurV->SetFloat2("TexelSize", l1TexelSize);
		m_BlurV->Bind(context);
		context->RSSetViewports(1, &m_VP[L1]);
		context->Draw(3, 0);

		// 9. Composite ( scene +  4x4 up scale bloom * intensity
		context->OMSetRenderTargets(1, bloom->GetRTV().GetAddressOf(), nullptr);
		m_Composite->SetSampler("Sampler", m_LinearClamp);
		m_Composite->SetTexture("SceneTexture", finalMap->GetSRV());
		m_Composite->SetTexture("BloomTexture", l1_4X4RT->GetSRV());
		m_Composite->Bind(context);
		context->RSSetViewports(1, &m_VP[L0]);
		context->Draw(3, 0);

		editorCore->BeginMRT("Final-Scene", isGame);

		ComPtr<ID3D11DepthStencilState> prevState;
		UINT ref;
		context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);

		context->OMSetDepthStencilState(m_DSState.Get(), 0);
		m_Final->SetSampler("Sampler", m_LinearClamp);
		m_Final->SetTexture("SrcTexture", bloom->GetSRV());
		m_Final->Bind(context);
		context->Draw(3, 0);

		editorCore->EndMRT(isGame);

		context->OMSetDepthStencilState(prevState.Get(), ref);

		return S_OK;
	}
}

void engine::GlowPass::Release()
{
}

void engine::GlowPass::initVP(UINT width, UINT height)
{
	setVP(L0, width, height);

	setVP(L1, width / 4, height /4 );

	setVP(L2, (width / 4 + 5) / 6, (height / 4 + 5) / 6);

	setVP(L3, (m_VP[L2].Width + 5) / 6, (m_VP[L2].Height + 5) / 6);
}

void engine::GlowPass::setVP(Level lv, UINT w, UINT h)
{
	D3D11_VIEWPORT vp{};
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<FLOAT>(w);
	vp.Height = static_cast<FLOAT>(h);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	m_VP[lv] = vp;
}
