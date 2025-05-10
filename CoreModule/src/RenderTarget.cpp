#include "RenderTarget.h"

HRESULT engine::RenderTarget::Initialize(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& context,
                                         const _uint sizeX, const _uint sizeY, const DXGI_FORMAT pixelFormat, const _float4& clearColor)
{
	D3D11_TEXTURE2D_DESC	texture2DDesc{};

	texture2DDesc.Width = sizeX;
	texture2DDesc.Height = sizeY;
	texture2DDesc.MipLevels = 1;
	texture2DDesc.ArraySize = 1;
	texture2DDesc.Format = pixelFormat;

	texture2DDesc.SampleDesc.Quality = 0;
	texture2DDesc.SampleDesc.Count = 1;

	texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2DDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texture2DDesc.CPUAccessFlags = 0;
	texture2DDesc.MiscFlags = 0;

	if (FAILED(device->CreateTexture2D(&texture2DDesc, nullptr, m_Texture2D.ReleaseAndGetAddressOf())))
	{
		return E_FAIL;
	}

	if (FAILED(device->CreateRenderTargetView(m_Texture2D.Get(), nullptr, m_RTV.ReleaseAndGetAddressOf())))
	{
		return E_FAIL;
	}

	if (FAILED(device->CreateShaderResourceView(m_Texture2D.Get(), nullptr, m_SRV.ReleaseAndGetAddressOf())))
	{
		return E_FAIL;
	}

	m_ClearColor = clearColor;

	return S_OK;
}

engine::SharedPtr<engine::RenderTarget> engine::RenderTarget::Create(const ComPtr<ID3D11Device>& device,
                                                                     const ComPtr<ID3D11DeviceContext>& context, const _uint sizeX, const _uint sizeY, DXGI_FORMAT pixelFormat, const _float4& clearColor)
{
	SharedPtr<RenderTarget> renderTarget{ new RenderTarget(), [](const RenderTarget* ptr) { delete ptr; } };

	if (FAILED(renderTarget->Initialize(device, context, sizeX, sizeY, pixelFormat, clearColor)))
	{
		std::cerr << "ERROR : Failed to Created : RenderTarget\n";
		return nullptr;
	}

	return renderTarget;
}
