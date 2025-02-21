#include "D3D11Manager.h"

IMPLEMENT_SINGLETON(engine::D3D11Manager)

engine::D3D11Manager::D3D11Manager()
	: m_Device(nullptr),
	m_DeviceContext(nullptr),
	m_SwapChain(nullptr),
	m_BackBufferRTV(nullptr),
	m_DepthStencilView(nullptr)
{
}

engine::D3D11Manager::~D3D11Manager()
{
	Release();
}

HRESULT engine::D3D11Manager::Initialize(HWND hwnd, _bool isWindowed, _uint winSizeX, _uint winSizeY)
{
	_uint	flag = 0;

#ifdef _DEBUG
	flag = D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL		featureLV;

	if (FAILED(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flag,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&m_Device,
		&featureLV,
		&m_DeviceContext)))
	{
		return E_FAIL;
	}

	if (FAILED(readySwapChain(hwnd, isWindowed, winSizeX, winSizeY)))
	{
		return E_FAIL;
	}

	if (FAILED(readyBackBufferRenderTargetView()))
	{
		return E_FAIL;
	}

	if (FAILED(readyDepthStencilView(winSizeX, winSizeY)))
	{
		return E_FAIL;
	}

	ID3D11RenderTargetView* RTVs[] =
	{
		m_BackBufferRTV
	};

	m_DeviceContext->OMSetRenderTargets(1, RTVs, m_DepthStencilView);

	D3D11_VIEWPORT		viewPortDesc;
	ZeroMemory(&viewPortDesc, sizeof(D3D11_VIEWPORT));
	viewPortDesc.TopLeftX = 0;
	viewPortDesc.TopLeftY = 0;
	viewPortDesc.Width = static_cast<_float>(winSizeX);
	viewPortDesc.Height = static_cast<_float>(winSizeY);
	viewPortDesc.MinDepth = 0.f;
	viewPortDesc.MaxDepth = 1.f;

	m_DeviceContext->RSSetViewports(1, &viewPortDesc);

	return S_OK;
}

HRESULT engine::D3D11Manager::ClearBackBufferView(_float4 clearColor) const
{
	if (nullptr == m_DeviceContext)
	{
		return E_FAIL;
	}

	m_DeviceContext->ClearRenderTargetView(
		m_BackBufferRTV,
		reinterpret_cast<_float*>(&clearColor));

	return S_OK;
}

HRESULT engine::D3D11Manager::ClearDepthStencilView() const
{
	if (nullptr == m_DeviceContext)
	{
		return E_FAIL;
	}

	m_DeviceContext->ClearDepthStencilView(
		m_DepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.f,
		0);

	return S_OK;
}

HRESULT engine::D3D11Manager::Present() const
{
	if (nullptr == m_SwapChain)
	{
		return E_FAIL;
	}

	return m_SwapChain->Present(0, 0);
}

HRESULT engine::D3D11Manager::ResizeBuffer()
{
	SafeRelease(m_BackBufferRTV);
	if (FAILED(m_SwapChain->ResizeBuffers(
		0,
		m_ResizeWidth, m_ResizeHeight,
		DXGI_FORMAT_UNKNOWN,
		0)))
	{
		return E_FAIL;
	}

	m_ResizeWidth = 0;
	m_ResizeHeight = 0;

	if (FAILED(readyBackBufferRenderTargetView()))
	{
		return E_FAIL;
	}
	return S_OK;
}

void engine::D3D11Manager::Release()
{
	SafeRelease(m_SwapChain);
	SafeRelease(m_DepthStencilView);
	SafeRelease(m_BackBufferRTV);
	SafeRelease(m_DeviceContext);
	SafeRelease(m_Device);
}

HRESULT engine::D3D11Manager::readySwapChain(const HWND hWnd, const _bool isWindowed, const _uint winSizeX, const _uint winSizeY)
{
	IDXGIDevice* device = nullptr;
	if (FAILED(m_Device->QueryInterface(
		__uuidof(IDXGIDevice), reinterpret_cast<void**>(&device))))
	{
		return E_FAIL;
	}

	IDXGIAdapter* adapter = nullptr;
	if (FAILED(device->GetParent(
		__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&adapter))))
	{
		return E_FAIL;
	}

	IDXGIFactory* factory = nullptr;
	if (FAILED(adapter->GetParent(
		__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory))))
	{
		return E_FAIL;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc.Width = winSizeX;
	swapChainDesc.BufferDesc.Height = winSizeY;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;

	swapChainDesc.BufferDesc.RefreshRate.Numerator = 144;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = isWindowed;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (FAILED(factory->CreateSwapChain(m_Device, &swapChainDesc, &m_SwapChain)))
	{
		return E_FAIL;
	}

	SafeRelease(factory);
	SafeRelease(adapter);
	SafeRelease(device);

	return S_OK;
}

HRESULT engine::D3D11Manager::readyBackBufferRenderTargetView()
{
	if (nullptr == m_Device)
	{
		return E_FAIL;
	}

	ID3D11Texture2D* backBufferTexture = nullptr;

	if (FAILED(m_SwapChain->GetBuffer(
		0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBufferTexture))))
	{
		return E_FAIL;
	}

	if (FAILED(m_Device->CreateRenderTargetView(
		backBufferTexture, nullptr, &m_BackBufferRTV)))
	{
		return E_FAIL;
	}

	SafeRelease(backBufferTexture);

	return S_OK;
}

HRESULT engine::D3D11Manager::readyDepthStencilView(const _uint winSizeX, const _uint winSizeY)
{
	if (nullptr == m_Device)
	{
		return E_FAIL;
	}

	ID3D11Texture2D* depthStencilTexture = nullptr;

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	textureDesc.Width = winSizeX;
	textureDesc.Height = winSizeY;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	textureDesc.SampleDesc.Quality = 0;
	textureDesc.SampleDesc.Count = 1;

	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	if (FAILED(m_Device->CreateTexture2D(
		&textureDesc, nullptr, &depthStencilTexture)))
	{
		return E_FAIL;
	}

	if (FAILED(m_Device->CreateDepthStencilView(
		depthStencilTexture, nullptr, &m_DepthStencilView)))
	{
		return E_FAIL;
	}

	SafeRelease(depthStencilTexture);

	return S_OK;
}
