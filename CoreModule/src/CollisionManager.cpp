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
	m_CollisionMap.clear();
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

		col->SetHit(false);
		col->SetBoardHit(false);
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

			edge.Collider->SetBoardHit(true);
			active.insert(edge.Collider);
		}

		else
		{
			edge.Collider->SetBoardHit(false);
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

		Contact contact;

		//Vector3 normal;
		//_float penetration;

		// 충돌 검사 일단은 BoxCollider끼리만
		if (checkCollider(a, b, contact))
		{
			std::pair<SharedPtr<Collider>, SharedPtr<Collider>> key(a, b);
			const CollisionData data(a, b, contact);

			SharedPtr<Rigidbody> rigidA = a->GetGameObject().lock()->GetComponent<Rigidbody>();
			SharedPtr<Rigidbody> rigidB = b->GetGameObject().lock()->GetComponent<Rigidbody>();

			if (rigidA && rigidB)
			{
				resolvePenetration(rigidA, rigidB, contact);
				applyImpulse(rigidA, rigidB, contact);
			}

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
				Contact contact = c.second.Contact;

				//resolvePenetration(rigidA, rigidB, contact);
				//applyImpulse(rigidA, rigidB, contact);
				if (rigidA->IsKinematic())
				{
					

					//objB->GetTransform()->Translate(mtv);
					//objB->GetTransform()->SetPosition(objB->GetTransform()->Position() - (-c.second.Normal * c.second.Penetration));
				}

				else if (rigidB->IsKinematic())
				{
					//objA->GetTransform()->Translate(-mtv);
					//objA->GetTransform()->SetPosition(objA->GetTransform()->Position() - (c.second.Normal * c.second.Penetration));
				}
			}
		}

		if (m_CollisionMap.find(c.first) == m_CollisionMap.end())
		{
			// enter
			invokeCollisionEnter(c.first.first, c.first.second, c.second.Contact);
		}

		else
		{
			// stay
			invokeCollisionStay(c.first.first, c.first.second, c.second.Contact);
		}
	}

	for (auto& c : m_CollisionMap)
	{
		if (newCollisionMap.find(c.first) == newCollisionMap.end())
		{
			// exit
			invokeCollisionExit(c.first.first, c.first.second, c.second.Contact);
		}
	}
}

void engine::CollisionManager::invokeCollisionEnter(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, const Contact& contact)
{
	auto objA = a->GetGameObject().lock();
	auto objB = b->GetGameObject().lock();

	a->SetHit(true);
	b->SetHit(true);

	objA->onCollisionEnter(Collision(objB, b));
	objB->onCollisionEnter(Collision(objA, a));
}

void engine::CollisionManager::invokeCollisionStay(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, const Contact& contact)
{
	auto objA = a->GetGameObject().lock();
	auto objB = b->GetGameObject().lock();

	a->SetHit(true);
	b->SetHit(true);

	objA->onCollisionStay(Collision(objB, b));
	objB->onCollisionStay(Collision(objA, a));
}

void engine::CollisionManager::invokeCollisionExit(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, const Contact& contact)
{
	auto objA = a->GetGameObject().lock();
	auto objB = b->GetGameObject().lock();

	objA->onCollisionExit(Collision(objB, b));
	objB->onCollisionExit(Collision(objA, a));
}

