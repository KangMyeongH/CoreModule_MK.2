#include "DebugRenderManager.h"

#include "BoxCollider.h"
#include "Collider.h"
#include "Material.h"
#include "SphereCollider.h"

IMPLEMENT_SINGLETON(engine::DebugRenderManager)

engine::DebugRenderManager::DebugRenderManager() = default;
engine::DebugRenderManager::~DebugRenderManager() = default;

void engine::DebugRenderManager::Initialize(const ComPtr<ID3D11Device>& device)
{
	createCapsuleWireframe(device);
	createOBBWireFrameVertices(device);
	createSphereWireframe(device);

	m_ColliderMaterial = Material::Create(nullptr);
	m_ColliderMaterial->LoadShader(L"..\\GameEngine\\resource\\Shader\\Collider.hlsl");
}

void engine::DebugRenderManager::RenderCollider(const std::vector<SharedPtr<Collider>>& colliders,
                                                const ComPtr<ID3D11DeviceContext>& context, const _float4X4& viewMat, const _float4X4& projMat)
{
	for (auto& col : colliders)
	{
		if (col->IsEnabled())
		{
			switch (col->GetColliderType())
			{
			case ColliderType_Sphere:
			{
				auto sphereCol = std::static_pointer_cast<SphereCollider>(col);
				ID3D11Buffer* vertexBuffers[] = {
					m_SphereVIBuffer->VertexBuffer.Get()
				};

				_uint vertexStrides[] = {
					m_SphereVIBuffer->VertexStride
				};

				_uint offsets[] = {
					0
				};

				// Sphere의 WorldMat에서 Scale은 uniformScale로 세팅해야됨.

				auto localCenter = sphereCol->GetCenter();
				_float3 worldScale = sphereCol->GetTransform()->Scale().Value;
				_float uniformScale = std::max({ std::abs(worldScale.x), std::abs(worldScale.y), std::abs(worldScale.z) });
				_matrix scaleMat = DirectX::XMMatrixScaling(uniformScale, uniformScale, uniformScale);
				_matrix rotMat = DirectX::XMMatrixRotationQuaternion(sphereCol->GetTransform()->Rotation().ToVector());
				_matrix posMat = DirectX::XMMatrixTranslationFromVector(sphereCol->GetTransform()->Position().ToVector());
				_matrix worldMat = scaleMat * rotMat * posMat;
				_matrix localPos = DirectX::XMMatrixTranslationFromVector(localCenter.ToVector());
				_float uniformLocalScale = sphereCol->GetRadius() * 0.5f;
				_matrix localScale = DirectX::XMMatrixScaling(uniformLocalScale, uniformLocalScale, uniformLocalScale);
				_matrix localMat = localScale * localPos;
				_matrix finalMat = localMat * worldMat;
				_float4 color{};
				if (col->IsHit())
				{
					color = { 1.f, 0.f, 0.f, 1.f };
				}

				else
				{
					color = { 0.f, 1.f, 0.f, 1.f };
				}

				m_ColliderMaterial->SetFloat4("g_Color", color);
				m_ColliderMaterial->SetMatrix("g_ViewMatrix", viewMat);
				m_ColliderMaterial->SetMatrix("g_ProjMatrix", projMat);
				m_ColliderMaterial->SetMatrix("g_WorldMatrix", worldMat);
				m_ColliderMaterial->Bind(context.Get());

				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
				context->IASetVertexBuffers(0, m_SphereVIBuffer->NumVertexBuffers, vertexBuffers, vertexStrides, offsets);
				context->IASetIndexBuffer(m_SphereVIBuffer->IndexBuffer.Get(), m_SphereVIBuffer->IndexFormat, 0);
				context->DrawIndexed(m_SphereVIBuffer->NumIndices, 0, 0);

			}
			break;
			case ColliderType_Box:
			{
				auto boxCol = std::static_pointer_cast<BoxCollider>(col);

				ID3D11Buffer* vertexBuffers[] = {
					m_BoxVIBuffer->VertexBuffer.Get()
				};

				_uint		vertexStrides[] = {
					m_BoxVIBuffer->VertexStride
				};

				_uint		offsets[] = {
					0,
				};

				auto localCenter = boxCol->GetCenter().ToVector();
				auto localScale = boxCol->GetSize().ToVector();
				_matrix localP = DirectX::XMMatrixTranslationFromVector(localCenter);
				_matrix localS = DirectX::XMMatrixScalingFromVector(localScale);
				_matrix localMat = localS * localP;
				_matrix worldMat = localMat * col->GetTransform()->GetWorldMatrix();

				_float4 color{};

				if (col->IsHit())
				{
					color = { 1.f, 0.f, 0.f, 1.f };
				}

				else if (col->IsBoardHit())
				{
					color = { 0.f, 0.f, 1.f, 1.f };
				}

				else
				{
					color = { 0.f, 1.f, 0.f, 1.f };
				}

				m_ColliderMaterial->SetFloat4("g_Color", color);
				m_ColliderMaterial->SetMatrix("g_ViewMatrix", viewMat);
				m_ColliderMaterial->SetMatrix("g_ProjMatrix", projMat);
				m_ColliderMaterial->SetMatrix("g_WorldMatrix", worldMat);
				m_ColliderMaterial->Bind(context.Get());

				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
				context->IASetVertexBuffers(0, m_BoxVIBuffer->NumVertexBuffers, vertexBuffers, vertexStrides, offsets);
				context->IASetIndexBuffer(m_BoxVIBuffer->IndexBuffer.Get(), m_BoxVIBuffer->IndexFormat, 0);
				context->DrawIndexed(m_BoxVIBuffer->NumIndices, 0, 0);
			}
			break;
			case ColliderType_Capsule:
				break;
			case ColliderType_Mesh:
				break;
			}
		}
	}
}

