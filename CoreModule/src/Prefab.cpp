#include "Prefab.h"

#include "GameObject.h"

engine::Prefab::Prefab() = default;

engine::Prefab::Prefab(const SharedPtr<GameObject>& root)
{
	m_TemplateRoot = root->Clone(PREFAB);
	RegenerateInstanceID(m_TemplateRoot);
}

engine::Prefab::Prefab(const Prefab& rhs)
	: m_IdGenerator(rhs.m_IdGenerator.load()),
	m_TemplateRoot(rhs.m_TemplateRoot),
	m_GameObjects(rhs.m_GameObjects)
{
	
}

engine::Prefab& engine::Prefab::operator=(const Prefab& rhs)
{
	m_IdGenerator = rhs.m_IdGenerator.load();
	m_TemplateRoot = rhs.m_TemplateRoot;
	m_GameObjects = rhs.m_GameObjects;

	return *this;
}

void engine::Prefab::RegenerateInstanceID(const SharedPtr<GameObject>& gameObject)
{
	if (m_IdGenerator == 0)
	{
		m_TemplateRoot = gameObject;
	}

	m_GameObjects.push_back(gameObject);
	gameObject->SetInstanceID(m_IdGenerator++);
	gameObject->GetTransform()->SetInstanceID(m_IdGenerator++);

	for (auto& componentPair : gameObject->GetComponents())
	{
		for (auto& component : componentPair.second)
		{
			component->SetInstanceID(m_IdGenerator++);
		}
	}

	for (auto& child : *gameObject->GetTransform()->GetChildren())
	{
		child->SetParentID(gameObject->GetTransform()->GetInstanceID());

		RegenerateInstanceID(child->GetGameObject().lock());
	}
}

void engine::Prefab::to_json(nlohmann::ordered_json& j)
{
	j = nlohmann::ordered_json
	{
		{"GameObjects", nlohmann::ordered_json::array()}
	};

	for (auto gameObjects : m_GameObjects)
	{
		j["GameObjects"].push_back(gameObjects);
	}
}

void engine::Prefab::from_json(const nlohmann::ordered_json& j)
{
	std::unordered_map<_int, SharedPtr<Transform>> transformMap;

	for (const auto& objJson : j.at("GameObjects"))
	{
		SharedPtr<GameObject> obj = GameObject::Create("GameObject", PREFAB);
		objJson.get_to(obj);
		m_GameObjects.push_back(obj);
		_int id = obj->GetTransform()->GetInstanceID();
		transformMap[id] = obj->GetTransform();
	}

	for (const auto& gameObject : m_GameObjects)
	{
		const SharedPtr<Transform> transform = gameObject->GetTransform();

		_int parentID = transform->GetParentID();

		if (parentID != -1)
		{
			auto it = transformMap.find(parentID);
			if (it != transformMap.end())
			{
				transform->SetParent(transformMap[parentID]);
			}
		}

		else if (parentID == -1)
		{
			m_TemplateRoot = gameObject;
		}
	}

	transformMap.clear();
}