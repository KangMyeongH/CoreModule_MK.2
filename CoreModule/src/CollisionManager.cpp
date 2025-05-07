#include "CollisionManager.h"

#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "Collider.h"
#include "D3D11Manager.h"
#include "DebugRenderManager.h"
#include "Material.h"
#include "MeshCollider.h"
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

		if (checkCollider(a, b, contact))
		{
			std::pair<SharedPtr<Collider>, SharedPtr<Collider>> key(a, b);
			const CollisionData data(a, b, contact);
			if (!a->IsTrigger() && !b->IsTrigger())
			{
				SharedPtr<Rigidbody> rigidA = a->GetGameObject().lock()->GetComponent<Rigidbody>();
				SharedPtr<Rigidbody> rigidB = b->GetGameObject().lock()->GetComponent<Rigidbody>();

				if (rigidA && rigidB)
				{
					resolvePenetration(rigidA, rigidB, contact);
					applyImpulse(rigidA, rigidB, contact);
				}
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
			return checkBoxCapsule(a, b, out);
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
			return checkBoxMesh(a, b, out);
		case ColliderType_Capsule:
			return checkCapsuleMesh(a, b, out);
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
			return checkBoxSphere(a, b, out);
		case ColliderType_Capsule:
			return checkCapsuleSphere(a, b, out);
		case ColliderType_Mesh:
			return checkMeshSphere(a, b, out);
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

	const _float epsilon = 1e-6f;
	const _float EDGE_BIAS = 1.05f;
	const _float SAME_AXIS_EPS = 0.005f;
	const _vector WORLD_UP = XMVectorSet(0.f, 1.f, 0.f, 0.f);


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
	_int minI = 0;	// 교차축용
	_int minJ = 0;	// 교차축용
	_int sign = 1;		// 노멀 방향

	_float faceOverlapMin = FLT_MAX;
	_int faceAxis = -1;
	_int faceSign = 1;

	auto IsNearlyEq = [&](_float a, _float b) {return std::fabs(a - b) < SAME_AXIS_EPS; };

	auto SaveAxis = [&](float overlap, int type, int i, int j, bool isFace)
		{
			_vector axisCand = 	(type < 3) ? uA[type] :
								(type < 6) ? uB[type - 3] :
								XMVector3Cross(uA[i], uB[j]);

			_float s = XMVectorGetX(XMVector3Dot(tV, axisCand));


			bool prefer = false;

			if (IsNearlyEq(overlap, minOverlap))
			{
				// tie break: 면축 우선, 또는 WORLD UP 기준 큰 축 우선
				bool curFace = isFace;
				bool prevFace = (minType < 6 ? true : false);
				_vector axisCur = (type < 3) ? uA[type] :
					(type < 6) ? uB[type - 3] :
					XMVector3Cross(uA[i], uB[j]);

				_vector axisPrev = (minType < 0) ? axisCur :    // 첫 저장
					(minType < 3) ? uA[minType] :
					(minType < 6) ? uB[minType - 3] :
					XMVector3Cross(uA[minI], uB[minJ]);

				if (curFace && !prevFace)                     prefer = true;
				else if (curFace == prevFace)                 // 둘 다 face 또는 edge
				{
					float dotCur = std::fabs(XMVectorGetX(XMVector3Dot(axisCur, WORLD_UP)));
					float dotPrev = std::fabs(XMVectorGetX(XMVector3Dot(axisPrev, WORLD_UP)));
					if (dotCur > dotPrev + 0.2f)              prefer = true;
				}
			}
			else if (overlap < minOverlap)                    prefer = true;

			if (prefer)
			{
				minOverlap = overlap;  minType = type;
				minI = i; minJ = j;  sign = (s >= 0.f) ? 1 : -1;
			}

			if (isFace && overlap < faceOverlapMin)
			{
				faceOverlapMin = overlap;  faceAxis = type;  faceSign = (s >= 0.f) ? 1 : -1;
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
		if (overlap < 0.f)
		{
			return false;
		}
		SaveAxis(overlap, i, i, 0, true);
	}

	// B 축 3개
	for (int j = 0; j < 3; ++j)
	{
		ra = hA[0] * absR[0][j] + hA[1] * absR[1][j] + hA[2] * absR[2][j];
		rb = hB[j];
		proj = std::fabs(t[0] * R[0][j] + t[1] * R[1][j] + t[2] * R[2][j]);
		overlap = ra + rb - proj;
		if (overlap < 0.f)
		{
			return false;
		}
		SaveAxis(overlap, 3 + j, 0, j, true);
	}

	for (int i = 0; i < 3; ++i)
	{
		int ia = (i + 1) % 3, ib = (i + 2) % 3;
		for (int j = 0; j < 3; ++j)
		{
			float parallel = std::fabs(R[i][j]);      
			if (parallel > 0.95f)  continue;          

			_vector cross = XMVector3Cross(uA[i], uB[j]);
			if (XMVectorGetX(XMVector3LengthSq(cross)) < 1e-6f) continue;

			//_vector cross = XMVector3Cross(uA[i], uB[j]);
			//if (XMVectorGetX(XMVector3LengthSq(cross)) < 1e-8f) continue; // 거의 평행

			int ja = (j + 1) % 3, jb = (j + 2) % 3;

			ra = hA[ia] * absR[ib][j] + hA[ib] * absR[ia][j];
			rb = hB[ja] * absR[i][jb] + hB[jb] * absR[i][ja];
			proj = std::fabs(t[ib] * R[ia][j] - t[ia] * R[ib][j]);
			overlap = ra + rb - proj;
			if (overlap < 0.f) return false;

			float s = (t[ib] * R[ia][j] - t[ia] * R[ib][j]);
			SaveAxis(overlap, 6 + 3 * i + j, i, j, false);
		}
	}

	if (minType >= 6 && faceAxis >= 0 && faceOverlapMin < minOverlap * EDGE_BIAS)
	{
		minType = faceAxis;
		minOverlap = faceOverlapMin;
		sign = faceSign;
	}

	// Normal 계산
	_vector n;
	if (minType < 3)
	{
		n = XMVectorScale(uA[minType], static_cast<float>(sign));
	}

	else if (minType < 6)
	{
		n = XMVectorScale(uB[minType - 3], static_cast<float>(sign));
	}

	else
	{
		n = XMVector3Cross(uA[minI], uB[minJ]);
		n = XMVector3Normalize(n) * static_cast<float>(sign);
		// 평행 예외, face축으로 이미 교체됨
	}

	if (XMVectorGetX(XMVector3Dot(tV, n)) < 0.0f)
	{
		n = -n;
		sign = -sign;
	}

	auto rigidA = a->GetGameObject().lock()->GetComponent<Rigidbody>();
	auto rigidB = b->GetGameObject().lock()->GetComponent<Rigidbody>();

	if (rigidA && rigidB)
	{
		_vector relVel = (rigidB->Velocity() - rigidA->Velocity()).ToVector();
		if (XMVectorGetX(XMVector3Dot(relVel, n)) > 0.f) n = -n;
	}

	// (선택) 상대속도로 노멀 방향 확인
/* if (RigidBody 정보가 있다면
_vector relVel = XMLoadFloat3(&bBody.velocity) - XMLoadFloat3(&aBody.velocity);
if (XMVectorGetX(XMVector3Dot(relVel, n)) > 0.f) n = -n;
*/

	float depthOnA =
		std::fabs(XMVectorGetX(XMVector3Dot(n, uA[0]))) * hA[0] +
		std::fabs(XMVectorGetX(XMVector3Dot(n, uA[1]))) * hA[1] +
		std::fabs(XMVectorGetX(XMVector3Dot(n, uA[2]))) * hA[2];

	_vector contactA = cA + XMVectorScale(n, depthOnA);
	_vector contactB = contactA - XMVectorScale(n, minOverlap);

	out.IsHit = true;
	out.Penetration = minOverlap;
	out.Normal = n;
	out.PointA = contactA;
	out.PointB = contactB;

	return true;
}

engine::_bool engine::CollisionManager::checkBoxCapsule(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Contact& out)
{
	using namespace DirectX;

	const auto typeA = a->GetColliderType();
	const auto typeB = b->GetColliderType();

	if (typeA == ColliderType_Capsule && typeB == ColliderType_Box)
	{
		Capsule cap = std::static_pointer_cast<CapsuleCollider>(a)->GetCapsule();
		OBB obb = std::static_pointer_cast<BoxCollider>(b)->GetOBB();
		_bool hit = intersectCapsuleOBB(cap, obb, out);
		if (!hit)
		{
			return false;
		}
		out.Normal.Value.x *= -1.f;
		out.Normal.Value.y *= -1.f;
		out.Normal.Value.z *= -1.f;
		std::swap(out.PointA, out.PointB);

		return true;
	}

	else if (typeA == ColliderType_Box && typeB == ColliderType_Capsule)
	{
		Capsule cap = std::static_pointer_cast<CapsuleCollider>(b)->GetCapsule();
		OBB obb = std::static_pointer_cast<BoxCollider>(a)->GetOBB();
		_bool hit = intersectCapsuleOBB(cap, obb, out);
		if (!hit)
		{
			return false;
		}
		return true;
	}

	return false;
}

engine::_bool engine::CollisionManager::intersectCapsuleOBB(const Capsule& cap, const OBB& box, Contact& out)
{
	using namespace DirectX;

	out.IsHit = false;
	
	_matrix R = XMMatrixSet(
		box.AxisX.Value.x, box.AxisX.Value.y, box.AxisX.Value.z, 0,
		box.AxisY.Value.x, box.AxisY.Value.y, box.AxisY.Value.z, 0,
		box.AxisZ.Value.x, box.AxisZ.Value.y, box.AxisZ.Value.z, 0,
		0, 0, 0, 1);
	_matrix T = XMMatrixTranslation(-box.Center.Value.x, -box.Center.Value.y, -box.Center.Value.z);
	_matrix toLocal = T * XMMatrixTranspose(R);
	_matrix toWorld = XMMatrixInverse(nullptr, toLocal);

	_vector A = XMVector3Transform(cap.P0W.ToVector(), toLocal);
	_vector B = XMVector3Transform(cap.P1W.ToVector(), toLocal);
	_vector d = B - A;                                   // 방향

	const _float3 h = box.Extents.Value;                       // half extents (x,y,z)

	auto Clamp = [](_float v, _float lo, _float hi)
		{ return (v < lo) ? lo : (v > hi) ? hi : v; };

	_float t = 0.f;
	for (int axis = 0; axis < 3; ++axis)
	{
		_float a = XMVectorGetByIndex(A, axis);
		_float b = XMVectorGetByIndex(B, axis);
		_float p = b - a;                               // d[axis]
		_float lo = -(&h.x)[axis];
		_float hi = (&h.x)[axis];

		if (a < lo && p > 0)          // 밖 > 안
			t = std::max(t, (lo - a) / p);
		else if (a > hi && p < 0)     // 밖 > 안 (반대쪽)
			t = std::max(t, (hi - a) / p);
	}
	t = Clamp(t, 0.f, 1.f);

	_vector S = A + d * t;

	_float3 sL; XMStoreFloat3(&sL, S);
	_float3 qL = { Clamp(sL.x,-h.x,h.x),
				   Clamp(sL.y,-h.y,h.y),
				   Clamp(sL.z,-h.z,h.z) };
	_vector Q = XMLoadFloat3(&qL);

	_vector diff = S - Q;
	_float  dist2 = XMVectorGetX(XMVector3LengthSq(diff));

	if (dist2 > cap.Radius * cap.Radius) return false;   // 미충돌

	_float dist = std::sqrt(dist2);
	_float overlap = cap.Radius - dist;                 // > 0

	_vector normalL;
	if (dist > 1e-6f)
		normalL = diff / dist;
	else
	{
		// 세로축 침투 등으로 dist=0 > 가장 깊은 방향으로
		_float dx = std::max(0.f, std::fabs(sL.x) - h.x);
		_float dy = std::max(0.f, std::fabs(sL.y) - h.y);
		_float dz = std::max(0.f, std::fabs(sL.z) - h.z);
		if (dx >= dy && dx >= dz) normalL = XMVectorSet((sL.x > 0) ? 1 : -1, 0, 0, 0);
		else if (dy >= dz)        normalL = XMVectorSet(0, (sL.y > 0) ? 1 : -1, 0, 0);
		else                      normalL = XMVectorSet(0, 0, (sL.z > 0) ? 1 : -1, 0);
	}
	_vector normalW = XMVector3TransformNormal(normalL, R);

	_vector pointB_W = XMVector3TransformCoord(S, toWorld);      // Capsule 면
	_vector pointA_W = pointB_W - normalW * overlap;             // Box 면

	out.IsHit = true;
	out.Penetration = overlap;
	out.Normal = normalW;
	out.PointA = pointA_W;
	out.PointB = pointB_W;
	return true;
}

engine::_bool engine::CollisionManager::checkBoxMesh(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
                                                     Contact& out)
{
	using namespace DirectX;

	const auto typeA = a->GetColliderType();
	const auto typeB = b->GetColliderType();

	if (typeA == ColliderType_Box && typeB == ColliderType_Mesh)
	{
		auto obb = std::static_pointer_cast<BoxCollider>(a);
		auto mesh = std::static_pointer_cast<MeshCollider>(b);
		_bool hit = intersectOBBMesh(obb, mesh, out);

		if (!hit)
		{
			return false;
		}

		return true;
	}

	if (typeA == ColliderType_Mesh && typeB == ColliderType_Box)
	{
		auto obb = std::static_pointer_cast<BoxCollider>(b);
		auto mesh = std::static_pointer_cast<MeshCollider>(a);
		_bool hit = intersectOBBMesh(obb, mesh, out);

		if (!hit)
		{
			return false;
		}

		out.Normal.Value.x *= -1.f;
		out.Normal.Value.y *= -1.f;
		out.Normal.Value.z *= -1.f;
		std::swap(out.PointA, out.PointB);

		return true;
	}

	return false;
}

engine::_bool engine::CollisionManager::intersectOBBMesh(const SharedPtr<BoxCollider>& box,
	const SharedPtr<MeshCollider>& meshCol, Contact& out)
{
	using namespace DirectX;

	OBB worldBox = box->GetOBB();
	OBB localBox = worldOBBToLocalOBB(box->GetOBB(), meshCol->GetTransform()->GetWorldMatrix());
	_matrix meshWorld = meshCol->GetTransform()->GetWorldMatrix();
	_matrix invW = XMMatrixInverse(nullptr, meshWorld);

	const auto& mesh = meshCol->GetMesh();
	const auto& nodes = mesh.BVHNodes;
	const auto& triOrder = mesh.BVHTriangles;
	const auto& indices = mesh.Indices;
	const auto& vertices = mesh.Vertices;

	out.IsHit = false;
	out.Penetration = 0.f;
	_float bestDepth = 0.f;
	_vector bestNormal = XMVectorZero();
	_vector bestPointA = XMVectorZero();
	_vector bestPointB = XMVectorZero();
	_float invScale = XMVectorGetX(XMVector3Length(invW.r[0]));

	std::function<void(int)> traverse = [&](int nodeIdx)
		{
			const BVHNodeData& node = nodes[nodeIdx];
			
			if (!AABBvsOBB(node.Bounds, localBox))
				return;


			if (node.Left < 0)
			{
				int start = ~node.Left;
				int cnt = ~node.Right;
				for (int i = 0; i < cnt; ++i)
				{
					int ti = triOrder[start + i];

					XMVECTOR v0 = XMLoadFloat3(&vertices[indices[ti * 3 + 0]].Position);
					XMVECTOR v1 = XMLoadFloat3(&vertices[indices[ti * 3 + 1]].Position);
					XMVECTOR v2 = XMLoadFloat3(&vertices[indices[ti * 3 + 2]].Position);

					XMVECTOR axisLocal;
					float   depthLocal;
					if (!triangleOBBIntersect(localBox, v0, v1, v2, axisLocal, depthLocal))
						continue;

					XMVECTOR worldNormal = XMVector3Normalize(
						XMVector3TransformNormal(axisLocal, meshWorld)
					);

					XMVECTOR wv0 = XMVector3TransformCoord(v0, meshWorld);
					XMVECTOR wv1 = XMVector3TransformCoord(v1, meshWorld);
					XMVECTOR wv2 = XMVector3TransformCoord(v2, meshWorld);

					float d0 = XMVectorGetX(XMVector3Dot(wv0, worldNormal));
					float d1 = XMVectorGetX(XMVector3Dot(wv1, worldNormal));
					float d2 = XMVectorGetX(XMVector3Dot(wv2, worldNormal));
					XMVECTOR pB = (d1 < d0 ? wv1 : wv0);
					float    minD = std::min(d0, d1);
					if (d2 < minD) { minD = d2; pB = wv2; }

					XMVECTOR wc = worldBox.Center.ToVector();
					XMVECTOR BA[3] = {
						worldBox.AxisX.ToVector(),
						worldBox.AxisY.ToVector(),
						worldBox.AxisZ.ToVector()
					};
					float ext[3] = {
						worldBox.Extents.Value.x,
						worldBox.Extents.Value.y,
						worldBox.Extents.Value.z
					};
					float R =
						ext[0] * fabsf(XMVectorGetX(XMVector3Dot(worldNormal, BA[0]))) +
						ext[1] * fabsf(XMVectorGetX(XMVector3Dot(worldNormal, BA[1]))) +
						ext[2] * fabsf(XMVectorGetX(XMVector3Dot(worldNormal, BA[2])));
					XMVECTOR pA = wc - worldNormal * R;

					XMVECTOR localPen = axisLocal * depthLocal;
					XMVECTOR worldPen = XMVector3TransformNormal(localPen, meshWorld);
					float   depthWorld = XMVectorGetX(XMVector3Length(worldPen));

					if (!out.IsHit || depthWorld > bestDepth)
					{
						out.IsHit = true;
						bestDepth = depthWorld;
						bestNormal = worldNormal;
						bestPointA = pA;
						bestPointB = pB;
					}
				}
			}
			else
			{
				traverse(node.Left);
				traverse(node.Right);
			}
		};

	traverse(0);

	if (out.IsHit)
	{
		out.Normal = Vector3::FromVector(bestNormal);
		out.Penetration = bestDepth / invScale;
		out.PointA = Vector3::FromVector(bestPointA);
		out.PointB = Vector3::FromVector(bestPointB);
		return true;
	}

	return false;

	//std::function<_bool(_int)> Recurse = [&](_int nodeIdx)->_bool
	//	{
	//		const BVHNodeData& node = nodes[nodeIdx];

	//		if (!AABBvsOBB(node.Bounds, localBox))
	//		{
	//			return false;
	//		}

	//		if (node.Left < 0)
	//		{
	//			_int start = ~node.Left, cnt = ~node.Right;

	//			for (_int i = 0; i < cnt; ++i)
	//			{
	//				_int ti = triOrder[start + i];
	//				auto v0 = XMLoadFloat3(&vertices[indices[ti * 3 + 0]].Position);
	//				auto v1 = XMLoadFloat3(&vertices[indices[ti * 3 + 1]].Position);
	//				auto v2 = XMLoadFloat3(&vertices[indices[ti * 3 + 2]].Position);
	//				_vector axisLocal;
	//				_float depthLocal;
	//				if (!triangleOBBIntersect(localBox, v0, v1, v2, axisLocal, depthLocal))
	//				{
	//					continue;
	//				}

	//				_vector worldNormal = XMVector3Normalize(XMVector3TransformCoord(axisLocal, meshCol->GetTransform()->GetWorldMatrix()));
	//				XMVECTOR wv[3] = {
	//					XMVector3Transform(XMLoadFloat3(&vertices[indices[ti * 3 + 0]].Position),
	//				   meshCol->GetTransform()->GetWorldMatrix()),
	//					XMVector3Transform(XMLoadFloat3(&vertices[indices[ti * 3 + 1]].Position),
	//				   meshCol->GetTransform()->GetWorldMatrix()),
	//					XMVector3Transform(XMLoadFloat3(&vertices[indices[ti * 3 + 2]].Position),
	//				   meshCol->GetTransform()->GetWorldMatrix())
	//				};

	//				_float bestDot = FLT_MAX;
	//				_vector pB = XMVectorZero();
	//				for (auto& vw : wv)
	//				{
	//					_float d = XMVectorGetX(XMVector3Dot(vw, worldNormal));
	//					if (d < bestDot)
	//					{
	//						bestDot = d;
	//						pB = vw;
	//					}
	//				}

	//				_vector wc = worldBox.Center.ToVector();
	//				XMVECTOR BA[3] = {
	//					worldBox.AxisX.ToVector(),
	//					worldBox.AxisY.ToVector(),
	//					worldBox.AxisZ.ToVector()
	//				};

	//				_float R =
	//					worldBox.Extents.Value.x * fabsf(XMVectorGetX(XMVector3Dot(worldNormal, BA[0]))) +
	//					worldBox.Extents.Value.y * fabsf(XMVectorGetX(XMVector3Dot(worldNormal, BA[1]))) +
	//					worldBox.Extents.Value.z * fabsf(XMVectorGetX(XMVector3Dot(worldNormal, BA[2])));
	//				_vector pA = wc - worldNormal * R;

	//				out.IsHit = true;
	//				out.Normal = Vector3::FromVector(worldNormal);
	//				out.Penetration = depthLocal;
	//				out.PointA = Vector3::FromVector(pA);
	//				out.PointB = Vector3::FromVector(pB);

	//				std::cerr << "Normal X : " << out.Normal.Value.x << ", Y : " << out.Normal.Value.y << ", Z : " << out.Normal.Value.z << "\n";
	//				std::cerr << "Penetration : " << out.Penetration << "\n";

	//				return true;
	//			}

	//			return false;
	//		}

	//		return Recurse(node.Left) || Recurse(node.Right);
	//	};

	//_bool hit = Recurse(0);

	//if (!hit)
	//{
	//	return false;
	//}

	return true;
}

engine::_bool engine::CollisionManager::checkBoxSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
                                                       Contact& out)
{
	return false;
}

engine::_bool engine::CollisionManager::checkCapsuleCapsule(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	using namespace DirectX;

	out.IsHit = false;

	Capsule capA = std::static_pointer_cast<CapsuleCollider>(a)->GetCapsule();
	Capsule capB = std::static_pointer_cast<CapsuleCollider>(b)->GetCapsule();

	Vector3 P1 = capA.P0W;
	Vector3 Q1 = capA.P1W;
	Vector3 P2 = capB.P0W;
	Vector3 Q2 = capB.P1W;

	_vector d1 = (Q1 - P1).ToVector();
	_vector d2 = (Q2 - P2).ToVector();
	_vector r = (P1 - P2).ToVector();

	_float temp = XMVectorGetX(XMVector3Dot(d1, d1));
	_float e = XMVectorGetX(XMVector3Dot(d2, d2));
	_float f = XMVectorGetX(XMVector3Dot(d2, r));

	_float s, t;

	const _float EPS = 1e-6f;
	if (temp <= EPS && e <= EPS)
	{
		s = t = 0.0f;
	}

	else if (temp <= EPS)
	{
		s = 0.0f;
		t = Clamp(f / e, 0.0f, 1.f);
	}

	else
	{
		_float c = XMVectorGetX(XMVector3Dot(d1, r));
		if (e <= EPS)
		{
			t = 0.0f;
			s = Clamp(-c / temp, 0.0f, 1.0f);
		}

		else
		{
			_float b = XMVectorGetX(XMVector3Dot(d1, d2));
			_float denom = temp * e - b * b;

			if (denom != 0.0f)
			{
				s = Clamp<_float>((b * f - c * e) / denom, 0.0f, 1.0f);
			}

			else
			{
				s = 0.0f; // 평행
			}

			t = (b * s + f) / e;
			if (t < 0.0f)
			{
				t = 0.0f;
				s = Clamp<_float>(-c / temp, 0.0f, 1.0f);
			}

			else if (t > 1.0f)
			{
				t = 1.0f;
				s = Clamp<_float>((b - c) / temp, 0.0f, 1.0f);
			}
		}
	}

	_vector CP1 = P1.ToVector() + d1 * s;
	_vector CP2 = P2.ToVector() + d2 * t;

	_vector diff = CP2 - CP1;
	_float dist2 = XMVectorGetX(XMVector3LengthSq(diff));
	_float sumR = capA.Radius + capB.Radius;

	if (dist2 > sumR * sumR)
	{
		return false;
	}

	_float dist = std::sqrt(dist2);
	_float penetration = sumR - dist;

	_vector normal;
	if (dist > EPS)
	{
		normal = diff / dist;
	}

	else
	{
		normal = XMVectorSet(1.f, 0.f, 0.f, 0.f);
	}

	_vector pointA = CP1 + normal * capA.Radius;
	_vector pointB = CP2 - normal * capB.Radius;

	out.IsHit = true;
	out.Penetration = penetration;
	out.Normal = normal;
	out.PointA = pointA;
	out.PointB = pointB;

	return true;
}

engine::_bool engine::CollisionManager::checkCapsuleMesh(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	using namespace DirectX;

	const auto typeA = a->GetColliderType();
	const auto typeB = b->GetColliderType();

	if (typeA == ColliderType_Capsule && typeB == ColliderType_Mesh)
	{
		auto capsule = std::static_pointer_cast<CapsuleCollider>(a);
		auto mesh = std::static_pointer_cast<MeshCollider>(b);
		_bool hit = intersectCapsuleMesh(capsule, mesh, out);

		if (!hit)
		{
			return false;
		}

		out.Normal.Value.x *= -1.f;
		out.Normal.Value.y *= -1.f;
		out.Normal.Value.z *= -1.f;
		std::swap(out.PointA, out.PointB);

		return true;
	}

	if (typeA == ColliderType_Mesh && typeB == ColliderType_Capsule)
	{
		auto capsule = std::static_pointer_cast<CapsuleCollider>(b);
		auto mesh = std::static_pointer_cast<MeshCollider>(a);
		_bool hit = intersectCapsuleMesh(capsule, mesh, out);

		if (!hit)
		{
			return false;
		}

		return true;
	}

	return false;
}

engine::_bool engine::CollisionManager::intersectCapsuleMesh(const SharedPtr<CapsuleCollider>& capsuleCol,
	const SharedPtr<MeshCollider>& meshCol, Contact& out)
{
	using namespace DirectX;

	auto worldMat = meshCol->GetTransform()->GetWorldMatrix();
	_matrix invW = XMMatrixInverse(nullptr, worldMat);
	const auto& capsule = capsuleCol->GetCapsule();


	_vector worldP0 = capsule.P0W.ToVector();
	_vector worldP1 = capsule.P1W.ToVector();
	_float worldR = capsule.Radius;

	_vector localP0 = XMVector3TransformCoord(worldP0, invW);
	_vector localP1 = XMVector3TransformCoord(worldP1, invW);

	_float invScale = XMVectorGetX(XMVector3Length(invW.r[0]));
	_float localR = worldR * invScale;

	const auto& nodes = meshCol->GetMesh().BVHNodes;
	const auto& triOrder = meshCol->GetMesh().BVHTriangles;
	const auto& idx = meshCol->GetMesh().Indices;
	const auto& vtx = meshCol->GetMesh().Vertices;

	out.IsHit = false;
	_float bestPen = FLT_MAX;
	XMVECTOR bestN = XMVectorZero();
	XMVECTOR bestPA = XMVectorZero(), bestPB = XMVectorZero();

	std::function<void(int)> Traverse = [&](int nodeIdx)
		{
			const auto& N = nodes[nodeIdx];
			// 광역 테스트: 세그먼트 vs 확장 AABB
			if (!segmentAABBIntersect(localP0, localP1, N.Bounds, localR))
				return;

			if (N.Left < 0)
			{
				// 리프: 삼각형 하나씩 검사
				int start = ~N.Left, cnt = ~N.Right;
				for (int i = 0; i < cnt; ++i)
				{
					int ti = triOrder[start + i];
					XMVECTOR v0 = XMLoadFloat3(&vtx[idx[ti * 3 + 0]].Position);
					XMVECTOR v1 = XMLoadFloat3(&vtx[idx[ti * 3 + 1]].Position);
					XMVECTOR v2 = XMLoadFloat3(&vtx[idx[ti * 3 + 2]].Position);

					XMVECTOR pSeg, pTri;
					_float   d2 = segmentTriangleDistSq(localP0, localP1, v0, v1, v2, pSeg, pTri);
					
					if (d2 > localR * localR)
						continue;

					// 3) 충돌 깊이·법선·접촉점 계산
					_float dist = std::sqrt(d2);
					_float pen = localR - dist;
					XMVECTOR diff = pSeg - pTri;

					XMVECTOR normalLoc;
					if (XMVectorGetX(XMVector3LengthSq(diff)) > 1e-8f) 
					{
						normalLoc = XMVector3Normalize(diff);
					}
					else 
					{
						// 삼각형 법선 또는 세그먼트 축으로 대체
						normalLoc = XMVector3Normalize(XMVector3Cross(v1 - v0, v2 - v0));
					}


					// 로컬 > 월드 변환
					XMVECTOR worldNormal = XMVector3Normalize(XMVector3TransformNormal(normalLoc, worldMat));
					XMVECTOR worldSeg = XMVector3TransformCoord(pSeg, worldMat);
					XMVECTOR worldTri = XMVector3TransformCoord(pTri, worldMat);
					XMVECTOR worldPA = worldSeg - worldNormal * pen;  // 캡슐 표면 위
					XMVECTOR worldPB = worldTri;             // 삼각형 상의 점
					_float	worldPen = pen / invScale;

					//XMVECTOR worldPB = XMVector3TransformCoord(pTri, worldMat);
					//// 캡슐 표면 접촉점: segment 방향으로 radius 만큼 들어간 점
					//XMVECTOR worldPA = worldPB + worldNormal * worldR;

					if (!out.IsHit || worldPen > bestPen)
					{
						out.IsHit = true;
						bestPen = worldPen;
						bestN = worldNormal;
						bestPA = worldPA;
						bestPB = worldPB;
					}
				}
			}
			else
			{
				Traverse(N.Left);
				Traverse(N.Right);
			}
		};

	Traverse(0);

	if (!out.IsHit)
	{
		return false;
	}

	out.Normal = Vector3::FromVector(bestN);
	out.Penetration = bestPen;
	out.PointA = Vector3::FromVector(bestPA);
	out.PointB = Vector3::FromVector(bestPB);
	return true;
}

engine::_bool engine::CollisionManager::checkCapsuleSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
                                                           Contact& out)
{
	return false;
}

