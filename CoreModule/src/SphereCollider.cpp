#include "SphereCollider.h"

DEFINE_REGISTER_COMPONENT(SphereCollider)

engine::SphereCollider::SphereCollider(const SharedPtr<GameObject>& owner, const _string& name)
	: Collider(owner, name), m_Radius(0.5f)
{

}

engine::SphereCollider::SphereCollider(const SphereCollider& rhs)
	: Collider(rhs), m_Center(rhs.m_Center), m_Radius(rhs.m_Radius)
{

}

void engine::SphereCollider::UpdateCollider()
{
	calcWorldSphere();
	calcWorldABB();
}

void engine::SphereCollider::Render(ComPtr<ID3D11DeviceContext> context)
{

}

void engine::SphereCollider::Destroy()
{
	m_bDestroyed = true;
}

engine::Sphere engine::SphereCollider::calcWorldSphere()
{
	Sphere sphere;

	_vector localCenter = m_Center.ToVector();
	_float localRadius = m_Radius;

	_matrix worldMat = GetTransform()->GetWorldMatrix();
	_float3 worldScale = GetTransform()->Scale().Value;
	_float 	uniformScale = (std::max)({ std::abs(worldScale.x), std::abs(worldScale.y), std::abs(worldScale.z) });
	_vector worldCenter = XMVector3TransformCoord(localCenter, worldMat);

	sphere.Center = worldCenter;
	sphere.Radius = localRadius * uniformScale;

	m_Sphere = sphere;

	return m_Sphere;
}

void engine::SphereCollider::calcWorldABB()
{
	AABB aabb;

	const _float3 worldCenter = m_Sphere.Center.Value;
	const _float worldRadius = m_Sphere.Radius;

	aabb.Min = Vector3{ worldCenter.x - worldRadius, worldCenter.y - worldRadius, worldCenter.z - worldRadius };
	aabb.Max = Vector3{ worldCenter.x + worldRadius, worldCenter.y + worldRadius, worldCenter.z + worldRadius };

	m_AABB = aabb;
}

void engine::SphereCollider::to_json(nlohmann::ordered_json& j)
{
	j = nlohmann::ordered_json{
		{"type", GetName()},
		{"enable", m_bEnabled},
		{"isTrigger", m_bTrigger},
		{"center", m_Center},
		{"radius", m_Radius}
	};
}

void engine::SphereCollider::from_json(const nlohmann::ordered_json& j)
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
}
