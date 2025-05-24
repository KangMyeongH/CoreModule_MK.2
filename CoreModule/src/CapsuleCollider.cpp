#include "CapsuleCollider.h"

DEFINE_REGISTER_COMPONENT(CapsuleCollider)

engine::CapsuleCollider::CapsuleCollider(const SharedPtr<GameObject>& owner, const _string& name)
	: Collider(owner, name), m_Capsule(), m_Radius(0.5f), m_Height(2.f), m_Dir(1)
{

}

engine::CapsuleCollider::CapsuleCollider(const CapsuleCollider& rhs)
	: Collider(rhs), m_Capsule(), m_Radius(rhs.m_Radius), m_Height(rhs.m_Height), m_Dir(rhs.m_Dir)
{
}

engine::Vector3 engine::CapsuleCollider::GetWorldCenter() const
{
	return m_Capsule.CenterW;
}

void engine::CapsuleCollider::UpdateCollider()
{
	calcWorldCapsule();
	calcWorldABB();
}

void engine::CapsuleCollider::Render(ComPtr<ID3D11DeviceContext> context, const SharedPtr<VIBuffer>& buffer)
{
	// TODO : capsule Debug render 추가해야함.
	// Debug Renderer Manager에 해당 기능 할당 중임.
}

void engine::CapsuleCollider::Destroy()
{
	m_bDestroyed = true;
}

engine::Capsule engine::CapsuleCollider::calcWorldCapsule()
{
	using namespace DirectX;

	Capsule capsule;

	auto transform = GetTransform();
	_vector S = transform->Scale().ToVector();
	_vector Rq = transform->Rotation().ToVector();
	_vector P = transform->Position().ToVector();

	_matrix SMat = XMMatrixScalingFromVector(S);
	_matrix RMat = XMMatrixRotationQuaternion(Rq);
	_matrix SR = SMat * RMat;
	_matrix worldMat = transform->GetWorldMatrix();

	static const _vector dirLUT[3] =
	{
		XMVectorSet(1.f,0.f,0.f,0.f),
		XMVectorSet(0.f,1.f,0.f,0.f),
		XMVectorSet(0.f, 0.f,1.f,0.f)
	};

	_int dir = m_Dir;
	_vector axisL = dirLUT[dir];

	_float sx = transform->Scale().Value.x, sy = transform->Scale().Value.y, sz = transform->Scale().Value.z;
	_float axisScale = (dir == 0) ? sx : (dir == 1) ? sy : sz;
	_float radialScale = std::max({ (dir == 0) ? sy : sx, (dir == 1) ? sz : sy, (dir == 2) ? sx : sz });

	_float radiusW = m_Radius * radialScale;

	_float cylHalfLocal = std::max(m_Height - 2.f * m_Radius, 0.f) * 0.5f;
	_float halfHeightW = cylHalfLocal * axisScale;

	_vector centerL = m_Center.ToVector();
	_vector centerW = XMVector3Transform(centerL, worldMat);
	_vector axisW = XMVector3Normalize(XMVector3TransformNormal(axisL, RMat));

	_vector p0 = centerW + axisW * halfHeightW;
	_vector p1 = centerW - axisW * halfHeightW;

	capsule.CenterW = centerW;
	capsule.AxisW = axisW;
	capsule.P0W = p0;
	capsule.P1W = p1;
	capsule.HalfHeight = halfHeightW;
	capsule.Radius = radiusW;

	m_Capsule = capsule;

	return capsule;
}

void engine::CapsuleCollider::calcWorldABB()
{
	AABB aabb;

	_vector absAxis = DirectX::XMVectorAbs(m_Capsule.AxisW.ToVector());
	Vector3 vAbsAxis = Vector3::FromVector(absAxis);
	Vector3 e = vAbsAxis * m_Capsule.HalfHeight + Vector3::FromVector(DirectX::XMVectorReplicate(m_Capsule.Radius));

	Vector3 boxMin = m_Capsule.CenterW - e;
	Vector3 boxMax = m_Capsule.CenterW + e;

	aabb.Min = boxMin;
	aabb.Max = boxMax;


	//_vector topCenter = m_Capsule.TopCenter.ToVector();
	//_vector bottomCenter = m_Capsule.BottomCenter.ToVector();

	//_vector minPoint = DirectX::XMVectorMin(topCenter, bottomCenter);
	//_vector maxPoint = DirectX::XMVectorMax(topCenter, bottomCenter);

	//_vector extents = DirectX::XMVectorSet(m_Capsule.Radius, m_Capsule.Radius, m_Capsule.Radius, 0.f);

	//aabb.Min = Vector3::FromVector(minPoint) - Vector3::FromVector(extents);
	//aabb.Max = Vector3::FromVector(maxPoint) - Vector3::FromVector(extents);

	m_AABB = aabb;
}

void engine::CapsuleCollider::to_json(nlohmann::ordered_json& j)
{
	_string type = "CapsuleCollider";
	j = nlohmann::ordered_json{
		{"type", type},
		{"enable", m_bEnabled},
		{"isTrigger", m_bTrigger},
		{"center", m_Center},
		{"radius", m_Radius},
		{"height", m_Height},
		{"dir", m_Dir}
	};
}

void engine::CapsuleCollider::from_json(const nlohmann::ordered_json& j)
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

	if (j.contains("radius"))
	{
		j.at("radius").get_to(m_Radius);
	}

	if (j.contains("height"))
	{
		j.at("height").get_to(m_Height);
	}

	if (j.contains("dir"))
	{
		j.at("dir").get_to(m_Dir);
	}
}
