#include "CollisionManager.h"

#include "BoxCollider.h"
#include "CapsuleCollider.h"
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
	_matrix toLocal = XMMatrixTranspose(R) * T;
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
	return false;
}

engine::_bool engine::CollisionManager::checkBoxSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	return false;
}

engine::_bool engine::CollisionManager::checkCapsuleCapsule(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	return false;
}

engine::_bool engine::CollisionManager::checkCapsuleMesh(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b,
	Contact& out)
{
	return false;
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

	_float correction = std::max(c.Penetration - slop, 0.f) * percent / totalInv;
	//_float correction = c.Penetration;
	Vector3 corr = c.Normal * correction;
	std::cerr << "penetration : " << c.Penetration << "\n";
	std::cerr << "Normal :  X : " << c.Normal.Value.x << ", Y : " << c.Normal.Value.y << ", Z : " << c.Normal.Value.z << "\n";
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