engine::_bool engine::CollisionManager::checkMeshMesh(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	return false;
}

engine::_bool engine::CollisionManager::checkMeshSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	return false;
}

engine::_bool engine::CollisionManager::checkSphereSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	return false;
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
	//std::cerr << "Penetration : " << c.Penetration << "\n";
	_float correction = std::max(c.Penetration - slop, 0.f) * percent / totalInv;
	Vector3 corr = c.Normal * correction;

	//std::cerr << "Correction : " << correction << "\n";


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

	if (invMassSum == 0.f)
	{
		return;
	}
	_float j = -(1.f + restitution) * velAlongN / invMassSum;

	Vector3 impulse = c.Normal * j;
	vA -= impulse * a->GetInvMass();
	vB += impulse * b->GetInvMass();

	a->Velocity() = vA;
	b->Velocity() = vB;
}

engine::OBB engine::CollisionManager::worldOBBToLocalOBB(const OBB& worldBox, const _matrix& meshWorld)
{
	using namespace DirectX;

	_matrix invMeshWorld = DirectX::XMMatrixInverse(nullptr, meshWorld);

	OBB localBox;

	_vector localCenter = XMVector3TransformCoord(worldBox.Center.ToVector(), invMeshWorld);

	_vector wx = worldBox.AxisX.ToVector() * worldBox.Extents.Value.x;
	_vector wy = worldBox.AxisY.ToVector() * worldBox.Extents.Value.y;
	_vector wz = worldBox.AxisZ.ToVector() * worldBox.Extents.Value.z;

	_vector lx = XMVector3TransformNormal(wx, invMeshWorld);
	_vector ly = XMVector3TransformNormal(wy, invMeshWorld);
	_vector lz = XMVector3TransformNormal(wz, invMeshWorld);

	_float ex = XMVectorGetX(XMVector3Length(lx));
	_float ey = XMVectorGetX(XMVector3Length(ly));
	_float ez = XMVectorGetX(XMVector3Length(lz));

	localBox.Center = Vector3::FromVector(localCenter);

	localBox.AxisX = Vector3::FromVector(XMVectorScale(lx, 1.0f / ex));
	localBox.AxisY = Vector3::FromVector(XMVectorScale(ly, 1.0f / ey));
	localBox.AxisZ = Vector3::FromVector(XMVectorScale(lz, 1.0f / ez));

	localBox.Extents = Vector3{ ex, ey, ez };

	return localBox;
}

