#include "Rigidbody.h"

#include "GameObject.h"
#include "PhysicsManager.h"
#include "RenderManager.h"

DEFINE_REGISTER_COMPONENT(Rigidbody)

engine::Rigidbody::Rigidbody(const SharedPtr<GameObject>& owner, const _string& name) : Component(owner, name),
	m_Mass(1.f),
	m_Drag(0.f),
	m_UseGravity(false),
	m_IsKinematic(false)
{
}

void engine::Rigidbody::AddForce(const Vector3& force)
{
	if (m_Mass != 0.f)
	{
		m_Velocity += force / m_Mass;
	}

	else
	{
		m_Velocity += force;
	}
}

void engine::Rigidbody::Destroy()
{
	if (!m_bDestroyed)
	{
		m_bDestroyed = true;
	}
}

void engine::Rigidbody::to_json(nlohmann::ordered_json& j)
{
	_string type = "Rigidbody";

	j = nlohmann::ordered_json{
		{"type", type},
		{"mass", m_Mass},
		{"drag", m_Drag},
		{"useGravity", m_UseGravity},
		{"isKinematic", m_IsKinematic}
	};
}

void engine::Rigidbody::from_json(const nlohmann::ordered_json& j)
{
	if (j.contains("mass"))
		SetMass(j.at("mass").get<float>());
	if (j.contains("drag"))
		SetDrag(j.at("drag").get<float>());
	if (j.contains("useGravity"))
		SetUseGravity(j.at("useGravity").get<bool>());
	if (j.contains("isKinematic"))
		SetIsKinematic(j.at("isKinematic").get<bool>());
}

void engine::Rigidbody::registerComponent(ApplicationMode mode)
{
	if (mode == CLIENT)
	{
		PhysicsManager::GetInstance().AddRigidbody(std::static_pointer_cast<Rigidbody>(shared_from_this()));
	}
}

void engine::Rigidbody::rigidbodyUpdate(const float deltaTime)
{
	if (m_UseGravity)
	{
		m_Velocity.Value.y -= 9.81f * deltaTime * 2.5f;
	}

	m_Velocity.Value.x *= 1.0f / (1.0f + m_Drag * deltaTime);
	m_Velocity.Value.y *= 1.0f / (1.0f + m_Drag * deltaTime);
	m_Velocity.Value.z *= 1.0f / (1.0f + m_Drag * deltaTime);

	auto transform = m_Owner.lock()->GetTransform();

	if (transform)
	{
		transform->Translate(m_Velocity * deltaTime);
	}
}
