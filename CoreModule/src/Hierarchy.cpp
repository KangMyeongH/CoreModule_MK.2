#include "Hierarchy.h"

#include "EditorComponentManager.h"
#include "GameObject.h"
#include "Renderer.h"
#include "UI.h"

IMPLEMENT_SINGLETON(engine::editor::Hierarchy)

engine::editor::Hierarchy::Hierarchy() = default;

engine::editor::Hierarchy::~Hierarchy()
{
	Release();
}

void engine::editor::Hierarchy::SetCurrentSceneName(const _string& name)
{
	m_CurrentSceneName = name;
}

engine::_string engine::editor::Hierarchy::GetCurrentSceneName() const
{
	return m_CurrentSceneName;
}

engine::SharedPtr<engine::GameObject> engine::editor::Hierarchy::AddGameObject()
{
	SharedPtr<GameObject> newGameObject{
		new GameObject(), [](const GameObject* ptr) { delete ptr; }
	};

	m_GameObjects.push_back(newGameObject);
	newGameObject->m_Transform->SetOwner(newGameObject);

	return newGameObject;
}

void engine::editor::Hierarchy::AddGameObject(const SharedPtr<GameObject>& gameObject)
{
	m_GameObjects.push_back(gameObject);
}

void engine::editor::Hierarchy::RemoveGameObject(const SharedPtr<GameObject>& gameObject)
{
	if (gameObject != nullptr)
	{
		gameObject->Destroy();
	}
}

void engine::editor::Hierarchy::FlushDestroyGameObject()
{
	for (auto it = m_GameObjects.begin(); it != m_GameObjects.end();)
	{
		if ((*it)->IsDestroyed())
		{
			auto target = *it;
			if (auto parent = target->GetTransform()->GetParent())
			{
				target->GetTransform()->SetParent(nullptr);
			}
			it = m_GameObjects.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::editor::Hierarchy::Release()
{
	m_GameObjects.clear();
	EditorComponentManager::GetInstance().Release();
}

void engine::editor::Hierarchy::setupTransformHierarchy() const
{
	std::unordered_map<_int, SharedPtr<Transform>> transformMap;

	transformMap.reserve(m_GameObjects.size());

	for (const auto& gameObject : m_GameObjects)
	{
		_int id = gameObject->GetTransform()->GetInstanceID();
		transformMap[id] = gameObject->GetTransform();
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
	}

	transformMap.clear();
}

nlohmann::ordered_json engine::editor::Hierarchy::ToJson() const
{
	nlohmann::ordered_json j = nlohmann::ordered_json
	{
		{"sceneName", m_CurrentSceneName},
		{"GameObjects", nlohmann::ordered_json::array()}
	};

	for (const auto& obj : m_GameObjects)
	{
		j["GameObjects"].push_back(obj);
	}

	return j;
}

void engine::editor::Hierarchy::FromJson(const nlohmann::ordered_json& j)
{
	if (!m_GameObjects.empty())
	{
		// 기존에 불러온 Editor hierarchy의 데이터 삭제
		Release();
	}

	m_CurrentSceneName = j.at("sceneName").get<_string>();

	for (const auto& objJson : j.at("GameObjects"))
	{
		auto gameObject = AddGameObject();
		objJson.get_to(gameObject);
	}

	for (auto& gameObject : m_GameObjects)
	{
		for (auto& pair : gameObject->GetComponents())
		{
			for (auto& component : pair.second)
			{
				EditorComponentManager::GetInstance().AddComponent(gameObject, component);
			}
		}
	}

	setupTransformHierarchy();
}