engine::_bool engine::CollisionManager::checkCollider(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Contact& out)
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
			return checkBoxBox(a, b, out);
		case ColliderType_Capsule:
			return checkBoxCapsule(a, b, out);
		case ColliderType_Mesh:
			return checkBoxMesh(a, b, out);
		case ColliderType_Sphere:
			return checkBoxSphere(a, b, out);
		}
	}
	break;
	case ColliderType_Capsule:
	{
		switch (typeB)
		{
		case ColliderType_Box:
			return checkBoxCapsule(b, a, out);
		case ColliderType_Capsule:
			return checkCapsuleCapsule(a, b, out);
		case ColliderType_Mesh:
			return checkCapsuleMesh(a, b, out);
		case ColliderType_Sphere:
			return checkCapsuleSphere(a, b, out);
		}
	}
	break;
	case ColliderType_Mesh:
	{
		switch (typeB)
		{
		case ColliderType_Box:
			return checkBoxMesh(b, a, out);
		case ColliderType_Capsule:
			return checkCapsuleMesh(b, a, out);
		case ColliderType_Mesh:
			return checkMeshMesh(a, b, out);
		case ColliderType_Sphere:
			return checkMeshSphere(a, b, out);
		}

	}
	break;
	case ColliderType_Sphere:
	{
		switch (typeB) {
		case ColliderType_Box:
			return checkBoxSphere(b, a, out);
		case ColliderType_Capsule:
			return checkCapsuleSphere(b, a, out);
		case ColliderType_Mesh:
			return checkMeshSphere(b, a, out);
		case ColliderType_Sphere:
			return checkSphereSphere(a, b, out);
		}
	}
	break;
	}

	return false;
}

