#include "EditorCore.h"

#include "Core.h"
#include "D3D11Manager.h"
#include "EditorComponentManager.h"

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

	return S_OK;
}

void engine::editor::EditorCore::Initialization()
{

}

void engine::editor::EditorCore::SceneRender(const ComPtr<ID3D11DeviceContext>& context)
{
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

	// TODO : Scene View에서 사용할 그리드 + 와이어 프래임 렌더링 하는거 추가 해야함.

	if (m_bEditorMode == true)
	{
		if (m_SceneTargetView.Get() && m_SceneResourceView.Get() && m_SceneDepthStencilView.Get())
		{
			context->OMSetRenderTargets(1, m_SceneTargetView.GetAddressOf(), m_SceneDepthStencilView.Get());

			float mainClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
			context->ClearRenderTargetView(m_SceneTargetView.Get(), mainClearColor);
			context->ClearDepthStencilView(m_SceneDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			m_EditorComponentManager->Render(context);
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

		// Texture2D 생성
		ComPtr<ID3D11Texture2D> renderTexture;
		device->CreateTexture2D(&rtDesc, nullptr, renderTexture.GetAddressOf());

		// RTV 생성
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = rtDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		device->CreateRenderTargetView(renderTexture.Get(), &rtvDesc, m_SceneTargetView.ReleaseAndGetAddressOf());

		// SRV 생성
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = rtDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		device->CreateShaderResourceView(renderTexture.Get(), &srvDesc, m_SceneResourceView.ReleaseAndGetAddressOf());

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
		device->CreateDepthStencilView(depthStencilTexture.Get(), nullptr, m_SceneDepthStencilView.ReleaseAndGetAddressOf());

		m_OffscreenWidth = width;
		m_OffscreenHeight = height;
	}
}

void engine::editor::EditorCore::RenderGame(const ComPtr<ID3D11DeviceContext>& context)
{
	if (m_bEditorMode == true)
	{
		context->OMSetRenderTargets(1, m_GameTargetView.GetAddressOf(), m_GameDepthStencilView.Get());

		float mainClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		context->ClearRenderTargetView(m_GameTargetView.Get(), mainClearColor);
		context->ClearDepthStencilView(m_GameDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		m_EditorComponentManager->Render(context);
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

}
