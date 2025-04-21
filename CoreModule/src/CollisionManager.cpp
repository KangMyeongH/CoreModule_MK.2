#include "CollisionManager.h"

#include "BoxCollider.h"
#include "Collider.h"
#include "D3D11Manager.h"
#include "DebugRenderManager.h"
#include "Material.h"
#include "Rigidbody.h"
#include "SphereCollider.h"

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

void engine::CollisionManager::RenderCollider(const ComPtr<ID3D11DeviceContext>& context, const _float4X4& viewMat, const _float4X4& projMat)
{
	DebugRenderManager::GetInstance().RenderCollider(m_Colliders, context, viewMat, projMat);
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
	std::vector<Collider*> toRemoveDyn;
	toRemoveDyn.reserve(m_DynamicColliders.size());

	for (auto it = m_Colliders.begin(); it != m_Colliders.end();)
	{
		Collider* c = it->get();
		if (c->IsDestroyed())
		{
			if (auto owner = c->GetGameObject().lock())
			{
				if (!owner->IsStatic())
				{
					toRemoveDyn.push_back(c);
				}

				owner->RemoveComponent(*it);
			}

			it = m_Colliders.erase(it);
		}

		else
		{
			++it;
		}
	}

	if (!toRemoveDyn.empty())
	{
		m_DynamicColliders.erase(
			std::remove_if(m_DynamicColliders.begin(), m_DynamicColliders.end(),
				[&](const SharedPtr<Collider>& sp)
				{
					return std::find(toRemoveDyn.begin(), toRemoveDyn.end(), sp.get()) != toRemoveDyn.end();
				}),
			m_DynamicColliders.end());
	}

	for (auto it = m_CollisionMap.begin(); it != m_CollisionMap.end();)
	{
		if (it->first.first->IsDestroyed() || it->first.second->IsDestroyed())
		{
			it = m_CollisionMap.erase(it);
		}

		else
		{
			++it;
		}
	}


	//for (auto it = m_Colliders.begin(); it != m_Colliders.end();)
	//{
	//	SharedPtr<Collider> collider = *it;
	//	if (collider->IsDestroyed())
	//	{
	//		if (const auto owner = collider->GetGameObject().lock())
	//		{
	//			if (!owner->IsStatic())
	//			{
	//				m_DynamicColliders.erase(std::remove(m_DynamicColliders.begin(), m_DynamicColliders.end(), collider), m_DynamicColliders.end());
	//			}

	//			owner->RemoveComponent(collider);
	//		}

	//		it = m_Colliders.erase(it);
	//	}

	//	else
	//	{
	//		++it;
	//	}
	//}

	//for (auto pairIt = m_CollisionMap.begin(); pairIt != m_CollisionMap.end();)
	//{
	//	auto& p = pairIt->first;
	//	auto a = p.first;
	//	auto b = p.second;

	//	if (a->IsDestroyed() || b->IsDestroyed())
	//	{
	//		pairIt = m_CollisionMap.erase(pairIt);
	//	}

	//	else
	//	{
	//		++pairIt;
	//	}
	//}
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
		SharedPtr<Collider> 	Collider;
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

		const auto& aabb = col->GetWorldAABB();
		edges.push_back({ aabb.Min.Value.x, col, true });
		edges.push_back({ aabb.Max.Value.x, col, false });
	}

	std::sort(edges.begin(), edges.end(),
		[](const Edge& a, const Edge& b)
		{
			return (a.Value == b.Value) ? (a.IsMin && !b.IsMin)
				: (a.Value < b.Value);
		});

	std::unordered_set<SharedPtr<Collider>> active;
	active.reserve(m_Colliders.size());

	for (auto& edge : edges)
	{
		if (edge.IsMin)
		{
			for (auto other : active)
			{
				SharedPtr<Collider> c1 = other.get() < edge.Collider.get() ? other : edge.Collider;
				SharedPtr<Collider> c2 = other.get() < edge.Collider.get() ? edge.Collider : other;

				outPotentialPairs.emplace_back(c1, c2);
			}

			active.insert(edge.Collider);
		}

		else
		{
			active.erase(edge.Collider);
		}
	}

	std::sort(outPotentialPairs.begin(), outPotentialPairs.end(),
		[](const std::pair<SharedPtr<Collider>, SharedPtr<Collider>>& lhs,
			const std::pair<SharedPtr<Collider>, SharedPtr<Collider>>& rhs)
		{
			if (lhs.first.get() < rhs.first.get()) return true;
			if (lhs.first.get() > rhs.first.get()) return false;
			return (lhs.second.get() < rhs.second.get());
		});

	auto last = std::unique(outPotentialPairs.begin(), outPotentialPairs.end());
	outPotentialPairs.erase(last, outPotentialPairs.end());
}