void engine::DebugRenderManager::Release()
{
	m_BoxVIBuffer = nullptr;
	m_SphereVIBuffer = nullptr;
	m_CapsuleVIBuffer = nullptr;
	m_ColliderMaterial = nullptr;
}

HRESULT engine::DebugRenderManager::createOBBWireFrameVertices(const ComPtr<ID3D11Device>& device)
{
	_float3 verts[8] =
	{
		{+0.5f, +0.5f, +0.5f}, {-0.5f, +0.5f, +0.5f}, {-0.5f, -0.5f, +0.5f}, {+0.5f, -0.5f, +0.5f},
		{+0.5f, +0.5f, -0.5f}, {-0.5f, +0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {+0.5f, -0.5f, -0.5f},
	};

	std::vector<DebugVertex> vb;

	vb.resize(8);

	for (int i = 0; i < 8; ++i)
	{
		vb[i].Position = verts[i];
	}

	_ushort indices[] =
	{
		0,1, 1,2, 2,3, 3,0, // 상단
		4,5, 5,6, 6,7, 7,4, // 하단
		0,4, 1,5, 2,6, 3,7  // 연결
	};

	auto viBuffer = std::make_shared<VIBuffer>();

	viBuffer->NumVertexBuffers = 1;
	viBuffer->VertexStride = sizeof(DebugVertex);
	viBuffer->NumVertices = 8;
	viBuffer->IndexStride = 2;
	viBuffer->NumIndices = 24;
	viBuffer->IndexFormat = DXGI_FORMAT_R16_UINT;
	viBuffer->PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = viBuffer->VertexStride * viBuffer->NumVertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = viBuffer->VertexStride;

	D3D11_SUBRESOURCE_DATA initDesc{};
	initDesc.pSysMem = vb.data();

	HRESULT hr = device->CreateBuffer(&vbd, &initDesc, viBuffer->VertexBuffer.GetAddressOf());

	D3D11_BUFFER_DESC ibd{};
	ibd.ByteWidth = viBuffer->IndexStride * viBuffer->NumIndices;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.StructureByteStride = viBuffer->IndexStride;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;

	ZeroMemory(&initDesc, sizeof(initDesc));
	initDesc.pSysMem = indices;

	hr = device->CreateBuffer(&ibd, &initDesc, viBuffer->IndexBuffer.GetAddressOf());

	m_BoxVIBuffer = viBuffer;

	return hr;
}

HRESULT engine::DebugRenderManager::createSphereWireframe(const ComPtr<ID3D11Device>& device)
{
	std::vector<DebugVertex> vb;
	std::vector<_ushort> ib;

	const int slices = 16;

	for (int i = 0; i < slices; ++i)
	{
		_float theta = DirectX::XM_2PI * static_cast<_float>(i) / static_cast<_float>(slices);
		_float cosT = cosf(theta);
		_float sinT = sinf(theta);

		DebugVertex xy{ {cosT * 0.5f, sinT * 0.5f, 0.f } };
		DebugVertex yz{ { 0.f, cosT * 0.5f, sinT * 0.5f } };
		DebugVertex xz{ { cosT * 0.5f, 0.f, sinT * 0.5f } };

		vb.push_back(xy);
		vb.push_back(yz);
		vb.push_back(xz);

		if (i > 0)
		{
			_ushort base = i * 3;
			ib.push_back(base - 3); ib.push_back(base);
			ib.push_back(base - 2); ib.push_back(base + 1);
			ib.push_back(base - 1); ib.push_back(base + 2);
		}
	}

	_ushort last = (slices - 1) * 3;
	ib.push_back(last); ib.push_back(0);
	ib.push_back(last + 1); ib.push_back(1);
	ib.push_back(last + 2); ib.push_back(2);

	auto viBuffer = std::make_shared<VIBuffer>();

	viBuffer->NumVertexBuffers = 1;
	viBuffer->VertexStride = sizeof(DebugVertex);
	viBuffer->NumVertices = vb.size();
	viBuffer->IndexStride = 2;
	viBuffer->NumIndices = ib.size();
	viBuffer->IndexFormat = DXGI_FORMAT_R16_UINT;
	viBuffer->PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = viBuffer->VertexStride * viBuffer->NumVertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = viBuffer->VertexStride;

	D3D11_SUBRESOURCE_DATA initDesc{};
	initDesc.pSysMem = vb.data();

	HRESULT hr = device->CreateBuffer(&vbd, &initDesc, viBuffer->VertexBuffer.GetAddressOf());

	D3D11_BUFFER_DESC ibd{};
	ibd.ByteWidth = viBuffer->IndexStride * viBuffer->NumIndices;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.StructureByteStride = viBuffer->IndexStride;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;

	ZeroMemory(&initDesc, sizeof(initDesc));
	initDesc.pSysMem = ib.data();

	hr = device->CreateBuffer(&ibd, &initDesc, viBuffer->IndexBuffer.GetAddressOf());

	m_SphereVIBuffer = viBuffer;

	return hr;
}

HRESULT engine::DebugRenderManager::createCapsuleWireframe(const ComPtr<ID3D11Device>& device)
{
	std::vector<DebugVertex> vb;
	std::vector<_ushort> ib;

	const int segments = 16;
	const float radius = 0.5f;
	const float halfHeight = 1.0f;

	for (int i = 0; i < segments; ++i)
	{
		float angle = DirectX::XM_2PI * i / segments;
		float x = cosf(angle) * radius;
		float z = sinf(angle) * radius;

		DirectX::XMFLOAT3 bottom = { x, -halfHeight, z };
		DirectX::XMFLOAT3 top = { x, +halfHeight, z };

		vb.push_back({ bottom });
		vb.push_back({ top });

		_ushort base = static_cast<_ushort>(vb.size()) - 2;
		ib.push_back(base);
		ib.push_back(base + 1);
	}

	_ushort topStart = static_cast<_ushort>(vb.size());
	for (int i = 0; i < segments; ++i)
	{
		float angle = DirectX::XM_2PI * i / segments;
		float x = cosf(angle) * radius;
		float z = sinf(angle) * radius;
		vb.push_back({ { x, +halfHeight, z } });
	}

	for (int i = 0; i < segments; ++i)
	{
		_ushort a = topStart + i;
		_ushort b = topStart + ((i + 1) % segments);
		ib.push_back(a);
		ib.push_back(b);
	}

	_ushort bottomStart = static_cast<_ushort>(vb.size());
	for (int i = 0; i < segments; ++i)
	{
		float angle = DirectX::XM_2PI * i / segments;
		float x = cosf(angle) * radius;
		float z = sinf(angle) * radius;
		vb.push_back({ { x, -halfHeight, z } });
	}

	for (int i = 0; i < segments; ++i)
	{
		_ushort a = bottomStart + i;
		_ushort b = bottomStart + ((i + 1) % segments);
		ib.push_back(a);
		ib.push_back(b);
	}

	auto viBuffer = std::make_shared<VIBuffer>();
	viBuffer->NumVertexBuffers = 1;
	viBuffer->VertexStride = sizeof(DebugVertex);
	viBuffer->NumVertices = static_cast<UINT>(vb.size());
	viBuffer->IndexStride = sizeof(_ushort);
	viBuffer->NumIndices = static_cast<UINT>(ib.size());
	viBuffer->IndexFormat = DXGI_FORMAT_R16_UINT;
	viBuffer->PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

	// VertexBuffer
	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = viBuffer->VertexStride * viBuffer->NumVertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.StructureByteStride = viBuffer->VertexStride;

	D3D11_SUBRESOURCE_DATA vbData{ vb.data() };
	HRESULT hr = device->CreateBuffer(&vbd, &vbData, viBuffer->VertexBuffer.GetAddressOf());
	if (FAILED(hr)) return hr;

	// IndexBuffer
	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = viBuffer->IndexStride * viBuffer->NumIndices;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.StructureByteStride = viBuffer->IndexStride;

	D3D11_SUBRESOURCE_DATA ibData{ ib.data() };
	hr = device->CreateBuffer(&ibd, &ibData, viBuffer->IndexBuffer.GetAddressOf());
	if (FAILED(hr)) return hr;

	m_CapsuleVIBuffer = viBuffer;

	return S_OK;
}