engine::_bool engine::CollisionManager::checkBoxBox(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Contact& out)
{
	using namespace DirectX;

	const _float epsilon = 1e-5f;

	out.IsHit = false;

	const OBB& obbA = std::static_pointer_cast<BoxCollider>(a)->GetOBB();
	const OBB& obbB = std::static_pointer_cast<BoxCollider>(b)->GetOBB();

	_vector cA = obbA.Center.ToVector();
	_vector cB = obbB.Center.ToVector();

	_vector uA[3] = {
		obbA.AxisX.ToVector(),
		obbA.AxisY.ToVector(),
		obbA.AxisZ.ToVector()
	};

	_vector uB[3] = {
		obbB.AxisX.ToVector(),
		obbB.AxisY.ToVector(),
		obbB.AxisZ.ToVector()
	};

	const float hA[3] = {
		obbA.Extents.Value.x,
		obbA.Extents.Value.y,
		obbA.Extents.Value.z
	};

	const float hB[3] = {
		obbB.Extents.Value.x,
		obbB.Extents.Value.y,
		obbB.Extents.Value.z
	};

	_float R[3][3];
	_float absR[3][3];

	for (_int i = 0; i < 3; ++i)
	{
		for (_int j = 0; j < 3; ++j)
		{
			R[i][j] = XMVectorGetX(XMVector3Dot(uA[i], uB[j]));
			absR[i][j] = std::fabs(R[i][j]) + epsilon;
		}
	}

	_vector tV = cB - cA;
	_float t[3] = {
		XMVectorGetX(XMVector3Dot(tV, uA[0])),
		XMVectorGetX(XMVector3Dot(tV, uA[1])),
		XMVectorGetX(XMVector3Dot(tV, uA[2])),
	};

	_float minOverlap = FLT_MAX;
	_int minType = -1;		// 0~2 : A축, 3~5 : B축, 6~14 : AXB 축
	_int minIdxI = 0;	// 교차축용
	_int minIdxJ = 0;	// 교차축용
	_int sign = 1;		// 노멀 방향

	auto TestAxis = [&](_float overlap, _int type, _int i = 0, _int j = 0, _float s = 1.f)
		{
			if (overlap < minOverlap)
			{
				minOverlap = overlap;
				minType = type;
				minIdxI = i;
				minIdxJ = j;
				sign = (s >= 0.f) ? 1 : -1;
			}
		};

	// 15개의 분리축 테스트
	_float ra, rb, proj, overlap;

	// A 축 3개
	for (_int i = 0; i < 3; ++i)
	{
		ra = hA[i];
		rb = hB[0] * absR[i][0] + hB[1] * absR[i][1] + hB[2] * absR[i][2];
		proj = std::fabs(t[i]);
		overlap = ra + rb - proj;
		if (overlap < 0)
		{
			return false;
		}
		TestAxis(overlap, i, i, 0, (t[i] < 0.f) ? -1.f : 1.f);
	}

	// B 축 3개
	for (int j = 0; j < 3; ++j)
	{
		ra = hA[0] * absR[0][j] + hA[1] * absR[1][j] + hA[2] * absR[2][j];
		rb = hB[j];
		proj = std::fabs(t[0] * R[0][j] + t[1] * R[1][j] + t[2] * R[2][j]);
		overlap = ra + rb - proj;
		if (overlap < 0)
		{
			return false;
		}
		TestAxis(overlap, 3 + j, 0, j, ((t[0] * R[0][j] + t[1] * R[1][j] + t[2] * R[2][j]) < 0.f) ? -1.f : 1.f);
	}

#define EDGE_TEST(i, j, T0, T1, TaX, TaY, TbX, TbY)	 			\
	ra = hA[T0] * absR[T1][j] + hA[T1] * absR[T0][j]; 			\
	rb = hB[TbX] * absR[i][TbY] + hB[TbY] * absR[i][TbX]; 		\
	proj = std::fabs(t[T0] * R[T1][j] - t[T1] * R[T0][j]); 		\
	overlap = ra + rb - proj; 									\
	if (overlap < 0) 											\
	{															\
		return false;											\
	}															\
	TestAxis(overlap, 6 + 3 * (i) + (j), i, j, ((t[T0] * R[T1][j] - t[T1] * R[T0][j]) < 0.f ? -1.f : 1.f));

	// i=0
	EDGE_TEST(0, 0, 1, 2, 1, 2, 1, 2);
	EDGE_TEST(0, 1, 1, 2, 1, 2, 0, 2);
	EDGE_TEST(0, 2, 1, 2, 1, 2, 0, 1);
	// i=1
	EDGE_TEST(1, 0, 0, 2, 0, 2, 1, 2);
	EDGE_TEST(1, 1, 0, 2, 0, 2, 0, 2);
	EDGE_TEST(1, 2, 0, 2, 0, 2, 0, 1);
	// i=2
	EDGE_TEST(2, 0, 0, 1, 0, 1, 1, 2);
	EDGE_TEST(2, 1, 0, 1, 0, 1, 0, 2);
	EDGE_TEST(2, 2, 0, 1, 0, 1, 0, 1);

#undef EDGE_TEST

	out.IsHit = true;
	out.Penetration = minOverlap;

	_vector n;

	if (minType < 3)
	{
		n = uA[minType] * static_cast<_float>(sign);
	}

	else if (minType < 6)
	{
		n = uB[minType - 3] * static_cast<_float>(sign);
	}

	else
	{
		n = XMVector3Cross(uA[minIdxI], uB[minIdxJ]);
		n = XMVector3Normalize(n) * static_cast<_float>(sign);
	}

	out.Normal = n;

	_vector contactA = cA + XMVectorScale(n, hA[0]);
	_vector contactB = contactA - XMVectorScale(n, out.Penetration);
	out.PointA = contactA;
	out.PointB = contactB;

	//// 9개 축
	//ra = hA[1] * absR[2][0] + hA[2] * absR[1][0];
	//rb = hB[1] * absR[0][2] + hB[2] * absR[0][1];
	//if (std::fabs(t[2] * R[1][0] - t[1] * R[2][0]) > ra + rb)
	//{
	//	return false;
	//}

	//ra = hA[1] * absR[2][1] + hA[2] * absR[1][1];
	//rb = hB[0] * absR[0][2] + hB[2] * absR[0][0];
	//if (std::fabs(t[2] * R[1][1] - t[1] * R[2][1]) > ra + rb)
	//{
	//	return false;
	//}

	//ra = hA[1] * absR[2][2] + hA[2] * absR[1][2];
	//rb = hB[0] * absR[0][1] + hB[1] * absR[0][0];
	//if (std::fabs(t[2] * R[1][2] - t[1] * R[2][2]) > ra + rb)
	//{
	//	return false;
	//}

	//ra = hA[0] * absR[2][0] + hA[2] * absR[0][0];
	//rb = hB[1] * absR[1][2] + hB[2] * absR[1][1];
	//if (std::fabs(t[0] * R[2][0] - t[2] * R[0][0]) > ra + rb)
	//{
	//	return false;
	//}

	//ra = hA[0] * absR[2][1] + hA[2] * absR[0][1];
	//rb = hB[0] * absR[1][2] + hB[2] * absR[1][0];
	//if (std::fabs(t[0] * R[2][1] - t[2] * R[0][1]) > ra + rb) 
	//{
	//	return false;
	//}

	//ra = hA[0] * absR[2][2] + hA[2] * absR[0][2];
	//rb = hB[0] * absR[1][1] + hB[1] * absR[1][0];
	//if (std::fabs(t[0] * R[2][2] - t[2] * R[0][2]) > ra + rb) 
	//{
	//	return false;
	//}

	//ra = hA[0] * absR[1][0] + hA[1] * absR[0][0];
	//rb = hB[1] * absR[2][2] + hB[2] * absR[2][1];
	//if (std::fabs(t[1] * R[0][0] - t[0] * R[1][0]) > ra + rb)
	//{
	//	return false;
	//}

	//ra = hA[0] * absR[1][1] + hA[1] * absR[0][1];
	//rb = hB[0] * absR[2][2] + hB[2] * absR[2][0];
	//if (std::fabs(t[1] * R[0][1] - t[0] * R[1][1]) > ra + rb)
	//{
	//	return false;
	//}

	//ra = hA[0] * absR[1][2] + hA[1] * absR[0][2];
	//rb = hB[0] * absR[2][1] + hB[1] * absR[2][0];
	//if (std::fabs(t[1] * R[0][2] - t[0] * R[1][2]) > ra + rb)
	//{
	//	return false;
	//}

	/*_float R[3][3], AbsR[3][3];
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
	outNormal = bestAxis;*/

	return true;
}

