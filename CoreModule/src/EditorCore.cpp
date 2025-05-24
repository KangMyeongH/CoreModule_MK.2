#include "EditorCore.h"

#include "Camera.h"
#include "Core.h"
#include "D3D11Manager.h"
#include "EditorComponentManager.h"
#include "Grid.h"
#include "Material.h"
#include "RenderTarget.h"
#include "Transform.h"

engine::editor::EditorCore::EditorCore() : m_EditorComponentManager(nullptr), m_OffscreenWidth(0), m_OffscreenHeight(0),
                                           m_bEditorMode(true)
{

}

engine::editor::EditorCore::~EditorCore()
{

}

HRESULT engine::editor::EditorCore::Initialize(HWND hwnd)
{
	m_EditorComponentManager = &EditorComponentManager::GetInstance();

	ReadyGameView(static_cast<int>(D3D11Manager::GetInstance().GetWinSizeX()), static_cast<int>(D3D11Manager::GetInstance().GetWinSizeY()));

	m_Grid = std::make_shared<Grid>();
	m_Grid->InitGrid(D3D11Manager::GetInstance().GetDevice(), 400);

	m_EditorComponentManager->Initialize();


	return S_OK;
}

void engine::editor::EditorCore::Initialization()
{

}

void engine::editor::EditorCore::SceneRender(const ComPtr<ID3D11DeviceContext>& context)
{
	ClearRenderTarget(context);
	RenderScene(context);
	RenderGame(context);
}

void engine::editor::EditorCore::Decommissioning()
{
	if (m_bEditorMode == true)
	{
		m_EditorComponentManager->FlushDestroyComponent();
	}

	else
	{
		Core::GetInstance().Decommissioning();
	}
}

void engine::editor::EditorCore::RenderScene(const ComPtr<ID3D11DeviceContext>& context)
{
	// Scene View와 Game View를 나눈 이유
	// 1. 에디터 카메라, 인게임 카메라 기준의 두 개의 화면.
	// 2. Scene View에서는 각종 디버깅 렌더링 ( 그리드, 와이어 프래임(충돌체) )
	// 3. Game View에서는 실제 인게임 화면을 보여주는 것.
	//=======================================================================//

	// TODO : Scene View에서 사용할 그리드(끝) + 와이어 프래임 렌더링 하는거 추가 해야함.

	if (m_bEditorMode == true)
	{
		if (m_SceneTargetView.Get() && m_SceneResourceView.Get() && m_SceneDepthStencilView.Get())
		{
			D3D11_VIEWPORT vp = {};
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			vp.Width = static_cast<float>(m_OffscreenWidth);
			vp.Height = static_cast<float>(m_OffscreenHeight);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;

			context->RSSetViewports(1, &vp);

			context->OMSetRenderTargets(1, m_SceneTargetView.GetAddressOf(), m_SceneDepthStencilView.Get());

			float mainClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.f };
			context->ClearRenderTargetView(m_SceneTargetView.Get(), mainClearColor);
			context->ClearDepthStencilView(m_SceneDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			_float4X4 viewMat, projMat;
			XMStoreFloat4x4(&viewMat, m_EditorCamera.GetViewMatrix());
			XMStoreFloat4x4(&projMat, m_EditorCamera.GetProjectMatrix());

			m_Grid->UpdateGridVertices(context, m_EditorCamera.GetCameraPos(), 1.0f, 100);
			m_Grid->Bind(context, viewMat, projMat);
			m_Grid->RenderGird(context);

			_float3 camPos = m_EditorCamera.GetCameraPos().Value;

			CamData data{};
			data.Position = { camPos.x, camPos.y, camPos.z, 1.f };
			data.ViewMat = viewMat;
			data.ProjMat = projMat;
			data.NearFarPlane = { m_EditorCamera.GetNearPlane(), m_EditorCamera.GetFarPlane(), 0.f, 0.f };

			m_EditorComponentManager->Render(context, &data, false);

			D3D11Manager::GetInstance().PostProcessForceAlphaOnePass();
		}
	}

	else
	{
		
	}
}

