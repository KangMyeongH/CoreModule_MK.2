#include "GameObject.h"

#include "Hierarchy.h"
#include "PrefabManager.h"
#include "Scene.h"

engine::GameObject::GameObject() : Object("GameObject"), m_Transform(Transform::create()), m_bActiveSelf(true),
                                   m_bActiveInHierarchy(true), m_bStatic(false)
{
}

engine::GameObject::~GameObject()
{
	if (auto parent = m_Transform->GetParent())
	{
		const auto children = m_Transform->GetParent()->GetChildren();
		children->erase(std::remove(children->begin(), children->end(), m_Transform), children->end());
	}
}

void engine::GameObject::SetActive(const bool active)
{
	if (m_bActiveSelf != active)
	{
		m_bActiveSelf = active;

		updateActiveHierarchy();
	}
}

bool engine::GameObject::IsActive() const
{
	return m_bActiveSelf && m_bActiveInHierarchy;
}

void engine::GameObject::SetTag(const _string& tag)
{
	Scene::GetInstance().updateGameObjectTag(std::static_pointer_cast<GameObject>(shared_from_this()), tag);
	m_Tag = tag;
}

engine::SharedPtr<engine::GameObject> engine::GameObject::Create(const _string& name, ApplicationMode mode)
{
	SharedPtr<GameObject> newGameObject{
		new GameObject(), [](const GameObject* ptr) { delete ptr; }
	};

	if (mode == CLIENT)
	{
		Scene::GetInstance().registerGameObject(newGameObject);
	}

	else if (mode == EDITOR)
	{
		editor::Hierarchy::GetInstance().AddGameObject(newGameObject);
	}

	else if (mode == PREFAB)
	{
		PrefabManager::GetInstance().AddTempGameObject(newGameObject);
	}

	newGameObject->m_Transform->SetOwner(newGameObject);

	return newGameObject;
}

engine::SharedPtr<engine::GameObject> engine::GameObject::Clone(const ApplicationMode mode) const
{
	SharedPtr<GameObject> clone(CLONE_SHARED_PTR(GameObject));

	if (mode == CLIENT)
	{
		Scene::GetInstance().registerGameObject(clone);
	}

	else if (mode == EDITOR)
	{
		editor::Hierarchy::GetInstance().AddGameObject(clone);
	}

	else if (mode == PREFAB)
	{
		PrefabManager::GetInstance().AddTempGameObject(clone);
	}

	clone->m_Transform->SetOwner(clone);

	for (const auto& pair : clone->m_Components)
	{
		for (const auto& component : pair.second)
		{
			component->SetOwner(clone);
			component->registerComponent(mode);
		}
	}

	for (const auto& child : m_Transform->m_Children)
	{
		if (const auto originChild = child->GetGameObject().lock())
		{
			SharedPtr<GameObject> newChild = originChild->Clone(mode);
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
	obj->SetInstanceID(j.at("instanceID").get<_int>());
	_string name;
	j.at("name").get_to(name);
	obj->SetName(name);
	obj->SetTag(j.at("tag").get<_string>());
	obj->SetActive(j.at("active").get<_bool>());
	j.at("transform").get_to(obj->m_Transform);

	for (const auto& component_json : j.at("components"))
	{
		_string type = component_json.at("type").get<_string>();

		SharedPtr<Component> component = ComponentFactory::GetInstance().CreateComponent(type);
		component->from_json(component_json);
		component->SetOwner(obj);
		obj->m_Components[typeid(*component)].push_back(component);
	}
}
