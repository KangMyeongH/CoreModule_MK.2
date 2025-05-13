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

void engine::MeshEffect::InputAssembler(ID3D11DeviceContext* context)
{
}

void engine::MeshEffect::Bind(ID3D11DeviceContext* context)
{
	if (m_Mesh)
	{
		m_Mesh->Bind(context);
	}
}

void engine::MeshEffect::Render(ID3D11DeviceContext* context)
{

}

void engine::MeshEffect::PreRender(ID3D11DeviceContext* context, const _float4X4& viewMat, const _float4X4& projMat)
{
	
}

void engine::MeshEffect::Destroy()
{
}

void engine::MeshEffect::registerComponent(ApplicationMode mode)
{
	Renderer::registerComponent(mode);
}

void engine::MeshEffect::to_json(nlohmann::ordered_json& j)
{
}

void engine::MeshEffect::from_json(const nlohmann::ordered_json& j)
{
}
