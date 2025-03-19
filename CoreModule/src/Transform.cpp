#include "Transform.h"
#include "GameObject.h"

engine::Transform::Transform(const Transform& rhs)
	: Component(rhs),
	m_Parent(rhs.m_Parent),
	m_ParentID(rhs.m_ParentID),
	m_WorldMatrix(),
	m_LocalPosition(rhs.m_LocalPosition),
	m_LocalRotation(rhs.m_LocalRotation),
	m_LocalScale(rhs.m_LocalScale),
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
			oldParent->detachChild(std::static_pointer_cast<Transform>(shared_from_this()));
		}

		m_Parent = parent;

		m_LocalPosition = m_WorldPosition;
		m_LocalRotation = m_WorldRotation;
		m_LocalScale = m_WorldScale;

		if (parent != nullptr)
		{
			m_ParentID = parent->GetInstanceID();

			_matrix parentWorldMat = parent->GetWorldMatrix();
			_matrix parentInverseMatrix = XMMatrixInverse(nullptr, parentWorldMat);

			_matrix localMat = XMMatrixMultiply(GetWorldMatrix(), parentInverseMatrix);

			_vector localScale, localRot, localPosition;
			XMMatrixDecompose(&localScale, &localRot, &localPosition, localMat);

			m_LocalScale = localScale;
			m_LocalRotation = localRot;
			m_LocalPosition = localPosition;

			parent->addChild(std::static_pointer_cast<Transform>(shared_from_this()));
		}

		else
		{
			m_ParentID = -1;
		}

		setDirty();
	}
}

std::vector<engine::SharedPtr<engine::Transform>>* engine::Transform::GetChildren()
{
	return &m_Children;
}

engine::_int engine::Transform::GetParentID() const
{
	return m_ParentID;
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
		[](const Transform* ptr) { delete ptr; }
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
	// TODO : 하다 말았음 비정방형 스케일 + 회전을 섞어서 비틀림이 발생. 해?결

	if (!m_bDirty)
	{
		return;
	}

	// 로컬 행렬 계산
	const _matrix matScale 	= DirectX::XMMatrixScalingFromVector(m_LocalScale.ToVector());
	const _matrix matRot 	= DirectX::XMMatrixRotationQuaternion(m_LocalRotation.ToVector());
	const _matrix matTrans 	= DirectX::XMMatrixTranslationFromVector(m_LocalPosition.ToVector());
	const _matrix matLocal 	= matScale * matRot * matTrans;

	if (const auto parent = m_Parent.lock())
	{
		const _matrix matParent = parent->GetWorldMatrix();
		const _matrix matWorld 	= XMMatrixMultiply(matLocal, matParent);

		_vector wScale, wRot, wTrans;
		XMMatrixDecompose(&wScale, &wRot, &wTrans, matWorld);
		XMStoreFloat4x4(&m_WorldMatrix, matWorld);

		m_WorldScale 	= Vector3::FromVector(wScale);
		m_WorldRotation = Quaternion::FromVector(wRot);
		m_WorldPosition = Vector3::FromVector(wTrans);
	}

	else
	{
		XMStoreFloat4x4(&m_WorldMatrix, matLocal);
		m_WorldPosition = m_LocalPosition;
		m_WorldRotation = m_LocalRotation;
		m_WorldScale 	= m_LocalScale;
	}

	m_bDirty = false;
}

void engine::Transform::addChild(const SharedPtr<Transform>& child)
{
	m_Children.push_back(child);
}

void engine::Transform::to_json(nlohmann::ordered_json& j)
{
	j = nlohmann::ordered_json{
		{"position", GetLocalPosition()},
		{"rotation", GetLocalRotation()},
		{"scale", GetLocalScale()},
		{"euler", m_LocalEulerAngles}
	};
}

void engine::Transform::from_json(const nlohmann::ordered_json& j)
{
	SetLocalPosition(j.at("position").get<Vector3>());
	SetLocalRotation(j.at("rotation").get<Quaternion>());
	SetLocalScale(j.at("scale").get<Vector3>());
}

void engine::to_json(nlohmann::ordered_json& j, const SharedPtr<Transform>& t)
{
	_int parentID = -1;

	if (auto parent = t->m_Parent.lock())
	{
		parentID = parent->GetInstanceID();
	}

	j = nlohmann::ordered_json{
		{"parentID", parentID},
		{"instanceID", t->GetInstanceID()},
		{"position", t->GetLocalPosition()},
		{"rotation", t->GetLocalRotation()},
		{"scale", t->GetLocalScale()}
	};
}

void engine::from_json(const nlohmann::ordered_json& j, const SharedPtr<Transform>& t)
{
	_int parentID;
	j.at("parentID").get_to(parentID);
	t->m_ParentID = parentID;
	t->SetInstanceID(j.at("instanceID").get<_int>());
	t->SetLocalPosition(j.at("position").get<Vector3>());
	t->SetLocalRotation(j.at("rotation").get<Quaternion>());
	t->SetLocalScale(j.at("scale").get<Vector3>());
}