engine::_bool engine::CollisionManager::AABBvsOBB(const AABBData& aabb, const OBB& obb)
{
	using namespace DirectX;

	_float3 cA, eA;
	cA.x = (aabb.Min.x + aabb.Max.x) * 0.5f;
	cA.y = (aabb.Min.y + aabb.Max.y) * 0.5f;
	cA.z = (aabb.Min.z + aabb.Max.z) * 0.5f;
	eA.x = (aabb.Max.x - aabb.Min.x) * 0.5f;
	eA.y = (aabb.Max.y - aabb.Min.y) * 0.5f;
	eA.z = (aabb.Max.z - aabb.Min.z) * 0.5f;

	_vector Aaxis[3] = {
		XMVectorSet(1,0,0,0),
		XMVectorSet(0,1,0,0),
		XMVectorSet(0,0,1,0)
	};

	_vector BoxAxis[3] = {
		XMLoadFloat3(&obb.AxisX.Value),
		XMLoadFloat3(&obb.AxisY.Value),
		XMLoadFloat3(&obb.AxisZ.Value)
	};

	_float R[3][3], absR[3][3];
	const _float EPSILON = 1e-6f;
	for (_int i = 0; i < 3; ++i)
	{
		for (_int j = 0; j < 3; ++j)
		{
			R[i][j] = XMVectorGetX(XMVector3Dot(Aaxis[i], BoxAxis[j]));
			absR[i][j] = std::fabs(R[i][j]) + EPSILON;
		}
	}

	_vector vA = XMVectorSet(cA.x, cA.y, cA.z, 0);
	_vector vB = XMLoadFloat3(&obb.Center.Value);
	_vector tVec = vB - vA;

	_float tA[3];
	for (_int i = 0; i < 3; ++i)
	{
		tA[i] = XMVectorGetX(XMVector3Dot(tVec, Aaxis[i]));
	}

	_float eB[3] = {
	obb.Extents.Value.x,
	obb.Extents.Value.y,
	obb.Extents.Value.z
	};

	_float ra, rb;

	for (int i = 0; i < 3; ++i)
	{
		ra = (&eA.x)[i];
		rb = eB[0] * absR[i][0] + eB[1] * absR[i][1] + eB[2] * absR[i][2];
		if (std::fabs(tA[i]) > ra + rb)
		{
			return false;
		}
	}

	for (int j = 0; j < 3; ++j)
	{
		float tB = XMVectorGetX(XMVector3Dot(tVec, BoxAxis[j]));
		ra = eA.x * absR[0][j] + eA.y * absR[1][j] + eA.z * absR[2][j];
		rb = (&eB[0])[j];
		if (std::fabs(tB) > ra + rb)
		{
			return false;
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			// ra = sum of A's extents projected onto this axis
			ra = (&eA.x)[(i + 1) % 3] * absR[(i + 2) % 3][j] + (&eA.x)[(i + 2) % 3] * absR[(i + 1) % 3][j];
			// rb = sum of B's extents projected onto this axis
			rb = (&eB[0])[(j + 1) % 3] * absR[i][(j + 2) % 3] + (&eB[0])[(j + 2) % 3] * absR[i][(j + 1) % 3];
			// t 테스트 값
			float tVal = std::fabs(
				tA[(i + 2) % 3] * R[(i + 1) % 3][j]
				- tA[(i + 1) % 3] * R[(i + 2) % 3][j]
			);
			if (tVal > ra + rb)
			{
				return false;
			}
		}
	}

	return true;
}

