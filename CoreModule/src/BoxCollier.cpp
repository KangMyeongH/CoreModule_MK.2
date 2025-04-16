#include "BoxCollier.h"

DEFINE_REGISTER_COMPONENT(BoxCollier)

engine::BoxCollier::BoxCollier(const SharedPtr<GameObject>& owner, const _string& name)
	: Collider(owner, name), m_Size(1.f, 1.f, 1.f)
{

}

engine::BoxCollier::BoxCollier(const BoxCollier& rhs)
	: Collider(rhs), m_Center(rhs.m_Center), m_Size(rhs.m_Size)
{

}

void engine::BoxCollier::UpdateCollider()
{
	calcWorldABB();
	calcWorldOBB();
}

void engine::BoxCollier::Render(ComPtr<ID3D11DeviceContext> context)
{

}

void engine::BoxCollier::Destroy()
{
	m_bDestroyed = true;
}

engine::OBB engine::BoxCollier::calcWorldOBB()
{
	OBB obb;

	_matrix worldMat = GetTransform()->GetWorldMatrix();

	_vector colX = DirectX::XMVectorSet(worldMat.r[0].m128_f32[0], worldMat.r[1].m128_f32[0], worldMat.r[2].m128_f32[0], 0.f); // _11, _21, _31
	_vector colY = DirectX::XMVectorSet(worldMat.r[0].m128_f32[1], worldMat.r[1].m128_f32[1], worldMat.r[2].m128_f32[1], 0.f); // _12, _22, _32
	_vector colZ = DirectX::XMVectorSet(worldMat.r[0].m128_f32[2], worldMat.r[1].m128_f32[2], worldMat.r[2].m128_f32[2], 0.f); // _13, _23, _33

	float sx = DirectX::XMVectorGetX(DirectX::XMVector3Length(colX));
	float sy = DirectX::XMVectorGetX(DirectX::XMVector3Length(colY));
	float sz = DirectX::XMVectorGetX(DirectX::XMVector3Length(colZ));

	_vector axisX = (sx > 1e-6f) ? DirectX::XMVectorScale(colX, 1.0f / sx) : DirectX::XMVectorZero();
	_vector axisY = (sy > 1e-6f) ? DirectX::XMVectorScale(colY, 1.0f / sy) : DirectX::XMVectorZero();
	_vector axisZ = (sz > 1e-6f) ? DirectX::XMVectorScale(colZ, 1.0f / sz) : DirectX::XMVectorZero();

	obb.AxisX = axisX;
	obb.AxisY = axisY;
	obb.AxisZ = axisZ;

	_vector center = m_Center.ToVector();
	_vector transformedCenter = XMVector3Transform(center, worldMat);
	// World 기준 Center 계산
	obb.Center = transformedCenter;

	obb.Extents.Value.x = m_Size.Value.x * sx * 0.5f;
	obb.Extents.Value.y = m_Size.Value.y * sy * 0.5f;
	obb.Extents.Value.z = m_Size.Value.z * sz * 0.5f;

	m_OBB = obb;

	return obb;
}

void engine::BoxCollier::calcWorldABB()
{
	_matrix worldMat = GetTransform()->GetWorldMatrix();

	_vector localCenter = m_Center.ToVector();
	_vector localExt = (m_Size * 0.5f).ToVector();

	_vector worldCenter = XMVector3TransformCoord(localCenter, worldMat);

	_float4X4 mat;
	XMStoreFloat4x4(&mat, worldMat);

	_vector row0 = DirectX::XMVectorSet(fabsf(mat._11), fabsf(mat._12), fabsf(mat._13), 0.f);
	_vector row1 = DirectX::XMVectorSet(fabsf(mat._21), fabsf(mat._22), fabsf(mat._23), 0.f);
	_vector row2 = DirectX::XMVectorSet(fabsf(mat._31), fabsf(mat._32), fabsf(mat._33), 0.f);

	_float ex = DirectX::XMVectorGetX(DirectX::XMVector3Dot(row0, localExt));
	_float ey = DirectX::XMVectorGetX(DirectX::XMVector3Dot(row1, localExt));
	_float ez = DirectX::XMVectorGetX(DirectX::XMVector3Dot(row2, localExt));

	_vector newExtents = DirectX::XMVectorSet(ex, ey, ez, 0.f);

	_vector minVec = DirectX::XMVectorSubtract(worldCenter, newExtents);
	_vector maxVec = DirectX::XMVectorAdd(worldCenter, newExtents);

	AABB aabb;

	aabb.Min = minVec;
	aabb.Max = maxVec;

	m_AABB = aabb;
}

void engine::BoxCollier::to_json(nlohmann::ordered_json& j)
{
	_string type = "BoxCollider";
	j = nlohmann::ordered_json{
		{"type", type},
		{"enable", m_bEnabled},
		{"isTrigger", m_bTrigger},
		{"center", m_Center},
		{"size", m_Size}
	};
}

void engine::BoxCollier::from_json(const nlohmann::ordered_json& j)
{
	if (j.contains("enable"))
	{
		j.at("enable").get_to(m_bEnabled);
	}

	if (j.contains("isTrigger"))
	{
		j.at("isTrigger").get_to(m_bTrigger);
	}

	if (j.contains("center"))
	{
		j.at("center").get_to(m_Center);
	}

	if (j.contains("size"))
	{
		j.at("size").get_to(m_Size);
	}
}
