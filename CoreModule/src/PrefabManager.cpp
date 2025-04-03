#include "PrefabManager.h"

#include "GameObject.h"
#include "LoadManager.h"
#include "Prefab.h"

engine::PrefabManager::PrefabManager() = default;

engine::PrefabManager::~PrefabManager() = default;

engine::Prefab& engine::PrefabManager::GetPrefab(const _wstring& path)
{
	auto it = m_PrefabMap.find(path);

	if (it == m_PrefabMap.end())
	{
		return AddPrefab(path);
	}

	return it->second;
}

engine::Prefab& engine::PrefabManager::AddPrefab(const _wstring& path)
{
	auto it = m_PrefabMap.find(path);

	if (it != m_PrefabMap.end())
	{
		return it->second;
	}

	Prefab prefab;

	LoadManager::GetInstance().LoadPrefab(prefab, path);

	auto result = m_PrefabMap.emplace(path, prefab);

	return result.first->second;
}

void engine::PrefabManager::MakePrefab(const SharedPtr<GameObject>& gameObject, const _wstring& path)
{
	std::wstring fullPath = path + L"\\" + StringToWString(gameObject->GetName()) + L".prefab";

	gameObject->SetAssetPath(fullPath);
	Prefab prefab(gameObject);
	LoadManager::GetInstance().SavePrefab(prefab, path);

	m_PrefabMap[fullPath] = prefab;
}

void engine::PrefabManager::AddTempGameObject(const SharedPtr<GameObject>& gameObject)
{
	m_TempGameObjects.push_back(gameObject);
}

void engine::PrefabManager::Release()
{
	m_PrefabMap.clear();
}

IMPLEMENT_SINGLETON(engine::PrefabManager)
