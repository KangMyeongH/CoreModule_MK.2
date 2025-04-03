#include "Prefab.h"

#include "GameObject.h"

engine::Prefab::Prefab() = default;

engine::Prefab::Prefab(const SharedPtr<GameObject>& root)
{
	m_TemplateRoot = root->Clone(PREFAB);
}

engine::Prefab::Prefab(const Prefab& rhs)
	: m_TemplateRoot(rhs.m_TemplateRoot)
{
	
}

engine::Prefab& engine::Prefab::operator=(const Prefab& rhs)
{
	m_TemplateRoot = rhs.m_TemplateRoot;

	return *this;
}

void engine::Prefab::to_json(nlohmann::ordered_json& j)
{
	j = nlohmann::ordered_json
	{
		{"GameObjects", nlohmann::ordered_json::array()}
	};

	nlohmann::ordered_json objJson;

	GameObject::ToJson(objJson, m_TemplateRoot, PREFAB);

	j["GameObjects"].push_back(objJson);
}

void engine::Prefab::from_json(const nlohmann::ordered_json& j)
{
	for (const auto& objJson : j.at("GameObjects"))
	{
		SharedPtr<GameObject> obj = GameObject::Create("GameObject", PREFAB);
		GameObject::FromJson(objJson, obj, PREFAB);
		m_TemplateRoot = obj;
	}
}