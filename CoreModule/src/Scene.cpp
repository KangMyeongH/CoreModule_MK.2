#include "Scene.h"

#include <fstream>

#include "GameObject.h"
#include "Test.h"

IMPLEMENT_SINGLETON(engine::Scene)

engine::Scene::Scene() = default;

engine::Scene::~Scene()
{
	Release();
}

engine::GameObjects* engine::Scene::GetGameObjects()
{
	return &m_GameObjects;
}

bool engine::Scene::Initialize(const _wstring& path)
{
	if (path.empty())
	{
		return false;
	}

	std::ifstream inFile(path);
	if (!inFile.is_open())
	{
		return false;
	}
	nlohmann::ordered_json j;
	inFile >> j;
	From_Json(j);
	inFile.clear();
	inFile.close();

	//SharedPtr<GameObject> temp = CreateGameObject("Test");
	//SharedPtr<Test> test = temp->AddComponent<Test>();

	return true;
}

void engine::Scene::ChangeScene(const _wstring& sceneName)
{
}

engine::SharedPtr<engine::GameObject> engine::Scene::Find(const _string& name)
{
	for (auto& gameObject : m_GameObjects)
	{
		if (gameObject->GetName() == name)
		{
			return gameObject;
		}
	}

	return nullptr;
}

std::vector<engine::SharedPtr<engine::GameObject>> engine::Scene::FindGameObjectsWithTag(const _string& tag)
{
	std::vector<SharedPtr<GameObject>> result;

	auto it = m_GameObjectsTagMap.find(tag);
	if (it != m_GameObjectsTagMap.end())
	{
		result.reserve(it->second.size());

		for (auto& gameObject : it->second)
		{
			result.push_back(gameObject);
		}
	}

	return result;
}

engine::SharedPtr<engine::GameObject> engine::Scene::FindWithTag(const _string& tag)
{
	auto it = m_GameObjectsTagMap.find(tag);
	if (it != m_GameObjectsTagMap.end())
	{
		return *it->second.begin();
	}
	return nullptr;
}

void engine::Scene::updateGameObjectTag(const SharedPtr<GameObject>& obj, const _string& newTag)
{
	_string oldTag = obj->GetTag();

	if (oldTag == newTag)
	{
		return;
	}

	auto tagIt = m_GameObjectsTagMap.find(oldTag);
	if (tagIt != m_GameObjectsTagMap.end())
	{
		tagIt->second.erase(obj);
		if (tagIt->second.empty())
		{
			m_GameObjectsTagMap.erase(tagIt);
		}
	}

	m_GameObjectsTagMap[newTag].insert(obj);
}

void engine::Scene::FlushDestroyGameObjects()
{
	for (auto it = m_GameObjects.begin(); it != m_GameObjects.end();)
	{
		if ((*it)->IsDestroyed())
		{
			_string tag = (*it)->GetTag();

			auto tagIt = m_GameObjectsTagMap.find(tag);

			if (tagIt != m_GameObjectsTagMap.end())
			{
				tagIt->second.erase(*it);
				if (tagIt->second.empty())
				{
					m_GameObjectsTagMap.erase(tagIt);
				}
			}

			it = m_GameObjects.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::Scene::Release()
{
	m_GameObjects.clear();
	m_GameObjectsTagMap.clear();
}

nlohmann::ordered_json engine::Scene::To_Json() const
{
	nlohmann::ordered_json j = nlohmann::ordered_json
	{
		{"sceneName", m_SceneName},
		{"GameObjects", nlohmann::ordered_json::array()}
	};

	for (const auto& obj : m_GameObjects)
	{
		j["GameObjects"].push_back(obj);
	}

	return j;
}

void engine::Scene::From_Json(const nlohmann::ordered_json& j)
{
	m_SceneName = j.at("sceneName").get<_string>();
	for (const auto& objJson : j.at("GameObjects"))
	{
		SharedPtr<GameObject> obj = GameObject::Create();
		objJson.get_to(obj);
	}

	setupTransformHierarchy();
}

void engine::Scene::setupTransformHierarchy() const
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

void engine::Scene::registerGameObject(const SharedPtr<GameObject>& gameObject)
{
	m_GameObjects.push_back(gameObject);
	m_GameObjectsTagMap[gameObject->GetTag()].insert(gameObject);
	gameObject->m_Transform->SetOwner(gameObject);
}
