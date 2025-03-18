#include "EditorCore.h"

#include "Core.h"
#include "D3D11Manager.h"
#include "EditorComponentManager.h"

editor::engine::EditorCore::EditorCore() : m_EditorComponentManager(nullptr), m_OffscreenWidth(0), m_OffscreenHeight(0),
                                           m_bEditorMode(true)
{

}

editor::engine::EditorCore::~EditorCore()
{

}

HRESULT editor::engine::EditorCore::Initialize(HWND hwnd)
{
	m_EditorComponentManager = &::engine::editor::EditorComponentManager::GetInstance();
	readyGameView(static_cast<int>(::engine::D3D11Manager::GetInstance().GetWinSizeX()), static_cast<int>(::engine::D3D11Manager::GetInstance().GetWinSizeY()));
}

void editor::engine::EditorCore::Initialization()
{

}

void editor::engine::EditorCore::SceneRender(const ::engine::ComPtr<ID3D11DeviceContext>& context)
{
	renderScene(context);
	renderGame(context);
}

void editor::engine::EditorCore::Decommissioning()
{
	if (m_bEditorMode == true)
	{
		m_EditorComponentManager->FlushDestroyComponent();
	}

	else
	{
		::engine::Core::GetInstance().Decommissioning();
	}
}

void editor::engine::EditorCore::renderScene(const ::engine::ComPtr<ID3D11DeviceContext>& context)
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

void editor::engine::EditorCore::readySceneView(int width, int height)
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

		auto device = ::engine::D3D11Manager::GetInstance().GetDevice();

		// Texture2D 생성
		::engine::ComPtr<ID3D11Texture2D> renderTexture;
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

		::engine::ComPtr<ID3D11Texture2D> depthStencilTexture;
		device->CreateTexture2D(&textureDesc, nullptr, depthStencilTexture.GetAddressOf());
		device->CreateDepthStencilView(depthStencilTexture.Get(), nullptr, m_SceneDepthStencilView.ReleaseAndGetAddressOf());

		m_OffscreenWidth = width;
		m_OffscreenHeight = height;
	}
}

void editor::engine::EditorCore::renderGame(const ::engine::ComPtr<ID3D11DeviceContext>& context)
{
	if (m_bEditorMode == true)
	{
		m_EditorComponentManager->Render(context);
	}

	else
	{

	}
}

void editor::engine::EditorCore::readyGameView(int width, int height)
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

	auto device = ::engine::D3D11Manager::GetInstance().GetDevice();

	// Texture2D 생성
	::engine::ComPtr<ID3D11Texture2D> renderTexture;
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

	::engine::ComPtr<ID3D11Texture2D> depthStencilTexture;
	device->CreateTexture2D(&textureDesc, nullptr, depthStencilTexture.GetAddressOf());
	device->CreateDepthStencilView(depthStencilTexture.Get(), nullptr, m_GameDepthStencilView.ReleaseAndGetAddressOf());

}