engine::_bool engine::CollisionManager::triangleOBBIntersect(const OBB& box, const _vector& v0, const _vector& v1,
	const _vector& v2, _vector& outAxis, _float& outDepth)
{
	using namespace DirectX;
	const float EPSILON = 1e-6f;

	_float bestOverlap = FLT_MAX;
	_vector bestAxis = XMVectorZero();

	// 삼각형 정점 -> 박스 센터 기준으로 이동
	XMVECTOR C = box.Center.ToVector();

	XMVECTOR tv0 = v0 - C;
	XMVECTOR tv1 = v1 - C;
	XMVECTOR tv2 = v2 - C;

	// 박스 축 & half extents
	XMVECTOR U[3] = {
		box.AxisX.ToVector(),
		box.AxisY.ToVector(),
		box.AxisZ.ToVector()
	};
	float e[3] = { box.Extents.Value.x, box.Extents.Value.y, box.Extents.Value.z };

	// 삼각형 에지
	XMVECTOR E[3] = {
		tv1 - tv0,
		tv2 - tv1,
		tv0 - tv2
	};

	// 박스 축 3개 테스트
	for (int i = 0; i < 3; ++i) 
	{
		// 삼각형을 U[i] 축에 투영
		float p0 = XMVectorGetX(XMVector3Dot(U[i], tv0));
		float p1 = XMVectorGetX(XMVector3Dot(U[i], tv1));
		float p2 = XMVectorGetX(XMVector3Dot(U[i], tv2));
		float triMin = std::min({ p0, p1, p2 });
		float triMax = std::max({ p0, p1, p2 });

		// 박스를 이 축에 투영한 간격은 [-e[i], +e[i]]
		float overlap = std::min(triMax, e[i])
			- std::max(triMin, -e[i]);

		if (overlap < 0)
		{
			return false;  // 분리축 발견
		}

		if (overlap < bestOverlap)
		{
			bestOverlap = overlap;
			// 어느 쪽이 더 짧게 빠져나오는지에 따라 축 방향 결정
			bestAxis = (e[i] - triMax < triMin + e[i])
				? U[i]
				: XMVectorNegate(U[i]);
		}
	}

	// 삼각형 법선 축 테스트
	XMVECTOR N = XMVector3Cross(E[0], E[1]);
	_float len2 = XMVectorGetX(XMVector3LengthSq(N));

	if (len2 < EPSILON)
	{
		return false;
	}

	XMVECTOR triN = XMVector3Normalize(N);

	// plane distance
	float d = XMVectorGetX(XMVector3Dot(triN, tv0));
	// box 투영 반경
	float r =
		e[0] * fabsf(XMVectorGetX(XMVector3Dot(triN, U[0]))) +
		e[1] * fabsf(XMVectorGetX(XMVector3Dot(triN, U[1]))) +
		e[2] * fabsf(XMVectorGetX(XMVector3Dot(triN, U[2])));
	_float overlapN = r - fabsf(d);
	if (overlapN < 0)
	{
		return false;
	}

	if (overlapN < bestOverlap)
	{
		bestOverlap = overlapN;
		// 법선 방향은 삼각형이 박스 안으로 침투된 방향
		bestAxis = (d < 0) ? XMVectorNegate(triN) : triN;
	}

	// 크로스 축 9개 테스트
	for (int i = 0; i < 3; ++i) 
	{
		for (int j = 0; j < 3; ++j) 
		{
			XMVECTOR axis = XMVector3Cross(U[i], E[j]);
			_float l2 = XMVectorGetX(XMVector3LengthSq(axis));
			if (l2 < 1e-6f)
			{
				continue;
			}
			axis = XMVectorScale(axis, 1.0f / sqrtf(l2));


			// 삼각형 투영
			_float t0 = XMVectorGetX(XMVector3Dot(axis, tv0));
			_float t1 = XMVectorGetX(XMVector3Dot(axis, tv1));
			_float t2 = XMVectorGetX(XMVector3Dot(axis, tv2));
			_float triMin = std::min({ t0, t1, t2 });
			_float triMax = std::max({ t0, t1, t2 });

			// 박스 반경
			_float r_cross =
				e[0] * fabsf(XMVectorGetX(XMVector3Dot(axis, U[0]))) +
				e[1] * fabsf(XMVectorGetX(XMVector3Dot(axis, U[1]))) +
				e[2] * fabsf(XMVectorGetX(XMVector3Dot(axis, U[2])));

			_float overlap = std::min(triMax, r_cross) - std::max(triMin, -r_cross);
			if (overlap < 0)
			{
				return false;
			}


			if (overlap < bestOverlap)
			{
				bestOverlap = overlap;
				bestAxis = (r_cross - triMax < triMin + r_cross)
					? axis
					: XMVectorNegate(axis);
			}
		}
	}

	outAxis = bestAxis;
	outDepth = bestOverlap;

	return true;
}