void engine::CollisionManager::narrowPhase(
	const std::vector<std::pair<SharedPtr<Collider>, SharedPtr<Collider>>>& potentialPairs)
{
	std::unordered_map<std::pair<SharedPtr<Collider>, SharedPtr<Collider>>, CollisionData, ColliderPairHash, ColliderPairEq> newCollisionMap;
	newCollisionMap.reserve(potentialPairs.size());

	for (auto& pair : potentialPairs)
	{
		SharedPtr<Collider> a = pair.first;
		SharedPtr<Collider> b = pair.second;

		Vector3 aMin = a->GetWorldAABB().Min;
		Vector3 aMax = a->GetWorldAABB().Max;
		Vector3 bMin = b->GetWorldAABB().Min;
		Vector3 bMax = b->GetWorldAABB().Max;

		_bool overlapX = aMin.Value.x <= bMax.Value.x && aMax.Value.x >= bMin.Value.x;
		_bool overlapY = (aMin.Value.y <= bMax.Value.y) && (aMax.Value.y >= bMin.Value.y);
		_bool overlapZ = (aMin.Value.z <= bMax.Value.z) && (aMax.Value.z >= bMin.Value.z);

		if (!(overlapX && overlapY && overlapZ))
		{
			continue;
		}

		Vector3 normal;
		_float penetration;

		// 충돌 검사 일단은 BoxCollider끼리만
		if (checkCollider(a, b, normal, penetration))
		{
			std::pair<SharedPtr<Collider>, SharedPtr<Collider>> key(a, b);
			const CollisionData data(a, b, normal, penetration);

			newCollisionMap[key] = data;
		}
	}

	processCollisionResults(newCollisionMap);

	m_CollisionMap = std::move(newCollisionMap);
}

void engine::CollisionManager::processCollisionResults(
	const std::unordered_map<std::pair<SharedPtr<Collider>, SharedPtr<Collider>>, CollisionData, ColliderPairHash,
	ColliderPairEq>& newCollisionMap)
{
	for (auto& c : newCollisionMap)
	{
		SharedPtr<GameObject> objA = c.first.first->GetGameObject().lock();
		SharedPtr<GameObject> objB = c.first.second->GetGameObject().lock();
		if (!objA || !objB)
		{
			continue;
		}

		SharedPtr<Rigidbody> rigidA = objA->GetComponent<Rigidbody>();
		SharedPtr<Rigidbody> rigidB = objB->GetComponent<Rigidbody>();

		if (rigidA && rigidB)
		{
			if (rigidA->IsKinematic() != rigidB->IsKinematic())
			{
				const Vector3 mtv = c.second.Normal * c.second.Penetration;
				if (rigidA->IsKinematic())
				{
					objB->GetTransform()->Translate(mtv);
					//objB->GetTransform()->SetPosition(objB->GetTransform()->Position() - (-c.second.Normal * c.second.Penetration));
				}

				else if (rigidB->IsKinematic())
				{
					objA->GetTransform()->Translate(-mtv);
					//objA->GetTransform()->SetPosition(objA->GetTransform()->Position() - (c.second.Normal * c.second.Penetration));
				}
			}
		}

		if (m_CollisionMap.find(c.first) == m_CollisionMap.end())
		{
			// enter
			invokeCollisionEnter(c.first.first, c.first.second, c.second.Normal, c.second.Penetration);
		}

		else
		{
			// stay
			invokeCollisionStay(c.first.first, c.first.second, c.second.Normal, c.second.Penetration);
		}
	}

	for (auto& c : m_CollisionMap)
	{
		if (newCollisionMap.find(c.first) == newCollisionMap.end())
		{
			// exit
			invokeCollisionExit(c.first.first, c.first.second, c.second.Normal, c.second.Penetration);
		}
	}
}

void engine::CollisionManager::invokeCollisionEnter(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	const Vector3& normal, const _float& penetration)
{
	auto objA = a->GetGameObject().lock();
	auto objB = b->GetGameObject().lock();

	objA->onCollisionEnter(Collision(objB, b));
	objB->onCollisionEnter(Collision(objA, a));
}

