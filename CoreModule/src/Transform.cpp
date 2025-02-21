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
