#include "CollisionManager.h"

#include "Collider.h"

engine::CollisionManager::CollisionManager() = default;

engine::CollisionManager::~CollisionManager() = default;

void engine::CollisionManager::AddCollider(const SharedPtr<Collider>& collider)
{
	m_RegisterQueue.push_back(collider);
}

void engine::CollisionManager::ColliderUpdate()
{
	for (const auto& col : m_DynamicColliders)
	{
		if (col->IsEnabled())
		{
			col->UpdateCollider();
		}
	}

	std::vector<std::pair<SharedPtr<Collider>, SharedPtr<Collider>>> potentialPairs;

	broadPhaseSap(potentialPairs);
	narrowPhase(potentialPairs);
}

void engine::CollisionManager::RenderCollider(const ComPtr<ID3D11DeviceContext>& context)
{
	for (auto& col : m_Colliders)
	{
		if (col->IsEnabled())
		{
			col->Render(context);
		}
	}
}

void engine::CollisionManager::RegisterCollider()
{
	for (auto it = m_RegisterQueue.begin(); it != m_RegisterQueue.end();)
	{
		SharedPtr<Collider> collider = *it;

		if (!collider->GetGameObject().lock()->IsStatic())
		{
			m_DynamicColliders.push_back(collider);
		}

		else
		{
			collider->UpdateCollider();
		}

		m_Colliders.push_back(collider);

		it = m_RegisterQueue.erase(it);
	}

	m_RegisterQueue.clear();
}

void engine::CollisionManager::FlushDestroyCollider()
{
	for (auto it = m_Colliders.begin(); it != m_Colliders.end();)
	{
		SharedPtr<Collider> collider = *it;
		if (collider->IsDestroyed())
		{
			if (const auto owner = collider->GetGameObject().lock())
			{
				if (!owner->IsStatic())
				{
					m_DynamicColliders.erase(std::remove(m_DynamicColliders.begin(), m_DynamicColliders.end(), collider), m_DynamicColliders.end());
				}

				owner->RemoveComponent(collider);
			}

			it = m_Colliders.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::CollisionManager::Release()
{
	m_Colliders.clear();
	m_DynamicColliders.clear();
	m_RegisterQueue.clear();
}

void engine::CollisionManager::broadPhaseSap(std::vector<std::pair<SharedPtr<Collider>, SharedPtr<Collider>>>& outPotentialPairs)
{
	struct Edge
	{
		_float		Value;
		SharedPtr<Collider> Collider;
		_bool		IsMin;
	};

	std::vector<Edge> edges;
	edges.reserve(m_Colliders.size() * 2);

	for (auto& col : m_Colliders)
	{
		if (!col->IsEnabled())
		{
			continue;
		}

		Edge eMin;
		eMin.Value = col->GetWorldAABB().Min.Value.x;
		eMin.Collider = col;
		eMin.IsMin = true;
		edges.push_back(eMin);

		Edge eMax;
		eMax.Value = col->GetWorldAABB().Max.Value.x;
		eMax.Collider = col;
		eMax.IsMin = false;
		edges.push_back(eMax);
	}

	std::sort(edges.begin(), edges.end(),
		[](const Edge& a, const Edge& b) { return a.Value < b.Value; });

	std::vector<SharedPtr<Collider>> activeList;
	activeList.reserve(m_Colliders.size());

	for (auto& edge : edges)
	{
		if (edge.IsMin)
		{
			for (auto& active : activeList)
			{
				SharedPtr<Collider> c1 = active.get() < edge.Collider.get() ? active : edge.Collider;
				SharedPtr<Collider> c2 = active.get() < edge.Collider.get() ? edge.Collider : active;


			}
		}
	}
}

void engine::CollisionManager::narrowPhase(
	const std::vector<std::pair<SharedPtr<Collider>, SharedPtr<Collider>>>& potentialPairs)
{
}

IMPLEMENT_SINGLETON(engine::CollisionManager)