void engine::CollisionManager::invokeCollisionStay(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	const Vector3& normal, const _float& penetration)
{
	auto objA = a->GetGameObject().lock();
	auto objB = b->GetGameObject().lock();

	objA->onCollisionStay(Collision(objB, b));
	objB->onCollisionStay(Collision(objA, a));
}

void engine::CollisionManager::invokeCollisionExit(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	const Vector3& normal, const _float& penetration)
{
	auto objA = a->GetGameObject().lock();
	auto objB = b->GetGameObject().lock();

	objA->onCollisionExit(Collision(objB, b));
	objB->onCollisionExit(Collision(objA, a));
}

engine::_bool engine::CollisionManager::checkCollider(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Vector3& outNormal, _float& outPenetration)
{
	auto typeA = a->GetColliderType();
	auto typeB = b->GetColliderType();

	switch (typeA)
	{
	case ColliderType_Box:
	{
		switch (typeB)
		{
		case ColliderType_Box:
			return checkBoxBox(a, b, outNormal, outPenetration);
		case ColliderType_Capsule:
			return checkBoxCapsule(a, b, outNormal, outPenetration);
		case ColliderType_Mesh:
			return checkBoxMesh(a, b, outNormal, outPenetration);
		case ColliderType_Sphere:
			return checkBoxSphere(a, b, outNormal, outPenetration);
		}
	}
	break;
	case ColliderType_Capsule:
	{
		switch (typeB)
		{
		case ColliderType_Box:
			return checkBoxCapsule(b, a, outNormal, outPenetration);
		case ColliderType_Capsule:
			return checkCapsuleCapsule(a, b, outNormal, outPenetration);
		case ColliderType_Mesh:
			return checkCapsuleMesh(a, b, outNormal, outPenetration);
		case ColliderType_Sphere:
			return checkCapsuleSphere(a, b, outNormal, outPenetration);
		}
	}
	break;
	case ColliderType_Mesh:
	{
		switch (typeB)
		{
		case ColliderType_Box:
			return checkBoxMesh(b, a, outNormal, outPenetration);
		case ColliderType_Capsule:
			return checkCapsuleMesh(b, a, outNormal, outPenetration);
		case ColliderType_Mesh:
			return checkMeshMesh(a, b, outNormal, outPenetration);
		case ColliderType_Sphere:
			return checkMeshSphere(a, b, outNormal, outPenetration);
		}

	}
	break;
	case ColliderType_Sphere:
	{
		switch (typeB) {
		case ColliderType_Box:
			return checkBoxSphere(b, a, outNormal, outPenetration);
		case ColliderType_Capsule:
			return checkCapsuleSphere(b, a, outNormal, outPenetration);
		case ColliderType_Mesh:
			return checkMeshSphere(b, a, outNormal, outPenetration);
		case ColliderType_Sphere:
			return checkSphereSphere(a, b, outNormal, outPenetration);
		}
	}
	break;
	}

	return false;
}

