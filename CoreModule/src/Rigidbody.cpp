#include "Rigidbody.h"

#include "PhysicsManager.h"
#include "RenderManager.h"

DEFINE_REGISTER_COMPONENT(Rigidbody)

engine::Rigidbody::Rigidbody(const SharedPtr<GameObject>& owner, const _string& name) : Component(owner, name),
	m_Mass(0),
	m_Drag(0),
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
}

void engine::Rigidbody::from_json(const nlohmann::ordered_json& j)
{
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
	//if (m_UseGravity)
	//{
	//	m_Velocity.y -= 9.81f * _deltaTime;
	//}

	//m_Velocity.x *= 1.0f / (1.0f + m_Drag * _deltaTime);
	//m_Velocity.y *= 1.0f / (1.0f + m_Drag * _deltaTime);
	//m_Velocity.z *= 1.0f / (1.0f + m_Drag * _deltaTime);

	//Transform* transform = &m_Owner->Get_Transform();

	//if (transform)
	//{
	//	transform->Translate(m_Velocity * _deltaTime);
	//}
}