void engine::editor::EditorCore::ReadySceneView(int width, int height)
{
	if (width <= 0 || height <= 0)
	{
		m_SceneTargetView.Reset();
		m_SceneResourceView.Reset();
		m_SceneDepthStencilView.Reset();
		m_SceneRenderTargets.clear();
		m_SceneMRTs.clear();

		return;
	}

	if (width != m_OffscreenWidth || height != m_OffscreenHeight)
	{
		// Render Target, Resource View Texture2D
		D3D11_TEXTURE2D_DESC rtDesc = {};
		rtDesc.Width = width;
		rtDesc.Height = height;
		rtDesc.MipLevels = 1;
		rtDesc.ArraySize = 1;
		rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtDesc.SampleDesc.Count = 1;
		rtDesc.Usage = D3D11_USAGE_DEFAULT;
		rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		auto device = D3D11Manager::GetInstance().GetDevice();
		auto context = D3D11Manager::GetInstance().GetContext();

		// Texture2D 생성
		ComPtr<ID3D11Texture2D> renderTexture;
		if (FAILED(device->CreateTexture2D(&rtDesc, nullptr, renderTexture.GetAddressOf())))
		{
			std::cerr << "ERROR : Failed Create Scene View renderTexture! \n";
		}

		// RTV 생성
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = rtDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		if (FAILED(device->CreateRenderTargetView(renderTexture.Get(), &rtvDesc, m_SceneTargetView.ReleaseAndGetAddressOf())))
		{
			std::cerr << "ERROR : Failed Create Scene View RenderTargetView! \n";
		}

		// SRV 생성
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = rtDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		if (FAILED(device->CreateShaderResourceView(renderTexture.Get(), &srvDesc, m_SceneResourceView.ReleaseAndGetAddressOf())))
		{
			std::cerr << "ERROR : Failed Create Scene View ShaderResourceView! \n";
		}


		// Depth Stencil Texture2D 
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

		textureDesc.SampleDesc.Quality = 0;
		textureDesc.SampleDesc.Count = 1;

		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		ComPtr<ID3D11Texture2D> depthStencilTexture;
		if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, depthStencilTexture.GetAddressOf())))
		{
			std::cerr << "ERROR : Failed Create Scene View depthStencilTexture! \n";
		}

		if (FAILED(device->CreateDepthStencilView(depthStencilTexture.Get(), nullptr, m_SceneDepthStencilView.ReleaseAndGetAddressOf())))
		{
			std::cerr << "ERROR : Failed Create Scene View depthStencilView! \n";
		}

		m_OffscreenWidth = width;
		m_OffscreenHeight = height;

		m_EditorCamera.SetAspectRatio(static_cast<float>(width) / static_cast<float>(height));

		m_SceneRenderTargets.clear();
		m_SceneMRTs.clear();

		// Scene View RTVs
		AddRenderTarget("Target_Position", device, context, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f), false);
		AddRenderTarget("Target_Diffuse", device, context, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(1.f, 1.f, 1.f, 0.f), false);
		AddRenderTarget("Target_Normal", device, context, width, height, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.f, 1.f, 1.f, 1.f), false);
		AddRenderTarget("Target_Depth", device, context, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f), false);
		AddRenderTarget("Target_Outline", device, context, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), false);

		AddRenderTarget("Target_Shade", device, context, width, height, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 0.f), false);
		AddRenderTarget("Target_Specular", device, context, width, height, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 0.f), false);

		AddRenderTarget("Target_GlowMap", device, context, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), false);

		AddRenderTarget("Target_FinalScene", device, context, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), false);

		AddRenderTarget("Target_L1_4X4", device, context, width / 4.f, height / 4.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), false);
		AddRenderTarget("Target_L2_6X6", device, context, width / 24.f, height / 24.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), false);
		AddRenderTarget("Target_L3_6X6", device, context, width / 144.f, height / 144.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), false);
		AddRenderTarget("Target_L3_Tmp", device, context, width / 144.f, height / 144.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), false);
		AddRenderTarget("Target_L2_Tmp", device, context, width / 24.f, height / 24.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), false);
		AddRenderTarget("Target_L1_Tmp", device, context, width / 4.f, height / 4.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), false);

		AddRenderTarget("Target_Bloom", device, context, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), false);


		AddMRT("G-Buffer", "Target_Position", false);
		AddMRT("G-Buffer", "Target_Diffuse", false);
		AddMRT("G-Buffer", "Target_Normal", false);
		AddMRT("G-Buffer", "Target_Depth", false);

		AddMRT("Light-Pass", "Target_Shade", false);
		AddMRT("Light-Pass", "Target_Specular", false);

		AddMRT("Outline-Pass", "Target_Outline", false);

		AddMRT("Effect-Pass", "Target_FinalScene", false);
		AddMRT("Effect-Pass", "Target_GlowMap", false);

		AddMRT("Final-Scene", "Target_FinalScene", false);
	}
}