engine::_bool engine::CollisionManager::checkBoxBox(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Vector3& outNormal, _float& outPenetration)
{
	const OBB& obbA = std::static_pointer_cast<BoxCollider>(a)->GetOBB();
	const OBB& obbB = std::static_pointer_cast<BoxCollider>(b)->GetOBB();

	_float R[3][3], AbsR[3][3];
	for (_int i = 0; i < 3; ++i)
	{
		Vector3 Ai = (i == 0 ? obbA.AxisX : (i == 1 ? obbA.AxisY : obbA.AxisZ));
		for (int j = 0; j < 3; ++j)
		{
			Vector3 Bj = (j == 0 ? obbB.AxisX : (j == 1 ? obbB.AxisY : obbB.AxisZ));
			R[i][j] = DirectX::XMVectorGetX(DirectX::XMVector3Dot(Ai.ToVector(), Bj.ToVector()));
			AbsR[i][j] = fabs(R[i][j]) + 1e-6f;
		}
	}

	Vector3 tV = obbB.Center - obbA.Center;
	float t[3] = {
		DirectX::XMVectorGetX(DirectX::XMVector3Dot(tV.ToVector(), obbA.AxisX.ToVector())),
		DirectX::XMVectorGetX(DirectX::XMVector3Dot(tV.ToVector(), obbA.AxisY.ToVector())),
		DirectX::XMVectorGetX(DirectX::XMVector3Dot(tV.ToVector(), obbA.AxisZ.ToVector()))
	};

	_float minPen = FLT_MAX;
	Vector3 bestAxis = Vector3::Zero();

	auto TestAxis = [&](const _float dist, const _float ra, const _float rb, const Vector3 axis)
		{
			const _float overlap = ra + rb - fabsf(dist);

			if (overlap < 0.f)
			{
				return false;
			}

			if (overlap < minPen)
			{
				minPen = overlap;
				bestAxis = axis;
				if (dist < 0.f)
				{
					bestAxis = -bestAxis;
				}
			}

			return true;
		};

	Vector3 aExt = obbA.Extents;
	Vector3 bExt = obbB.Extents;

	for (_int i = 0; i < 3; ++i)
	{
		_float ra = (&aExt.Value.x)[i];
		_float rb = bExt.Value.x * AbsR[i][0] + bExt.Value.y * AbsR[i][1] + bExt.Value.z * AbsR[i][2];

		if (!TestAxis(t[i], ra, rb, (i == 0 ? obbA.AxisX : (i == 1 ? obbA.AxisY : obbA.AxisZ))))
		{
			return false;
		}
	}

	for (int j = 0; j < 3; ++j)
	{
		_float ra = aExt.Value.x * AbsR[0][j] + aExt.Value.y * AbsR[1][j] + aExt.Value.z * AbsR[2][j];
		_float rb = (&bExt.Value.x)[j];
		_float dist = t[0] * R[0][j] + t[1] * R[1][j] + t[2] * R[2][j];

		if (!TestAxis(dist, ra, rb, (j == 0 ? obbB.AxisX : (j == 1 ? obbB.AxisY : obbB.AxisZ))))
		{
			return false;
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		Vector3 Ai = (i == 0 ? obbA.AxisX : (i == 1 ? obbA.AxisY : obbA.AxisZ));
		for (int j = 0; j < 3; ++j)
		{
			Vector3 Bj = (j == 0 ? obbB.AxisX : (j == 1 ? obbB.AxisY : obbB.AxisZ));
			_vector axis = DirectX::XMVector3Cross(Ai.ToVector(), Bj.ToVector());

			if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(axis)) < 1e-6f)
			{
				continue;
			}

			_float ra = aExt.Value.y * AbsR[(i + 1) % 3][j] + aExt.Value.z * AbsR[(i + 2) % 3][j];
			_float rb = bExt.Value.y * AbsR[i][(j + 2) % 3] + bExt.Value.z * AbsR[i][(j + 1) % 3];
			_float dist = fabsf( t[(i + 2) % 3] * R[(i + 1) % 3][j] - t[(i + 1) % 3] * R[(i + 2) % 3][j]);
			if (!TestAxis(dist, ra, rb, Vector3::FromVector(DirectX::XMVector3Normalize(axis))))
			{
				return false;
			}
		}
	}

	outPenetration = minPen;
	outNormal = bestAxis;

	return true;
}

engine::_bool engine::CollisionManager::checkBoxCapsule(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Vector3& outNormal, _float& outPenetration)
{
	return true;
}

engine::_bool engine::CollisionManager::checkBoxMesh(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Vector3& outNormal, _float& outPenetration)
{
	return true;
}

engine::_bool engine::CollisionManager::checkBoxSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Vector3& outNormal, _float& outPenetration)
{
	return true;
}

engine::_bool engine::CollisionManager::checkCapsuleCapsule(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Vector3& outNormal, _float& outPenetration)
{
	return true;
}

engine::_bool engine::CollisionManager::checkCapsuleMesh(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Vector3& outNormal, _float& outPenetration)
{
	return true;
}

engine::_bool engine::CollisionManager::checkCapsuleSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Vector3& outNormal, _float& outPenetration)
{
	return true;
}

engine::_bool engine::CollisionManager::checkMeshMesh(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Vector3& outNormal, _float& outPenetration)
{
	return true;
}

engine::_bool engine::CollisionManager::checkMeshSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Vector3& outNormal, _float& outPenetration)
{
	return true;
}

engine::_bool engine::CollisionManager::checkSphereSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Vector3& outNormal, _float& outPenetration)
{
	return true;
}

IMPLEMENT_SINGLETON(engine::CollisionManager)

