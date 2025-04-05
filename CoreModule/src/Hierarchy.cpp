#include "Hierarchy.h"

#include "EditorComponentManager.h"
#include "GameObject.h"
#include "LoadManager.h"
#include "Material.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Prefab.h"
#include "PrefabManager.h"
#include "Renderer.h"
#include "SkinnedMeshRenderer.h"
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

void engine::editor::Hierarchy::AddModel(const ModelData& model, const _wstring& path)
{
	auto rootObj = GameObject::Create(model.ModelName, EDITOR);

	rootObj->SetAssetPath(path);

	for (auto& meshData : model.Meshes)
	{
		SharedPtr<Mesh> mesh = Mesh::Create(meshData.VIBuffer);

		auto meshObj = GameObject::Create(meshData.MeshName, EDITOR);

		meshObj->GetTransform()->SetLocalPosition({ meshData.tx, meshData.ty, meshData.tz });
		meshObj->GetTransform()->SetLocalRotation({ meshData.rx, meshData.ry, meshData.rz, meshData.rw });
		meshObj->GetTransform()->SetLocalScale({ meshData.sx, meshData.sy, meshData.sz });

		meshObj->GetTransform()->SetParent(rootObj->GetTransform());

		if (meshData.IsSkinned)
		{
			SharedPtr<Component> component = ComponentFactory::GetInstance().CreateComponent("SkinnedMeshRenderer");
			component->SetOwner(meshObj);
			meshObj->m_Components[typeid(*component)].push_back(component);
			EditorComponentManager::GetInstance().AddComponent(meshObj, component);

			Skeleton skeleton;

			for (auto& boneData : meshData.Bones)
			{
				auto boneObj = GameObject::Create(boneData.BoneName, EDITOR);

				boneObj->GetTransform()->SetLocalPosition({ boneData.tx, boneData.ty, boneData.tz });
				boneObj->GetTransform()->SetLocalRotation({ boneData.rx, boneData.ry, boneData.rz, boneData.rw });
				boneObj->GetTransform()->SetLocalScale({ boneData.sx, boneData.sy, boneData.sz });

				Bone bone;
				bone.Name = boneData.BoneName;
				bone.ParentIndex = boneData.parentIndex;
				bone.Transform = boneObj->GetTransform();
				memcpy(&bone.Offset, boneData.offsetMatrix, sizeof(boneData.offsetMatrix));

				skeleton.Bones.push_back(bone);
			}

			for (auto& bone : skeleton.Bones)
			{
				int parentIndex = bone.ParentIndex;

				if (parentIndex >= 0)
				{
					bone.Transform->SetParent(skeleton.Bones[parentIndex].Transform);
				}

				else
				{
					bone.Transform->SetParent(rootObj->GetTransform());
					skeleton.RootBone = bone.Transform;
				}
			}

			std::static_pointer_cast<SkinnedMeshRenderer>(component)->SetSkeleton(skeleton);

			//==============SubMesh=====================
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
			//==========================================

			for (auto& materialData : meshData.Materials)
			{
				auto material = Material::Create(component);
				material->SetName(materialData.MaterialName);
				material->LoadShader(L"..\\Client\\Assets\\Resource\\Shader\\SkinnedMeshShader.hlsl");

				std::static_pointer_cast<SkinnedMeshRenderer>(component)->SetMaterial(material, materialData.MaterialIndex);
			}

			std::static_pointer_cast<SkinnedMeshRenderer>(component)->SetMesh(mesh);
		}

		else
		{
			SharedPtr<Component> component = ComponentFactory::GetInstance().CreateComponent("MeshRenderer");
			component->SetOwner(meshObj);
			meshObj->m_Components[typeid(*component)].push_back(component);
			EditorComponentManager::GetInstance().AddComponent(meshObj, component);

			//==============SubMesh=====================
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
			//==========================================

			for (auto& materialData : meshData.Materials)
			{
				auto material = Material::Create(component);
				material->SetName(materialData.MaterialName);
				material->LoadShader(L"..\\Client\\Assets\\Resource\\Shader\\MeshShader.hlsl");

				std::static_pointer_cast<MeshRenderer>(component)->SetMaterial(material, materialData.MaterialIndex);
			}

			std::static_pointer_cast<MeshRenderer>(component)->SetMesh(mesh);
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
		if (obj->GetTransform()->GetParentID() == -1)
		{
			nlohmann::ordered_json objJson;

			GameObject::ToJson(objJson, obj, EDITOR);

			j["GameObjects"].push_back(objJson);
		}
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
		_string assetPath;
		objJson.at("assetPath").get_to(assetPath);

		_wstring ext = GetFileExtensionW(StringToWString(assetPath));

		if (ext == L"prefab")
		{
			auto gameObject = PrefabManager::GetInstance().GetPrefab(StringToWString(assetPath)).GetRoot()->Clone(EDITOR);
		}

		else
		{
			auto gameObject = GameObject::Create("GameObject", EDITOR);
			GameObject::FromJson(objJson, gameObject, EDITOR);
		}
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
