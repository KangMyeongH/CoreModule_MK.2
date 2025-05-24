#include "Effect.h"

#include "EditorComponentManager.h"
#include "LoadManager.h"
#include "Mesh.h"
#include "RenderManager.h"

engine::Effect::Effect(const SharedPtr<GameObject>& owner, const _string& name)
	: Behaviour(owner, name), m_bWrap(true), m_bClamp(false)
{
}

engine::Effect::Effect(const Effect& rhs)
	: Behaviour(rhs), m_Mesh(rhs.m_Mesh), m_bWrap(rhs.m_bWrap), m_bClamp(rhs.m_bClamp), m_ModelPath(rhs.m_ModelPath)
{
}

void engine::Effect::SetMesh(const _wstring& modelPath)
{
	const auto& modelData = LoadManager::GetInstance().ReadModelDataFromFile(modelPath);

	m_Mesh = Mesh::Create(modelData.Meshes[0].VIBuffer);
	m_Mesh->SetName(modelData.Meshes[0].MeshName);

	std::vector<SubMesh> subMeshes;

	for (const auto& subMeshData : modelData.Meshes[0].SubMeshes)
	{
		SubMesh subMesh;
		subMesh.IndexOffset = subMeshData.IndexOffset;
		subMesh.IndexCount = subMeshData.IndexCount;
		subMesh.MaterialIndex = subMeshData.MaterialIndex;

		subMeshes.push_back(subMesh);
	}

	m_Mesh->SetSubMesh(subMeshes);
	m_ModelPath = modelPath;
}

void engine::Effect::SetMaterial(const _wstring& materialPath) const
{
	LoadManager::GetInstance().LoadMaterialData(m_Material, materialPath);
}

void engine::Effect::SetMaterial(const SharedPtr<Material>& material)
{
	m_Material = material;
}

void engine::Effect::SetWrap()
{
	m_bWrap = true;
	m_bClamp = false;
}

void engine::Effect::SetClamp()
{
	m_bWrap = false;
	m_bClamp = true;
}

void engine::Effect::registerComponent(ApplicationMode mode)
{
	if (mode == CLIENT)
	{
		RenderManager::GetInstance().AddEffect(std::static_pointer_cast<Effect>(shared_from_this()));
	}

	if (mode == EDITOR)
	{
		editor::EditorComponentManager::GetInstance().AddComponent(std::static_pointer_cast<Effect>(shared_from_this()));
	}
}
