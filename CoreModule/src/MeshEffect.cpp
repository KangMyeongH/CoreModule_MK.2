#include "MeshEffect.h"

#include "LoadManager.h"
#include "Mesh.h"

void engine::MeshEffect::SetMesh(const _wstring& modelPath, _int meshIdx)
{
	const auto& modelData = LoadManager::GetInstance().ReadModelDataFromFile(modelPath);
	const auto& meshData = modelData.Meshes[meshIdx];

	SharedPtr<Mesh> mesh = Mesh::Create(meshData.VIBuffer);
	std::vector<SubMesh> subMeshes;

	for (const auto& subMeshData : meshData.SubMeshes)
	{
		SubMesh subMesh;
		subMesh.IndexOffset = subMeshData.IndexOffset;
		subMesh.IndexCount = subMeshData.IndexCount;
		subMesh.MaterialIndex = subMeshData.MaterialIndex;

		subMeshes.push_back(subMesh);
	}


	mesh->SetSubMesh(subMeshes);

	m_Mesh = mesh;
}

void engine::MeshEffect::Bind(const ComPtr<ID3D11DeviceContext>& context)
{
	if (m_Mesh)
	{
		m_Mesh->Bind(context);
	}
}

void engine::MeshEffect::Render(const ComPtr<ID3D11DeviceContext>& context)
{

}

void engine::MeshEffect::Destroy()
{
}
