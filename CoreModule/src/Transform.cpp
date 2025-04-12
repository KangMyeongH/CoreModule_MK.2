#include "Transform.h"
#include "GameObject.h"

engine::Transform::Transform(const Transform& rhs)
	: Component(rhs),
	m_Parent(),
	m_ParentID(-1),
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
	if (parent.get() == this)
	{
		return;
	}

	auto oldParent = m_Parent.lock();

	if (oldParent != parent)
	{
		_matrix worldMatrixBeforeChange = GetWorldMatrix();

		if (oldParent != nullptr)
		{
			oldParent->detachChild(std::static_pointer_cast<Transform>(shared_from_this()));
		}

		//m_LocalPosition = Position();
		//m_LocalRotation = Rotation();
		//m_LocalScale = Scale();

		m_Parent = parent;

		if (parent != nullptr)
		{
			m_ParentID = parent->GetInstanceID();

			_matrix parentWorldMat = parent->GetWorldMatrix();
			_matrix parentInverseMatrix = XMMatrixInverse(nullptr, parentWorldMat);
			_matrix newLocalMatrix = worldMatrixBeforeChange * parentInverseMatrix;

			_vector localScale, localRot, localPosition;
			XMMatrixDecompose(&localScale, &localRot, &localPosition, newLocalMatrix);

			m_LocalScale = localScale;
			m_LocalRotation = localRot;
			m_LocalPosition = localPosition;

			parent->addChild(std::static_pointer_cast<Transform>(shared_from_this()));
		}

		else
		{
			m_ParentID = -1;

			_vector scale, rot, pos;
			XMMatrixDecompose(&scale, &rot, &pos, worldMatrixBeforeChange);

			m_LocalScale = scale;
			m_LocalRotation = rot;
			m_LocalPosition = pos;
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

void engine::Transform::SetParentID(const _int id)
{
	m_ParentID = id;
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

void engine::Transform::LookAt(SharedPtr<Transform> target)
{
	Vector3 forward = (target->Position() - Position()).Normalized();
	Vector3 up = Vector3::Up();

	if (Vector3::Dot(forward, up) > 0.999f)
	{
		up = Vector3::Forward();
	}

	Vector3 right = Vector3::Cross(up, forward).Normalized();
	Vector3 newUp = Vector3::Cross(forward, right);


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
		const _matrix matWorld 	= matLocal * matParent;

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

}

void engine::Transform::from_json(const nlohmann::ordered_json& j)
{

}

void engine::to_json(nlohmann::ordered_json& j, const SharedPtr<Transform>& t)
{
	j = nlohmann::ordered_json{
		{"position", t->GetLocalPosition()},
		{"rotation", t->GetLocalRotation()},
		{"scale", t->GetLocalScale()}
	};
}

void engine::from_json(const nlohmann::ordered_json& j, const SharedPtr<Transform>& t)
{
	t->SetLocalPosition(j.at("position").get<Vector3>());
	t->SetLocalRotation(j.at("rotation").get<Quaternion>());
	t->SetLocalScale(j.at("scale").get<Vector3>());
	t->setDirty();
}
