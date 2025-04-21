#include "CapsuleRenderer.h"

#include <sstream>

#include "D3D11Manager.h"
#include "Material.h"

engine::CapsuleRenderer::CapsuleRenderer()
{
}

engine::CapsuleRenderer::~CapsuleRenderer()
{
}

void engine::CapsuleRenderer::Bind(const ComPtr<ID3D11DeviceContext>& context, const _float4X4& view,
	const _float4X4& proj)
{
	m_Batch.clear();

	m_Material->SetMatrix("g_ViewMatrix", view);
	m_Material->SetMatrix("g_ProjMatrix", proj);
}

void engine::CapsuleRenderer::Render(const ComPtr<ID3D11DeviceContext>& context, const _vector& center,
	const Capsule& capsule)
{
}

void engine::CapsuleRenderer::AddLine(const _float3& a, const _float3& b, const _float4& color)
{
}

HRESULT engine::CapsuleRenderer::CreateDynamicVB(const ComPtr<ID3D11Device>& device, UINT maxVerts)
{
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(DebugVertex) * maxVerts;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	return device->CreateBuffer(&bd, nullptr, m_VTXBuffer.GetAddressOf());
}

HRESULT engine::CapsuleRenderer::compileShaderFromFile(const _wstring& path, const _string& entryPoint,
	const _string& targetProfile, ComPtr<ID3DBlob>& outBlob)
{
	ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), targetProfile.c_str(), 0, 0, outBlob.GetAddressOf(), errorBlob.GetAddressOf());

	if (FAILED(hr))
	{
		std::stringstream ss;
		ss << "Failed to compile shader from file : " << WStringToString(path) << "\n";
		std::cerr << ss.str().c_str();

		if (errorBlob)
		{
			std::cerr << static_cast<const char*>(errorBlob->GetBufferPointer());
		}
		return hr;
	}

	return hr;
}

HRESULT engine::CapsuleRenderer::Initialize(const ComPtr<ID3D11Device>& device)
{
	HRESULT hr = CreateDynamicVB(device, 32768);
	if (FAILED(hr))
	{
		return hr;
	}

	m_Material = Material::Create(nullptr);
	m_Material->LoadShader(L".\\GameEngine\\resource\\Shader\\Collider.hlsl");
}
