#include "Collider.h"

engine::Collider::Collider(const SharedPtr<GameObject>& owner, const _string& name)
	: Behaviour(owner, name), m_bTrigger(false)
{

}

engine::Collider::Collider(const Collider& rhs)
	: Behaviour(rhs), m_bTrigger(rhs.m_bTrigger)
{

}

void engine::Collider::registerComponent(ApplicationMode mode)
{
	// Collision Manager에 던져주기
}
