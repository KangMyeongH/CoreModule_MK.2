#include "Collider.h"

#include "CollisionManager.h"

engine::Collider::Collider(const SharedPtr<GameObject>& owner, const _string& name)
	: Behaviour(owner, name), m_bTrigger(false), m_bHit(false)
{
}

engine::Collider::Collider(const Collider& rhs)
	: Behaviour(rhs), m_bTrigger(rhs.m_bTrigger), m_bHit(false)
{
}

void engine::Collider::registerComponent(ApplicationMode mode)
{
	if (mode == CLIENT)
	{
		CollisionManager::GetInstance().AddCollider(std::static_pointer_cast<Collider>(shared_from_this()));
	}
}
