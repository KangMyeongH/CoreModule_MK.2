#include "Scene.h"

#include <fstream>

#include "CollisionManager.h"
#include "GameObject.h"
#include "LoadManager.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "PhysicsManager.h"
#include "Prefab.h"
#include "PrefabManager.h"
#include "RenderManager.h"
#include "ScriptBehaviour.h"
#include "SkinnedMeshRenderer.h"
#include "TimeManager.h"
#include "UIManager.h"

IMPLEMENT_SINGLETON(engine::Scene)

engine::Scene::Scene() = default;

engine::Scene::~Scene()
{

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

	return true;
}

void engine::Scene::ChangeScene(const _wstring& sceneName)
{
	m_NextScene = sceneName;
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

void engine::Scene::RegisterNextScene()
{
	if (!m_NextScene.empty())
	{
		if (!m_LoadingThread.joinable())
		{
			const _wstring basePath = L"..\\Client\\Assets\\Scenes\\";
			const _wstring fileExtension = L".Scene";
			const _wstring fullPath = basePath + m_NextScene + fileExtension;

			m_bSceneLoaded = false;
			m_LoadingThread = std::thread(&Scene::loadSceneInBackGround, this, fullPath);
		}

		while (true)
		{
			MSG msg;
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// TODO : ·Îµù ¾À ·»´õ¸µ

			{
				std::unique_lock<std::mutex> lock(m_LoadingMutex);
				if (m_bSceneLoaded)
				{
					break;
				}
			}

			Sleep(16);
		}

		TimeManager::GetInstance().Initialize();

		m_NextScene.clear();

		if (m_LoadingThread.joinable())
		{
			m_LoadingThread.join();
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
		if (obj->GetTransform()->GetParentID() == -1)
		{
			nlohmann::ordered_json objJson;
			GameObject::ToJson(objJson, obj, CLIENT);
			j["GameObjects"].push_back(objJson);
		}
	}

	return j;
}

void engine::Scene::From_Json(const nlohmann::ordered_json& j)
{
	m_SceneName = j.at("sceneName").get<_string>();

	for (const auto& objJson : j.at("GameObjects"))
	{
		_string assetPath;
		objJson.at("assetPath").get_to(assetPath);

		_wstring ext = GetFileExtensionW(StringToWString(assetPath));

		if (ext == L"prefab")
		{
			auto gameObject = PrefabManager::GetInstance().GetPrefab(StringToWString(assetPath));
		}

		else
		{
			auto gameObject = GameObject::Create("GameObject", CLIENT);
			GameObject::FromJson(objJson, gameObject, CLIENT);
		}
	}

	for (auto& gameObject : m_GameObjects)
	{
		std::wstring path = gameObject->GetAssetPath();

		_wstring ext = GetFileExtensionW(path);

		if (ext == L"model")
		{
			auto modelData = LoadManager::GetInstance().ReadModelDataFromFile(path);

			for (auto& meshData : modelData.Meshes)
			{
				auto meshObj = gameObject->FindGameObject(meshData.MeshName);
				SharedPtr<Mesh> mesh = Mesh::Create(meshData.VIBuffer);

				if (meshData.IsSkinned)
				{
					auto skinnedComponent = meshObj->GetComponent<SkinnedMeshRenderer>();

					Skeleton skeleton;

					for (auto& boneData : meshData.Bones)
					{
						auto boneObj = gameObject->FindGameObject(boneData.BoneName);

						Bone bone;
						bone.Name = boneData.BoneName;
						bone.ParentIndex = boneData.parentIndex;
						bone.Transform = boneObj->GetTransform();
						memcpy(&bone.Offset, boneData.offsetMatrix, sizeof(boneData.offsetMatrix));

						if (bone.ParentIndex == -1)
						{
							skeleton.RootBone = bone.Transform;
						}

						skeleton.Bones.push_back(bone);
					}

					skinnedComponent->SetSkeleton(skeleton);

					std::vector<SubMesh> subMeshes;

					for (auto& subMeshData : meshData.SubMeshes)
					{
						SubMesh subMesh;

						subMesh.IndexOffset = subMeshData.IndexOffset;
						subMesh.IndexCount = subMeshData.IndexCount;
						subMesh.MaterialIndex = subMeshData.MaterialIndex;

						subMeshes.push_back(subMesh);
					}

					mesh->SetSubMesh(subMeshes);

					skinnedComponent->SetMesh(mesh);
				}

				else
				{
					auto meshRendererComponent = meshObj->GetComponent<MeshRenderer>();

					std::vector<SubMesh> subMeshes;

					for (auto& subMeshData : meshData.SubMeshes)
					{
						SubMesh subMesh;
						subMesh.IndexOffset = subMeshData.IndexOffset;
						subMesh.IndexCount = subMeshData.IndexCount;
						subMesh.MaterialIndex = subMeshData.MaterialIndex;

						subMeshes.push_back(subMesh);
					}

					mesh->SetSubMesh(subMeshes);

					meshRendererComponent->SetMesh(mesh);
				}
			}
		}
	}
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
	//gameObject->m_Transform->SetOwner(gameObject);
}

void engine::Scene::loadSceneInBackGround(const std::wstring& nextScene)
{
	Release();
	ScriptBehaviourManager::GetInstance().Release();
	CollisionManager::GetInstance().Release();
	PhysicsManager::GetInstance().Release();
	RenderManager::GetInstance().Release();
	UIManager::GetInstance().Release();

	loadSceneData(nextScene);

	{
		std::lock_guard<std::mutex> lock(m_LoadingMutex);
		m_bSceneLoaded = true;
	}

	m_CV.notify_one();
}

engine::_bool engine::Scene::loadSceneData(const _wstring& path)
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

	return true;
}
