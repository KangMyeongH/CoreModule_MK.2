#include "PhysicsManager.h"

#include "GameObject.h"
#include "Rigidbody.h"

engine::PhysicsManager::~PhysicsManager()
{

}

void engine::PhysicsManager::PhysicsUpdate(const float deltaTime) const
{
	for (auto& rigidbody : m_Rigidbodies)
	{
		if (auto owner = rigidbody->GetGameObject().lock())
		{
			if (owner->IsActive())
			{
				rigidbody->rigidbodyUpdate(deltaTime);
			}
		}
	}
}

void engine::PhysicsManager::AddRigidbody(const SharedPtr<Rigidbody>& rigidbody)
{
	m_RegisterQueue.push_back(rigidbody);
}

void engine::PhysicsManager::RegisterRigidbody()
{
	for (auto it = m_RegisterQueue.begin(); it != m_RegisterQueue.end();)
	{
		m_Rigidbodies.push_back(*it);
		it = m_RegisterQueue.erase(it);
	}

	m_RegisterQueue.clear();
}

void engine::PhysicsManager::FlushDestroyRigidbody()
{
	for (auto it = m_Rigidbodies.begin(); it != m_Rigidbodies.end();)
	{
		const auto rigidbody = *it;

		if (rigidbody->IsDestroyed())
		{
			if (const auto owner = (*it)->GetGameObject().lock())
			{
				owner->RemoveComponent(*it);
			}

			it = m_Rigidbodies.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::PhysicsManager::Release()
{
	m_Rigidbodies.clear();
	m_RegisterQueue.clear();
}

IMPLEMENT_SINGLETON(engine::PhysicsManager)