engine::_bool engine::CollisionManager::segmentAABBIntersect(DirectX::XMVECTOR p0, DirectX::XMVECTOR p1,
	const AABBData& box, float radius)
{
	using namespace DirectX;

	// AABB 확장
	AABBData b = box;
	b.Min.x -= radius; b.Min.y -= radius; b.Min.z -= radius;
	b.Max.x += radius; b.Max.y += radius; b.Max.z += radius;

	// 세그먼트 엔드포인트 최소/최대
	XMFLOAT3 f0, f1, mn, mx;
	XMStoreFloat3(&f0, p0);
	XMStoreFloat3(&f1, p1);
	mn.x = std::min(f0.x, f1.x);
	mn.y = std::min(f0.y, f1.y);
	mn.z = std::min(f0.z, f1.z);
	mx.x = std::max(f0.x, f1.x);
	mx.y = std::max(f0.y, f1.y);
	mx.z = std::max(f0.z, f1.z);

	// 축별 분리축 체크
	if (mx.x < b.Min.x || mn.x > b.Max.x) return false;
	if (mx.y < b.Min.y || mn.y > b.Max.y) return false;
	if (mx.z < b.Min.z || mn.z > b.Max.z) return false;

	return true;
}

engine::_float engine::CollisionManager::segmentTriangleDistSq(_vector segA, _vector segB, _vector v0, _vector v1,
	_vector v2, _vector& outSeg, _vector& outTri)
{
	using namespace DirectX;

	// 1) 삼각형 평면과 세그먼트의 교차점 검사
	XMVECTOR ab = v1 - v0;
	XMVECTOR ac = v2 - v0;
	XMVECTOR n = XMVector3Cross(ab, ac);
	float nLen2 = XMVectorGetX(XMVector3LengthSq(n));
	if (nLen2 > 1e-8f)
	{
		XMVECTOR nN = XMVector3Normalize(n);
		XMVECTOR dir = segB - segA;
		float denom = XMVectorGetX(XMVector3Dot(nN, dir));
		if (fabsf(denom) > 1e-6f)
		{
			float t = XMVectorGetX(XMVector3Dot(nN, v0 - segA)) / denom;
			if (t >= 0.0f && t <= 1.0f)
			{
				XMVECTOR ip = segA + dir * t;
				// 삼각형 내부 검사 (바리센트릭)
				XMVECTOR v0p = ip - v0;
				float d00 = XMVectorGetX(XMVector3Dot(ab, ab));
				float d01 = XMVectorGetX(XMVector3Dot(ab, ac));
				float d11 = XMVectorGetX(XMVector3Dot(ac, ac));
				float d20 = XMVectorGetX(XMVector3Dot(v0p, ab));
				float d21 = XMVectorGetX(XMVector3Dot(v0p, ac));
				float denom2 = d00 * d11 - d01 * d01;
				float v = (d11 * d20 - d01 * d21) / denom2;
				float w = (d00 * d21 - d01 * d20) / denom2;
				float u = 1.0f - v - w;
				if (u >= 0 && v >= 0 && w >= 0)
				{
					outSeg = ip;
					outTri = ip;
					return 0.0f;
				}
			}
		}
	}

	// 2) 교차점 없으면, 다음 후보들 중 최소를 고른다.
	float best = FLT_MAX;

	// 2a) 세그먼트 엔드포인트 -> 삼각형
	{
		XMVECTOR c0 = closestPtPointTriangle(segA, v0, v1, v2);
		float d0 = XMVectorGetX(XMVector3LengthSq(segA - c0));
		if (d0 < best) { best = d0; outSeg = segA; outTri = c0; }
	}
	{
		XMVECTOR c1 = closestPtPointTriangle(segB, v0, v1, v2);
		float d1 = XMVectorGetX(XMVector3LengthSq(segB - c1));
		if (d1 < best) { best = d1; outSeg = segB; outTri = c1; }
	}

	// 2b) 세그먼트 <-> 삼각형 엣지 세그먼트
	{
		XMVECTOR cS, cT;
		float d = closestPtSegmentSegment(segA, segB, v0, v1, cS, cT);
		if (d < best) { best = d; outSeg = cS; outTri = cT; }
	}
	{
		XMVECTOR cS, cT;
		float d = closestPtSegmentSegment(segA, segB, v1, v2, cS, cT);
		if (d < best) { best = d; outSeg = cS; outTri = cT; }
	}
	{
		XMVECTOR cS, cT;
		float d = closestPtSegmentSegment(segA, segB, v2, v0, cS, cT);
		if (d < best) { best = d; outSeg = cS; outTri = cT; }
	}

	return best;
}