engine::_bool engine::CollisionManager::checkBoxCapsule(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Contact& out)
{
	return true;
}

engine::_bool engine::CollisionManager::checkBoxMesh(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	return true;
}

engine::_bool engine::CollisionManager::checkBoxSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	return true;
}

engine::_bool engine::CollisionManager::checkCapsuleCapsule(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	return true;
}

engine::_bool engine::CollisionManager::checkCapsuleMesh(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	return true;
}

engine::_bool engine::CollisionManager::checkCapsuleSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	return true;
}

engine::_bool engine::CollisionManager::checkMeshMesh(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	return true;
}

engine::_bool engine::CollisionManager::checkMeshSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	return true;
}

engine::_bool engine::CollisionManager::checkSphereSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	return true;
}

void engine::CollisionManager::resolvePenetration(const SharedPtr<Rigidbody>& a, const SharedPtr<Rigidbody>& b,
	const Contact& c, _float percent, _float slop)
{
	if (!c.IsHit)
	{
		return;
	}

	_float invMassA = a->GetInvMass();
	_float invMassB = b->GetInvMass();
	_float totalInv = invMassA + invMassB;

	if (totalInv == 0.f)
	{
		return;
	}

	_float correction = std::max(c.Penetration - slop, 0.f) * percent / totalInv;

	Vector3 corr = c.Normal * correction;
	std::cerr << "penetration : " << c.Penetration << "\n";
	std::cerr << "X : " << corr.Value.x << ", Y : " << corr.Value.y << ", Z : " << corr.Value.z << "\n";

	a->GetTransform()->Translate(-(corr * invMassA));
	b->GetTransform()->Translate(corr * invMassB);
}

void engine::CollisionManager::applyImpulse(const SharedPtr<Rigidbody>& a, const SharedPtr<Rigidbody>& b,
	const Contact& c, _float restitution)
{
	if (!c.IsHit)
	{
		return;
	}

	_vector n = c.Normal.ToVector();
	Vector3 vA = a->Velocity();
	Vector3 vB = b->Velocity();
	_vector rv = (vB - vA).ToVector();

	_float velAlongN = DirectX::XMVectorGetX(DirectX::XMVector3Dot(rv, n));
	if (velAlongN > 0)
	{
		return;
	}

	_float invMassSum = a->GetInvMass() + b->GetInvMass();
	_float j = -(1.f + restitution) * velAlongN / invMassSum;

	Vector3 impulse = c.Normal * j;
	vA -= impulse * a->GetInvMass();
	vB += impulse * b->GetInvMass();

	a->Velocity() = vA;
	b->Velocity() = vB;
}

IMPLEMENT_SINGLETON(engine::CollisionManager)

