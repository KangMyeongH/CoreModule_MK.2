#include "BoxCollider.h"

DEFINE_REGISTER_COMPONENT(BoxCollider)

engine::BoxCollider::BoxCollider(const SharedPtr<GameObject>& owner, const _string& name)
	: Collider(owner, name), m_Size(1.f, 1.f, 1.f)
{

}

engine::BoxCollider::BoxCollider(const BoxCollider& rhs)
	: Collider(rhs), m_Center(rhs.m_Center), m_Size(rhs.m_Size)
{

}

engine::Vector3 engine::BoxCollider::GetWorldCenter() const
{
	_matrix worldMat = GetTransform()->GetWorldMatrix();

	_vector centerVec = m_Center.ToVector();
	_vector worldCenterVec = DirectX::XMVector3Transform(centerVec, worldMat);

	return Vector3::FromVector(worldCenterVec);
}

void engine::BoxCollider::UpdateCollider()
{
	calcWorldOBB();
	calcWorldABB();
}

void engine::BoxCollider::Render(ComPtr<ID3D11DeviceContext> context, const SharedPtr<VIBuffer>& buffer)
{

}

void engine::BoxCollider::Destroy()
{
	m_bDestroyed = true;
}

engine::OBB engine::BoxCollider::calcWorldOBB()
{
	using namespace DirectX;

	auto transform = GetTransform();

	OBB obb;

	_matrix scaleMat = XMMatrixScalingFromVector(transform->Scale().ToVector());
	_matrix rotationMat = XMMatrixRotationQuaternion(transform->Rotation().ToVector());
	_matrix positionMat = XMMatrixTranslationFromVector(transform->Position().ToVector());
	_matrix worldMat = transform->GetWorldMatrix();

	// 로컬 중심을 월드로
	_vector centerL = m_Center.ToVector();
	_vector centerW = XMVector3Transform(centerL, worldMat);   // (S*R)*center + T

	// 월드 축 (S * R 의 열벡터 길이를 스케일까지 포함해서 얻음)
	XMFLOAT3 axis[3];
	XMFLOAT3 half;
	const XMVECTOR halfL = XMVectorSet(m_Size.Value.x * 0.5f,
		m_Size.Value.y * 0.5f,
		m_Size.Value.z * 0.5f,
		0.0f);

	// S * R 부분만 분리
	XMMATRIX SR = scaleMat * rotationMat;

	XMFLOAT3 axisArr[3];
	XMFLOAT3 halfW;  // 결과 half-extent

	for (int i = 0; i < 3; ++i)
	{
		// SR 열벡터 = 스케일이 포함된 월드 축
		XMVECTOR col = XMVectorSet(SR.r[0].m128_f32[i],
			SR.r[1].m128_f32[i],
			SR.r[2].m128_f32[i],
			0.0f);

		float len = XMVectorGetX(XMVector3Length(col)); // 축 길이(=스케일)
		XMVECTOR n = XMVectorScale(col, 1.0f / len);     // 정규화

		XMStoreFloat3(&axisArr[i], n);                    // 축 저장
		// halfW[i] = halfL[i] * len
		reinterpret_cast<float*>(&halfW)[i] = reinterpret_cast<const float*>(&halfL)[i] * len;
	}

	obb.AxisX = XMLoadFloat3(&axisArr[0]);
	obb.AxisY = XMLoadFloat3(&axisArr[1]);
	obb.AxisZ = XMLoadFloat3(&axisArr[2]);
	obb.Center = centerW;
	obb.Extents = Vector3(halfW);

	m_OBB = obb;

	return obb;
}

void engine::BoxCollider::calcWorldABB()
{
	using namespace DirectX;

	AABB aabb{};

	_vector c = m_OBB.Center.ToVector();

	XMMATRIX AbsR;
	_float3 axisW[3];
	axisW[0] = m_OBB.AxisX.Value;
	axisW[1] = m_OBB.AxisY.Value;
	axisW[2] = m_OBB.AxisZ.Value;

	for (int row = 0; row < 3; ++row)
	{
		for (int col = 0; col < 3; ++col)
		{
			AbsR.r[row].m128_f32[col] = std::fabs((&axisW[col].x)[row]); // 축 벡터 전개	
		}
		AbsR.r[row].m128_f32[3] = 0.0f;
	}

	XMVECTOR e = XMVectorSet(
		XMVectorGetX(AbsR.r[0]) * m_OBB.Extents.Value.x +
		XMVectorGetX(AbsR.r[1]) * m_OBB.Extents.Value.y +
		XMVectorGetX(AbsR.r[2]) * m_OBB.Extents.Value.z,

		XMVectorGetY(AbsR.r[0]) * m_OBB.Extents.Value.x +
		XMVectorGetY(AbsR.r[1]) * m_OBB.Extents.Value.y +
		XMVectorGetY(AbsR.r[2]) * m_OBB.Extents.Value.z,

		XMVectorGetZ(AbsR.r[0]) * m_OBB.Extents.Value.x +
		XMVectorGetZ(AbsR.r[1]) * m_OBB.Extents.Value.y +
		XMVectorGetZ(AbsR.r[2]) * m_OBB.Extents.Value.z,
		0.0f);

	_vector minV = c - e;
	_vector maxV = c + e;

	aabb.Min = Vector3::FromVector(minV);
	aabb.Max = Vector3::FromVector(maxV);

	m_AABB = aabb;
}

void engine::BoxCollider::to_json(nlohmann::ordered_json& j)
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

void engine::BoxCollider::from_json(const nlohmann::ordered_json& j)
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
