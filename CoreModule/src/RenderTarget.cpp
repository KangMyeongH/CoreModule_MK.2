#include "RenderTarget.h"

#include "D3D11Manager.h"
#include "Material.h"

engine::RenderTarget::RenderTarget() = default;

engine::RenderTarget::RenderTarget(const RenderTarget& rhs)
	: Object(rhs), m_Texture2D(rhs.m_Texture2D), m_RTV(rhs.m_RTV), m_SRV(rhs.m_SRV), m_ClearColor(rhs.m_ClearColor)
{

}

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

	D3D11Manager::GetInstance().CreateShader(L"..\\Client\\Assets\\Resource\\Shader\\TextureShader.hlsl", m_Shader);
	m_VIBuffer = D3D11Manager::GetInstance().GetVIBuffer(VIBufferType_POSTEX_RECT);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;  			// 고품질 필터링 (텍스처 왜곡 방지)
	samplerDesc.MaxAnisotropy = 16;  							// 최대 16배 이방성 필터링
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 0.0f;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ComPtr<ID3D11SamplerState> sampler;
	D3D11Manager::GetInstance().CreateSampler(samplerDesc, sampler);

	m_Shader->SetSampler("g_Sampler", sampler);

	return S_OK;
}

void engine::RenderTarget::Clear(ID3D11DeviceContext* context)
{
	context->ClearRenderTargetView(m_RTV.Get(), reinterpret_cast<_float*>(&m_ClearColor));
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

HRESULT engine::RenderTarget::ReadyDebug(_float x, _float y, _float sizeX, _float sizeY)
{
	using namespace DirectX;
	D3D11_VIEWPORT vpDesc{};

	_uint numViewports = { 1 };

	ID3D11DeviceContext* context = D3D11Manager::GetInstance().GetContext().Get();

	context->RSGetViewports(&numViewports, &vpDesc);

	XMStoreFloat4x4(&m_WorldMat, XMMatrixScaling(sizeX, sizeY, 1.f) * XMMatrixTranslation(x - vpDesc.Width * 0.5f, -y + vpDesc.Height * 0.5f, 0.f));

	return S_OK;
}

HRESULT engine::RenderTarget::Render(ID3D11DeviceContext* context)
{
	const _float winSizeX = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeX());
	const _float winSizeY = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeY());

	_float4X4 viewMat, projMat;
	XMStoreFloat4x4(&viewMat, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&projMat, DirectX::XMMatrixOrthographicLH(winSizeX, winSizeY, -1.f, 1.f));

	ID3D11Buffer* vertexBuffers[] = {
	m_VIBuffer->VertexBuffer.Get()
	};

	_uint		vertexStrides[] = {
		m_VIBuffer->VertexStride
	};

	_uint		offsets[] = {
		0,
	};

	context->IASetVertexBuffers(0, m_VIBuffer->NumVertexBuffers, vertexBuffers, vertexStrides, offsets);
	context->IASetIndexBuffer(m_VIBuffer->IndexBuffer.Get(), m_VIBuffer->IndexFormat, 0);
	context->IASetPrimitiveTopology(m_VIBuffer->PrimitiveTopology);

	m_Shader->SetMatrix("g_ViewMatrix", viewMat);
	m_Shader->SetMatrix("g_ProjMatrix", projMat);
	m_Shader->SetMatrix("g_WorldMatrix", m_WorldMat);
	m_Shader->SetTexture("g_Texture", m_SRV);
	m_Shader->Bind(context);
	context->DrawIndexed(m_VIBuffer->NumIndices, 0, 0);

	return S_OK;
}