engine::_vector engine::CollisionManager::closestPtPointTriangle(_vector p, _vector a, _vector b, _vector c)
{
	using namespace DirectX;

	// from Christer Ericson, Real Time Collision Detection, p.141
	XMVECTOR ab = b - a;
	XMVECTOR ac = c - a;
	XMVECTOR ap = p - a;

	float d1 = XMVectorGetX(XMVector3Dot(ab, ap));
	float d2 = XMVectorGetX(XMVector3Dot(ac, ap));
	if (d1 <= 0 && d2 <= 0) return a;              // region A

	XMVECTOR bp = p - b;
	float d3 = XMVectorGetX(XMVector3Dot(ab, bp));
	float d4 = XMVectorGetX(XMVector3Dot(ac, bp));
	if (d3 >= 0 && d4 <= d3) return b;             // region B

	float vc = d1 * d4 - d3 * d2;
	if (vc <= 0 && d1 >= 0 && d3 <= 0)             // edge AB
	{
		float v = d1 / (d1 - d3);
		return a + ab * v;
	}

	XMVECTOR cp = p - c;
	float d5 = XMVectorGetX(XMVector3Dot(ab, cp));
	float d6 = XMVectorGetX(XMVector3Dot(ac, cp));
	if (d6 >= 0 && d5 <= d6) return c;             // region C

	float vb = d5 * d2 - d1 * d6;
	if (vb <= 0 && d2 >= 0 && d6 <= 0)             // edge AC
	{
		float w = d2 / (d2 - d6);
		return a + ac * w;
	}

	float va = d3 * d6 - d5 * d4;
	if (va <= 0 && (d4 - d3) >= 0 && (d5 - d6) >= 0) // edge BC
	{
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		return b + (c - b) * w;
	}

	// region inside face
	XMVECTOR normal = XMVector3Cross(ab, ac);
	normal = XMVector3Normalize(normal);
	float dist = XMVectorGetX(XMVector3Dot(p - a, normal));
	return p - normal * dist;
}

