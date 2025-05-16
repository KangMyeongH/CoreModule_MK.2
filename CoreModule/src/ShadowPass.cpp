#include "ShadowPass.h"

#include "Camera.h"
#include "RenderManager.h"

HRESULT engine::ShadowPass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	D3D11_TEXTURE2D_DESC td = {};
	td.Width = SHADOW_SIZE;
	td.Height = SHADOW_SIZE;
	td.ArraySize = NUM_CASCADES;
	td.MipLevels = 1;
	td.Format = DXGI_FORMAT_R32_TYPELESS; // ±íÀÌ + »ùÇÃ¸µ °â¿ë
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	td.SampleDesc.Count = 1;
	if (FAILED(device->CreateTexture2D(&td, nullptr, m_ShadowTex.ReleaseAndGetAddressOf())))
	{
		return E_FAIL;
	}

	for (UINT i = 0; i< NUM_CASCADES; ++i)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsDesc = {};
		dsDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsDesc.Texture2DArray.ArraySize = 1;
		dsDesc.Texture2DArray.FirstArraySlice = i;
		if (FAILED(device->CreateDepthStencilView(m_ShadowTex.Get(), &dsDesc, m_ShadowDSV[i].ReleaseAndGetAddressOf())))
		{
			return E_FAIL;
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srcDesc = {};
	srcDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srcDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srcDesc.Texture2DArray.ArraySize = NUM_CASCADES;
	srcDesc.Texture2DArray.MipLevels = 1;
	if (FAILED(device->CreateShaderResourceView(m_ShadowTex.Get(), &srcDesc, m_ShadowSRV.ReleaseAndGetAddressOf())))
	{
		return E_FAIL;
	}

	D3D11_SAMPLER_DESC smpDesc = {};
	smpDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	smpDesc.AddressU = smpDesc.AddressV = smpDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	smpDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	if(FAILED(device->CreateSamplerState(&smpDesc, m_SmpCmpLinear.ReleaseAndGetAddressOf())))
	{
		return E_FAIL;
	}

	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_BACK;
	rd.DepthBias = 500;
	rd.SlopeScaledDepthBias = 2.0f;
	rd.DepthClipEnable = TRUE;
	if (FAILED(device->CreateRasterizerState(&rd, m_RSDepthBias.ReleaseAndGetAddressOf())))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT engine::ShadowPass::Render(ID3D11DeviceContext* context, void* data)
{
	using namespace DirectX;

	auto* passData = static_cast<ShadowPassData*>(data);
	if (!passData)
	{
		return E_INVALIDARG;
	}

	RenderManager* renderManager = &RenderManager::GetInstance();

	_float splits[NUM_CASCADES]{};
	{
		_float n = passData->NearZ;
		_float f = passData->FarZ;
		constexpr _float lambda = 0.95f;
		for (UINT i = 1; i <= NUM_CASCADES; ++i)
		{
			_float si = i / static_cast<_float>(NUM_CASCADES);
			_float logd = n * powf(f / n, si);
			_float unid = n + (f - n) * si;
			splits[i - 1] = lambda * logd + (1.0f - lambda) * unid;
		}
	}

	m_ShadowParams = { 0.001f, 2.0f, 1.5f, 0.0f }; // ±âº» Bias°ª

	for (UINT c = 0; c < NUM_CASCADES; ++c)
	{
		_float prevSplit = (c == 0) ? passData->NearZ : splits[c - 1];
		_float currSplit = splits[c];

		// Ä«¸Þ¶ó ÇÁ·¯½ºÅÒ ÄÚ³Ê (NDC)
		_float3 frustumCorners[8] = {
			{-1.f, 1.f, 0.f}, { 1.f, 1.f, 0.f }, {1.f, -1.f, 0.f}, {-1.f, -1.f, 0.f}, // near
			{-1.f, 1.f, 1.f}, {1.f, 1.f, 1.f}, {1.f, -1.f, 1.f}, { -1.f, -1.f, 1.f} // far
		};
		_matrix invProj = XMMatrixInverse(nullptr, passData->Proj);
		_matrix invView = XMMatrixInverse(nullptr, passData->View);
		for (_int i = 0; i < 8; ++i)
		{
			_vector pt = XMVectorSet(frustumCorners[i].x, frustumCorners[i].y, frustumCorners[i].z, 1.f);
			pt = XMVector3TransformCoord(pt, invProj);
			_float splitDepth = (i < 4) ? prevSplit : currSplit; // near(0-3) vs far(4-7)
			pt = XMVectorScale(pt, splitDepth / passData->FarZ);
			pt = XMVector3TransformCoord(pt, invView);
			XMStoreFloat3(&frustumCorners[i], pt);
		}

		_vector lightDir = XMVector3Normalize(-passData->LightDir);
		_float3 centroid = { 0,0,0 };
		for (const auto& v : frustumCorners)
		{
			centroid.x += v.x;
			centroid.y += v.y;
			centroid.z += v.z;
		}

		centroid.x /= 8;
		centroid.y /= 8;
		centroid.z /= 8;

		_vector eye = XMVectorSet(centroid.x, centroid.y, centroid.z, 1.f);
		_vector at = XMVectorAdd(eye, lightDir);
		_vector up = XMVectorSet(0, 1, 0, 0);
		_matrix lightView = XMMatrixLookAtLH(eye, at, up);

		// AABB in light space
		_vector vMin = XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 1);
		_vector vMax = XMVectorSet(-FLT_MAX,-FLT_MAX, -FLT_MAX, 1);
		for (auto& v : frustumCorners)
		{
			_vector pt = XMLoadFloat3(&v);
			pt = XMVector3TransformCoord(pt, lightView);
			vMin = XMVectorMin(vMin, pt);
			vMax = XMVectorMax(vMax, pt);
		}
		_float minX = XMVectorGetX(vMin);
		_float maxX = XMVectorGetX(vMax);
		_float minY = XMVectorGetY(vMin);
		_float maxY = XMVectorGetY(vMax);
		_float minZ = XMVectorGetZ(vMin);
		_float maxZ = XMVectorGetZ(vMax);

		_matrix lightProj = XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, -maxZ - 100.f, -minZ);

		// Texel snapping
		_float texelSize = (maxX - minX) / static_cast<_float>(SHADOW_SIZE);
		_vector origin = XMVectorSet(minX, minY, 0, 0) / texelSize;
		origin = XMVectorFloor(origin) * texelSize;
		_float offsetX = XMVectorGetX(origin) - minX;
		_float offsetY = XMVectorGetY(origin) - minY;
		lightProj.r[3].m128_f32[0] += offsetX;
		lightProj.r[3].m128_f32[1] += offsetY;

		_matrix lightVP = XMMatrixMultiply(lightView, lightProj);
		XMStoreFloat4x4(&m_LightViewProj[c], XMMatrixTranspose(lightVP));

		// depth-only render to slice c
		context->OMSetRenderTargets(0, nullptr, m_ShadowDSV[c].Get());
		context->RSSetViewports(1, &m_ShadowVP);
		context->RSSetState(m_RSDepthBias.Get());
		context->ClearDepthStencilView(m_ShadowDSV[c].Get(), D3D11_CLEAR_DEPTH, 1.f, 0.f);
		// passData->DepthOnlyDraw(context);

	}

	m_CascadeSplits = { splits[0], splits[1], splits[2], splits[3] };

	context->RSSetState(nullptr);

	return S_OK;
}

HRESULT engine::ShadowPass::RenderEditor(ID3D11DeviceContext* context, void* data, _bool isGame)
{
	return S_OK;
}

void engine::ShadowPass::Release()
{
}