void engine::editor::EditorCore::RenderGame(const ComPtr<ID3D11DeviceContext>& context)
{
	if (m_bEditorMode == true)
	{
		D3D11_VIEWPORT vp = {};
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		vp.Width = static_cast<float>(D3D11Manager::GetInstance().GetWinSizeX());
		vp.Height = static_cast<float>(D3D11Manager::GetInstance().GetWinSizeY());
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		context->RSSetViewports(1, &vp);

		context->OMSetRenderTargets(1, m_GameTargetView.GetAddressOf(), m_GameDepthStencilView.Get());
		
		float mainClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		context->ClearRenderTargetView(m_GameTargetView.Get(), mainClearColor);
		context->ClearDepthStencilView(m_GameDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// TODO : 임시로 카메라 값 넣은거임 수정해야함.
		auto mainCam = m_EditorComponentManager->GetMainCam();
		_float4X4 viewMat, projMat;
		_float3 camPos;
		_float nearPlane;
		_float farPlane;

		if (mainCam)
		{
			mainCam->UpdateCamera(viewMat, projMat);
			camPos = mainCam->GetTransform()->Position().Value;
			nearPlane = mainCam->GetNearPlane();
			farPlane = mainCam->GetFarPlane();
			//XMStoreFloat4x4(&viewMat, XMMatrixTranspose(XMLoadFloat4x4(&viewMat)));
			//XMStoreFloat4x4(&projMat, XMMatrixTranspose(XMLoadFloat4x4(&projMat)));
		}

		else
		{
			XMStoreFloat4x4(&viewMat, m_EditorCamera.GetViewMatrix());
			XMStoreFloat4x4(&projMat, m_EditorCamera.GetProjectMatrix());
			camPos = _float3{ 0.f,0.f,0.f };
			nearPlane = 0.5f;
			farPlane = 100.f;
		}


		CamData data{};
		data.Position = { camPos.x, camPos.y, camPos.z, 1.f };
		data.ViewMat = viewMat;
		data.ProjMat = projMat;
		data.NearFarPlane = { nearPlane, farPlane, 0.f, 0.f };

		m_EditorComponentManager->Render(context, &data, true);

		D3D11Manager::GetInstance().PostProcessForceAlphaOnePass();
	}

	else
	{

	}
}

void engine::editor::EditorCore::ReadyGameView(int width, int height)
{
	// Render Target, Resource View Texture2D
	D3D11_TEXTURE2D_DESC rtDesc = {};
	rtDesc.Width = width;
	rtDesc.Height = height;
	rtDesc.MipLevels = 1;
	rtDesc.ArraySize = 1;
	rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtDesc.SampleDesc.Count = 1;
	rtDesc.Usage = D3D11_USAGE_DEFAULT;
	rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	auto device = D3D11Manager::GetInstance().GetDevice();
	auto context = D3D11Manager::GetInstance().GetContext();

	// Texture2D 생성
	ComPtr<ID3D11Texture2D> renderTexture;
	device->CreateTexture2D(&rtDesc, nullptr, renderTexture.GetAddressOf());

	// RTV 생성
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = rtDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(renderTexture.Get(), &rtvDesc, m_GameTargetView.ReleaseAndGetAddressOf());

	// SRV 생성
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = rtDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(renderTexture.Get(), &srvDesc, m_GameResourceView.ReleaseAndGetAddressOf());

	// Depth Stencil Texture2D 
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	textureDesc.SampleDesc.Quality = 0;
	textureDesc.SampleDesc.Count = 1;

	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> depthStencilTexture;
	device->CreateTexture2D(&textureDesc, nullptr, depthStencilTexture.GetAddressOf());
	device->CreateDepthStencilView(depthStencilTexture.Get(), nullptr, m_GameDepthStencilView.ReleaseAndGetAddressOf());

	// Scene View RTVs
	AddRenderTarget("Target_Position", device, context, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f), true);
	AddRenderTarget("Target_Diffuse", device, context, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(1.f, 1.f, 1.f, 0.f), true);
	AddRenderTarget("Target_Normal", device, context, width, height, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.f, 1.f, 1.f, 1.f), true);
	AddRenderTarget("Target_Depth", device, context, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f), true);
	AddRenderTarget("Target_Outline", device, context, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), true);

	AddRenderTarget("Target_Shade", device, context, width, height, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 0.f), true);
	AddRenderTarget("Target_Specular", device, context, width, height, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 0.f), true);

	AddRenderTarget("Target_GlowMap", device, context, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), true);

	AddRenderTarget("Target_FinalScene", device, context, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(1.f, 0.f, 0.f, 0.f), true);

	AddRenderTarget("Target_L1_4X4", device, context, width / 4.f, height / 4.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), true);
	AddRenderTarget("Target_L2_6X6", device, context, width / 24.f, height / 24.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), true);
	AddRenderTarget("Target_L3_6X6", device, context, width / 144.f, height / 144.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), true);
	AddRenderTarget("Target_L3_Tmp", device, context, width / 144.f, height / 144.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), true);
	AddRenderTarget("Target_L2_Tmp", device, context, width / 24.f, height / 24.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), true);
	AddRenderTarget("Target_L1_Tmp", device, context, width / 4.f, height / 4.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), true);

	AddRenderTarget("Target_Bloom", device, context, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f), true);


	AddMRT("G-Buffer", "Target_Position", true);
	AddMRT("G-Buffer", "Target_Diffuse", true);
	AddMRT("G-Buffer", "Target_Normal", true);
	AddMRT("G-Buffer", "Target_Depth", true);

	AddMRT("Light-Pass", "Target_Shade", true);
	AddMRT("Light-Pass", "Target_Specular", true);

	AddMRT("Outline-Pass", "Target_Outline", true);

	AddMRT("Effect-Pass", "Target_FinalScene", true);
	AddMRT("Effect-Pass", "Target_GlowMap", true);

	AddMRT("Final-Scene", "Target_FinalScene", true);
}

