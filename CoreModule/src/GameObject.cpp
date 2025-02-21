#include "GameObject.h"

#include "Scene.h"

engine::GameObject::GameObject(): Object("GameObject"), m_Transform(Transform::create()), m_bActiveSelf(true), m_bStatic(false)
{
}

engine::GameObject::~GameObject()
{
}

void engine::GameObject::SetActive(const bool active)
{
	if (m_bActiveSelf != active)
	{
		m_bActiveSelf = active;
	}
}

bool engine::GameObject::IsActive() const
{
	return m_bActiveSelf;
}

void engine::GameObject::SetTag(const _string& tag)
{
	Scene::GetInstance().UpdateGameObjectTag(std::dynamic_pointer_cast<GameObject>(shared_from_this()), tag);
	m_Tag = tag;
}

engine::SharedPtr<engine::GameObject> engine::GameObject::create()
{
	return {
	new GameObject(),
	[](const GameObject* ptr) { delete ptr; }
	};
}

engine::SharedPtr<engine::GameObject> engine::GameObject::Clone() const
{
	SharedPtr<GameObject> clone(CLONE_SHARED_PTR(GameObject));

	clone->m_Transform->SetOwner(clone);

	for (const auto& pair : clone->m_Components)
	{
		for (const auto& component : pair.second)
		{
			component->SetOwner(clone);
		}
	}

	for (const auto& child : m_Transform->m_Children)
	{
		if (const auto originChild = child->GetGameObject().lock())
		{
			SharedPtr<GameObject> newChild = originChild->Clone();
			newChild->m_Transform->SetOwner(newChild);
			newChild->m_Transform->SetParent(clone->GetTransform());
		}
	}

	return clone;
}

void engine::GameObject::Destroy()
{
	if (!m_bDestroyed)
	{
		m_bDestroyed = true;

		for (auto& pair : m_Components)
		{
			for (auto& component : pair.second)
			{
				component->Destroy();
			}
		}

		for (auto& child : m_Transform->m_Children)
		{
			if (auto childObj = child->GetGameObject().lock())
			{
				childObj->Destroy();
			}
		}
	}
}

void engine::to_json(nlohmann::ordered_json& j, const SharedPtr<GameObject>& obj)
{
	j = nlohmann::ordered_json
	{
		{"instanceID", obj->GetInstanceID()},
		{"name", obj->GetName()},
		{"tag", obj->GetTag()},
		{"active", obj->IsActive()},
		{"transform", obj->m_Transform},
		{"components", nlohmann::ordered_json::array()}
	};

	for (auto& component : obj->m_Components)
	{
		nlohmann::ordered_json componentJson;
		component.second.front()->to_json(componentJson);
		j["components"].push_back(componentJson);
	}
}

void engine::from_json(const nlohmann::ordered_json& j, const SharedPtr<GameObject>& obj)
{
	obj->SetInstanceID(j.at("instanceID").get<int>());
	_string name;
	j.at("name").get_to(name);
	obj->SetName(name);
	obj->SetTag(j.at("tag").get<_string>());
	obj->SetActive(j.at("active").get<bool>());
	j.at("transform").get_to(obj->m_Transform);
}