engine::_float engine::CollisionManager::closestPtSegmentSegment(_vector p1, _vector q1, _vector p2, _vector q2,
	_vector& c1, _vector& c2)
{
	using namespace DirectX;

	// from Christer Ericson, Real Time Collision Detection, p.149
	XMVECTOR d1 = q1 - p1; // segment1 direction
	XMVECTOR d2 = q2 - p2; // segment2 direction
	XMVECTOR r = p1 - p2;
	float a = XMVectorGetX(XMVector3Dot(d1, d1));
	float e = XMVectorGetX(XMVector3Dot(d2, d2));
	float f = XMVectorGetX(XMVector3Dot(d2, r));

	float s, t;
	if (a <= 1e-6f && e <= 1e-6f)
	{
		// both segments degenerate to points
		s = t = 0.0f;
		c1 = p1;
		c2 = p2;
	}
	else if (a <= 1e-6f)
	{
		// first segment degenerate to point
		s = 0.0f;
		t = f / e;
		t = Clamp(t, 0.0f, 1.0f);
	}
	else
	{
		float c = XMVectorGetX(XMVector3Dot(d1, r));
		if (e <= 1e-6f)
		{
			// second segment degenerate
			t = 0.0f;
			s = Clamp(-c / a, 0.0f, 1.0f);
		}
		else
		{
			float b = XMVectorGetX(XMVector3Dot(d1, d2));
			float denom = a * e - b * b;
			if (denom != 0.0f)
				s = Clamp((b * f - c * e) / denom, 0.0f, 1.0f);
			else
				s = 0.0f;
			t = (b * s + f) / e;
			if (t < 0.0f)
			{
				t = 0.0f;
				s = Clamp(-c / a, 0.0f, 1.0f);
			}
			else if (t > 1.0f)
			{
				t = 1.0f;
				s = Clamp((b - c) / a, 0.0f, 1.0f);
			}
		}
	}

	c1 = p1 + d1 * s;
	c2 = p2 + d2 * t;
	return XMVectorGetX(XMVector3LengthSq(c1 - c2));
}

IMPLEMENT_SINGLETON(engine::CollisionManager)

