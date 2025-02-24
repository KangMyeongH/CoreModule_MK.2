#include "Transform.h"
#include "GameObject.h"

namespace engine
{
	std::unordered_map<int, SharedPtr<Transform>> Transform::s_TransformMap;
	std::unordered_map<int, std::vector<SharedPtr<Transform>>> Transform::s_ChildTransformMap;
}

engine::Transform::Transform(const Transform& rhs)
	: Component(rhs),
	m_Parent(rhs.m_Parent),
	m_bDirty(true)
{
	m_Children.reserve(rhs.m_Children.size());
}

engine::SharedPtr<engine::Transform> engine::Transform::GetParent() const
{
	return m_Parent.lock();
}

void engine::Transform::SetParent(const SharedPtr<Transform>& parent)
{
	auto oldParent = m_Parent.lock();

	if (oldParent != parent)
	{
		if (oldParent != nullptr)
		{
			oldParent->detachChild(std::dynamic_pointer_cast<Transform>(shared_from_this()));
		}

		m_Parent = parent;

		// TODO : 아래 주석 해제 해야함.
		m_LocalPosition = m_WorldPosition;
		m_LocalRotation = m_WorldRotation;
		m_LocalScale = m_WorldScale;

		if (parent != nullptr)
		{
			// TODO : 계층 구조 기준으로 즉 부모의 Transform 정보 기준으로 LocalSpace 연산을 해줘야함

			parent->addChild(std::dynamic_pointer_cast<Transform>(shared_from_this()));
		}
	}
}

void engine::Transform::detachChild(const SharedPtr<Transform>& child)
{
	auto it = std::remove(m_Children.begin(), m_Children.end(), child);

	if (it != m_Children.end())
	{
		m_Children.erase(it, m_Children.end());
	}
}

void engine::Transform::setDirty()
{
	m_bDirty = true;

	if (!m_Children.empty())
	{
		for (auto& it : m_Children)
		{
			it->setDirty();
		}
	}
}

engine::SharedPtr<engine::Transform> engine::Transform::create()
{
	return {
		new Transform(nullptr),
		[](const Transform* ptr) {delete ptr; }
	};
}

void engine::Transform::Destroy()
{
}

std::shared_ptr<engine::Component> engine::Transform::Clone() const
{
	SharedPtr<Transform> clone(CLONE_SHARED_PTR(Transform));

	return clone;
}

void engine::Transform::updateMatrixIfNeeded() const
{
	// TODO : 하다 말았음 비정방형 스케일 + 회전을 섞어서 Shear가 발생.
	if (!m_bDirty)
	{
		return;
	}

	// 로컬 행렬 계산
	_matrix matScale = DirectX::XMMatrixScalingFromVector(m_LocalScale.ToVector());
	_matrix matRot = DirectX::XMMatrixRotationQuaternion(m_LocalRotation.ToVector());
	_matrix matTrans = DirectX::XMMatrixTranslationFromVector(m_LocalPosition.ToVector());

	_matrix matLocal = matScale * matRot * matTrans;

	if (auto parent = m_Parent.lock())
	{
		_matrix matParent = parent->GetWorldMatrix();
		_matrix matWorld = XMMatrixMultiply(matLocal, matParent);
		_vector wScale, wRot, wTrans;
		XMMatrixDecompose(&wScale, &wRot, &wTrans, matWorld);

		XMStoreFloat4x4(&m_WorldMatrix, matWorld);
		m_WorldScale = Vector3::FromVector(wScale);
		m_WorldRotation = Quaternion::FromVector(wRot);
		m_WorldPosition = Vector3::FromVector(wTrans);
	}

	else
	{
		XMStoreFloat4x4(&m_WorldMatrix, matLocal);
		m_WorldPosition = m_LocalPosition;
		m_WorldRotation = m_LocalRotation;
		m_WorldScale = m_LocalScale;
	}

	m_bDirty = false;
}

void engine::Transform::addChild(const SharedPtr<Transform>& child)
{
	m_Children.push_back(child);
}

void engine::Transform::to_json(nlohmann::ordered_json& j)
{
	j = nlohmann::ordered_json{};
}

void engine::Transform::from_json(const nlohmann::ordered_json& j)
{
}

void engine::to_json(nlohmann::ordered_json& j, const SharedPtr<Transform>& t)
{
	int parentID = -1;
	if (auto parent = t->m_Parent.lock())
	{
		parentID = parent->GetInstanceID();
	}

	j = nlohmann::ordered_json{
		{"parentID", parentID},
		{"instanceID", t->GetInstanceID()},
		// TODO : add position rotation sca1e
	};
}

void engine::from_json(const nlohmann::ordered_json& j, SharedPtr<Transform>& t)
{
	int parentID;
	j.at("parentID").get_to(parentID);
	if (parentID != -1)
	{
		Transform::s_ChildTransformMap[parentID].push_back(t);
	}
}