void engine::editor::EditorCore::AddRenderTarget(const _string& tag, const ComPtr<ID3D11Device>& device,
	const ComPtr<ID3D11DeviceContext>& context, _uint sizeX, _uint sizeY, DXGI_FORMAT pixelFormat,
	const _float4& clearColor, _bool isGame)
{
	const auto& rt = RenderTarget::Create(device, context, sizeX, sizeY, pixelFormat, clearColor);

	if (nullptr == rt)
	{
		return;
	}

	if (!isGame)
	{
		m_SceneRenderTargets.emplace(tag, rt);
	}

	else
	{
		m_GameRenderTargets.emplace(tag, rt);
	}
}

void engine::editor::EditorCore::AddMRT(const _string& mrtTag, const _string& targetTag, _bool isGame)
{
	const auto& renderTarget = FindRenderTarget(targetTag, isGame);
	if (renderTarget == nullptr)
	{
		return;
	}

	std::list<SharedPtr<RenderTarget>>* mrt = FindMRT(mrtTag, isGame);

	if (mrt == nullptr)
	{
		std::list<SharedPtr<RenderTarget>> mrtList;
		mrtList.push_back(renderTarget);

		if (!isGame)
		{
			m_SceneMRTs.emplace(mrtTag, mrtList);
		}

		else
		{
			m_GameMRTs.emplace(mrtTag, mrtList);
		}
	}

	else
	{
		mrt->push_back(renderTarget);
	}
}

