#include "CapsuleCollider.h"

DEFINE_REGISTER_COMPONENT(CapsuleCollider)

engine::CapsuleCollider::CapsuleCollider(const SharedPtr<GameObject>& owner, const _string& name)
	: Collider(owner, name), m_Capsule(), m_Radius(0.5f), m_Height(2.f)
{

}

engine::CapsuleCollider::CapsuleCollider(const CapsuleCollider& rhs)
	: Collider(rhs), m_Radius(rhs.m_Radius), m_Height(rhs.m_Height)
{

}

void engine::CapsuleCollider::UpdateCollider()
{
	calcWorldCapsule();
	calcWorldABB();
}

void engine::CapsuleCollider::Render(ComPtr<ID3D11DeviceContext> context)
{

}

void engine::CapsuleCollider::Destroy()
{
	m_bDestroyed = true;
}

engine::Capsule engine::CapsuleCollider::calcWorldCapsule()
{
	Capsule capsule;

	auto transform = GetTransform();

	_matrix worldMat = transform->GetWorldMatrix();

	// Center 변환
	_vector rot = transform->Rotation().ToVector();
	capsule.Center = XMVector3TransformCoord(m_Center.ToVector(), worldMat);

	// Direction 변환
	_vector localDirVec = Vector3::Up().ToVector();
	capsule.Direction = DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(localDirVec, rot));

	_float scaleX = transform->Scale().Value.x;
	_float scaleY = transform->Scale().Value.y;
	_float scaleZ = transform->Scale().Value.z;

	_float scaleRadius = (std::max)(scaleX, scaleZ);
	_float scaleDirection = scaleY;

	capsule.Radius = m_Radius * scaleRadius;
	capsule.Height = m_Height * scaleDirection;

	_float halfSegment = (std::max)(0.0f, (capsule.Height - 2.f * capsule.Radius) * 0.5f);

	capsule.TopCenter = capsule.Center + capsule.Direction * halfSegment;
	capsule.BottomCenter = capsule.Center - capsule.Direction * halfSegment;

	m_Capsule = capsule;

	return m_Capsule;
}

void engine::CapsuleCollider::calcWorldABB()
{
	AABB aabb;

	_vector topCenter = m_Capsule.TopCenter.ToVector();
	_vector bottomCenter = m_Capsule.BottomCenter.ToVector();

	_vector minPoint = DirectX::XMVectorMin(topCenter, bottomCenter);
	_vector maxPoint = DirectX::XMVectorMax(topCenter, bottomCenter);

	_vector extents = DirectX::XMVectorSet(m_Capsule.Radius, m_Capsule.Radius, m_Capsule.Radius, 0.f);

	aabb.Min = Vector3::FromVector(minPoint) - Vector3::FromVector(extents);
	aabb.Max = Vector3::FromVector(maxPoint) - Vector3::FromVector(extents);

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
		{"height", m_Height}
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
}
