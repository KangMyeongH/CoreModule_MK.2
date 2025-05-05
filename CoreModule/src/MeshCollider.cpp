#include "MeshCollider.h"

#include "D3D11Manager.h"
#include "LoadManager.h"

DEFINE_REGISTER_COMPONENT(MeshCollider)

engine::MeshCollider::MeshCollider(const SharedPtr<GameObject>& owner, const _string& name)
	: Collider(owner, name), m_Mesh(), m_MeshIdx(-1), m_bDirty(false)
{
}

engine::MeshCollider::MeshCollider(const MeshCollider& rhs)
	: Collider(rhs), m_Mesh(rhs.m_Mesh), m_Path(rhs.m_Path), m_MeshIdx(rhs.m_MeshIdx), m_bDirty(false)
{
}

void engine::MeshCollider::SetMesh(const _wstring& modelPath, const _int meshIdx)
{
	auto modelData = LoadManager::GetInstance().ReadModelDataFromFile(modelPath);

	m_Mesh = modelData.Meshes[meshIdx];
	m_Path = modelPath;
	m_MeshIdx = meshIdx;
	m_bDirty = true;

	m_DebugVIBuffer = std::make_shared<VIBuffer>();
	std::vector<DebugVertex> vb;

	vb.reserve(m_Mesh.Vertices.size());
	for (const auto& v : m_Mesh.Vertices)
	{
		vb.push_back({ v.Position });
	}

	auto viBuffer = m_Mesh.VIBuffer;

	m_DebugVIBuffer->NumVertexBuffers = 1;
	m_DebugVIBuffer->VertexStride = sizeof(DebugVertex);
	m_DebugVIBuffer->NumVertices = viBuffer->NumVertices;
	m_DebugVIBuffer->IndexStride = viBuffer->IndexStride;
	m_DebugVIBuffer->NumIndices = viBuffer->NumIndices;
	m_DebugVIBuffer->IndexFormat = viBuffer->IndexFormat;
	m_DebugVIBuffer->PrimitiveTopology = viBuffer->PrimitiveTopology;

	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = m_DebugVIBuffer->VertexStride * m_DebugVIBuffer->NumVertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = m_DebugVIBuffer->VertexStride;

	D3D11_SUBRESOURCE_DATA initDesc{};
	initDesc.pSysMem = vb.data();

	HRESULT hr = D3D11Manager::GetInstance().GetDevice()->CreateBuffer(&vbd, &initDesc, m_DebugVIBuffer->VertexBuffer.ReleaseAndGetAddressOf());

	m_DebugVIBuffer->IndexBuffer = viBuffer->IndexBuffer;
}

void engine::MeshCollider::UpdateCollider()
{
	if (m_bDirty)
	{
		calcWorldABB();
	}
}

void engine::MeshCollider::Render(ComPtr<ID3D11DeviceContext> context, const SharedPtr<VIBuffer>& buffer)
{
}

void engine::MeshCollider::Destroy()
{
}

void engine::MeshCollider::calcWorldABB()
{
	using namespace DirectX;

	AABB localAABB;

	_float3 lmn = { FLT_MAX, FLT_MAX,FLT_MAX };
	_float3 lmx = { FLT_MIN, FLT_MIN,FLT_MIN };

	for (auto& v : m_Mesh.Vertices)
	{
		_float3 p = v.Position;
		lmn.x = std::min(lmn.x, p.x);
		lmn.y = std::min(lmn.y, p.y);
		lmn.z = std::min(lmn.z, p.z);

		lmx.x = std::max(lmx.x, p.x);
		lmx.y = std::max(lmx.y, p.y);
		lmx.z = std::max(lmx.z, p.z);
	}

	localAABB.Min = Vector3(lmn);
	localAABB.Max = Vector3(lmx);

	_vector corners[8];
	_vector mn = localAABB.Min.ToVector();
	_vector mx = localAABB.Max.ToVector();
	_matrix worldMatrix = GetTransform()->GetWorldMatrix();

	corners[0] = { mn.m128_f32[0], mn.m128_f32[1], mn.m128_f32[2], 1 };
	corners[1] = { mx.m128_f32[0], mn.m128_f32[1], mn.m128_f32[2], 1 };
	corners[2] = { mn.m128_f32[0], mx.m128_f32[1], mn.m128_f32[2], 1 };
	corners[3] = { mn.m128_f32[0], mn.m128_f32[1], mx.m128_f32[2], 1 };
	corners[4] = { mx.m128_f32[0], mx.m128_f32[1], mn.m128_f32[2], 1 };
	corners[5] = { mx.m128_f32[0], mn.m128_f32[1], mx.m128_f32[2], 1 };
	corners[6] = { mn.m128_f32[0], mx.m128_f32[1], mx.m128_f32[2], 1 };
	corners[7] = { mx.m128_f32[0], mx.m128_f32[1], mx.m128_f32[2], 1 };

	_vector w0 = XMVector3TransformCoord(corners[0], worldMatrix);
	_float3 wmn, wmx;
	XMStoreFloat3(&wmn, w0);
	wmx = wmn;

	for (_int i = 1; i < 8; ++i)
	{
		_vector w = XMVector3TransformCoord(corners[i], worldMatrix);
		_float3 fw;
		XMStoreFloat3(&fw, w);

		wmn.x = std::min(wmn.x, fw.x);
		wmn.y = std::min(wmn.y, fw.y);
		wmn.z = std::min(wmn.z, fw.z);

		wmx.x = std::max(wmx.x, fw.x);
		wmx.y = std::max(wmx.y, fw.y);
		wmx.z = std::max(wmx.z, fw.z);
	}

	m_AABB.Min = Vector3(wmn);
	m_AABB.Max = Vector3(wmx);

	m_bDirty = false;
}