engine::SharedPtr<engine::RenderTarget> engine::editor::EditorCore::FindRenderTarget(const _string& tag, _bool isGame)
{
	if (!isGame)
	{
		const auto iter = m_SceneRenderTargets.find(tag);

		if (iter == m_SceneRenderTargets.end())
		{
			return nullptr;
		}

		return iter->second;
	}

	else
	{
		const auto iter = m_GameRenderTargets.find(tag);

		if (iter == m_GameRenderTargets.end())
		{
			return nullptr;
		}

		return iter->second;
	}
}

std::list<engine::SharedPtr<engine::RenderTarget>>* engine::editor::EditorCore::FindMRT(const _string& tag, _bool isGame)
{
	if (!isGame)
	{
		const auto iter = m_SceneMRTs.find(tag);

		if (iter == m_SceneMRTs.end())
		{
			return nullptr;
		}

		return &iter->second;
	}

	else
	{

		const auto iter = m_GameMRTs.find(tag);

		if (iter == m_GameMRTs.end())
		{
			return nullptr;
		}

		return &iter->second;
	}
}

HRESULT engine::editor::EditorCore::BeginMRT(const _string& tag, _bool isGame)
{
	if (!isGame)
	{
		auto iter = m_SceneMRTs.find(tag);

		if (iter == m_SceneMRTs.end())
		{
			return E_FAIL;
		}

		std::list<SharedPtr<RenderTarget>>* mtvs = &(iter->second);

		ID3D11DeviceContext* context = D3D11Manager::GetInstance().GetContext().Get();

		_uint numRenderTargets = 0;

		ID3D11RenderTargetView* renderTargets[8] = {};

		for (auto& renderTarget : *mtvs)
		{
			//renderTarget->Clear(context);

			renderTargets[numRenderTargets++] = renderTarget->GetRTV().Get();
		}
		// scene View 냐 GameView냐에 따라서 dsv설정 바꾸기
		ID3D11DepthStencilView* dsv = m_SceneDepthStencilView.Get();

		context->OMSetRenderTargets(numRenderTargets, renderTargets, dsv);

		return S_OK;
	}

	else
	{
		auto iter = m_GameMRTs.find(tag);

		if (iter == m_GameMRTs.end())
		{
			return E_FAIL;
		}

		std::list<SharedPtr<RenderTarget>>* mtvs = &(iter->second);

		ID3D11DeviceContext* context = D3D11Manager::GetInstance().GetContext().Get();

		_uint numRenderTargets = 0;

		ID3D11RenderTargetView* renderTargets[8] = {};

		for (auto& renderTarget : *mtvs)
		{
			//renderTarget->Clear(context);

			renderTargets[numRenderTargets++] = renderTarget->GetRTV().Get();
		}
		// scene View 냐 GameView냐에 따라서 dsv설정 바꾸기
		ID3D11DepthStencilView* dsv = m_GameDepthStencilView.Get();

		context->OMSetRenderTargets(numRenderTargets, renderTargets, dsv);

		return S_OK;
	}
}

HRESULT engine::editor::EditorCore::EndMRT(_bool isGame)
{
	if (!isGame)
	{
		ID3D11DeviceContext* context = D3D11Manager::GetInstance().GetContext().Get();
		ID3D11RenderTargetView* mainRTV = m_SceneTargetView.Get();
		ID3D11DepthStencilView* dsv = m_SceneDepthStencilView.Get();

		context->OMSetRenderTargets(1, &mainRTV, dsv);
	}

	else
	{
		ID3D11DeviceContext* context = D3D11Manager::GetInstance().GetContext().Get();
		ID3D11RenderTargetView* mainRTV = m_GameTargetView.Get();
		ID3D11DepthStencilView* dsv = m_GameDepthStencilView.Get();

		context->OMSetRenderTargets(1, &mainRTV, dsv);
	}

	return S_OK;
}

void engine::editor::EditorCore::ClearRenderTarget(const ComPtr<ID3D11DeviceContext>& context)
{
	for (const auto& pair : m_GameRenderTargets)
	{
		pair.second->Clear(context.Get());
	}

	for (const auto& pair : m_SceneRenderTargets)
	{
		pair.second->Clear(context.Get());
	}
}
