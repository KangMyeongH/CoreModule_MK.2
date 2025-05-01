#include "LoadManager.h"
#include <ShObjIdl.h>

#include "D3D11Manager.h"
#include "EditorComponentManager.h"
#include "Hierarchy.h"
#include "Material.h"
#include "MeshRenderer.h"
#include "Prefab.h"
#include "Renderer.h"
#include "Scene.h"
#include "TimeManager.h"

engine::LoadManager::LoadManager()
{
}

engine::LoadManager::~LoadManager()
{
}

engine::_bool engine::LoadManager::LoadProject(const std::string& path)
{
	// TODO : ProjectSetting.json 파일을 불러와서 읽고 초기 세팅 하기.




	// TODO : Resource에 있는 모든 파일들 불러오기

	return false;
}

engine::_bool engine::LoadManager::CreateNewScene(const _string& sceneName, const _wstring& path)
{
	if (path.empty())
	{
		return false;
	}

	std::ifstream inFile(path);

	if (!inFile.is_open())

	return false;
}

engine::_bool engine::LoadManager::LoadSceneData(const _wstring& path)
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
	editor::Hierarchy::GetInstance().FromJson(j);

	inFile.clear();
	inFile.close();
}

engine::_bool engine::LoadManager::SaveSceneData()
{
	nlohmann::ordered_json j = editor::Hierarchy::GetInstance().ToJson();
	//nlohmann::ordered_json j = GameEngine::Scene::GetInstance().To_Json();

	std::wstring basePath = L"..\\Client\\Assets\\Scenes\\";
	std::wstring sceneName = StringToWString(editor::Hierarchy::GetInstance().GetCurrentSceneName());
	std::wstring fullPath = basePath + sceneName + L".Scene";

	std::ofstream outFile(fullPath, std::ios::trunc);
	if (!outFile.is_open())
	{
		return false;
	}
	outFile << j.dump(4);
	outFile.close();

	return false;
}

engine::_bool engine::LoadManager::LoadMaterialData(const SharedPtr<Material>& material, const _wstring& path)
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

	material->from_json(j);

	material->SetPath(path);

	return true;
}

engine::_bool engine::LoadManager::SaveMaterialData(const SharedPtr<Material>& material, const _wstring& path)
{
	nlohmann::ordered_json j;
	material->to_json(j);

	auto renderer = std::static_pointer_cast<Renderer>(material->GetOwner());
	_wstring materialName = StringToWString(material->GetName());
	std::wstring fullPath = path + L"\\" + materialName + L".mat";

	std::ofstream outFile(fullPath, std::ios::trunc);
	if (!outFile.is_open())
	{
		return false;
	}

	outFile << j.dump(4);
	outFile.close();

	return true;
}

engine::_bool engine::LoadManager::LoadPrefab(Prefab& prefab, const _wstring& path)
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

	prefab.from_json(j);

	return true;
}

engine::_bool engine::LoadManager::SavePrefab(Prefab& prefab, const _wstring& path)
{
	nlohmann::ordered_json j;
	prefab.to_json(j);

	auto gameObject = prefab.GetRoot();
	_wstring name = StringToWString(gameObject->GetName());
	std::wstring fullPath = path + L"\\" + name + L".prefab";

	std::ofstream outFile(fullPath, std::ios::trunc);
	if (!outFile.is_open())
	{
		return false;;
	}

	outFile << j.dump(4);
	outFile.close();

	return true;
}

void engine::LoadManager::LoadHierarchyToScene()
{
	Scene::GetInstance().From_Json(editor::Hierarchy::GetInstance().ToJson());
	TimeManager::GetInstance().Initialize();
}

engine::_wstring engine::LoadManager::OpenFileDialog()
{
	wchar_t currentDir[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, currentDir);
	std::wcout << currentDir << "\n";
	OPENFILENAME ofn;
	wchar_t filePath[MAX_PATH] = L"";
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFilter = L"All Files\0*.*\0Text Files\0*.TXT\0"; // 필터 설정
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = L"../Client/Assets/Scenes";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameW(&ofn))
	{
		SetCurrentDirectoryW(currentDir);
		return std::basic_string<wchar_t>(filePath);
	}

	return L"";
}

engine::_wstring engine::LoadManager::BrowseFolderDialog()
{
	HRESULT hr = CoInitialize(nullptr); // COM 초기화
	if (FAILED(hr)) {
		std::wcerr << L"COM 초기화 실패" << std::endl;
		return L"";
	}

	IFileDialog* pFileDialog = nullptr;

	// IFileDialog 생성
	hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
	if (SUCCEEDED(hr)) {
		// 폴더 선택 모드 활성화
		DWORD dwOptions;
		pFileDialog->GetOptions(&dwOptions);
		pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS);

		// 다이얼로그 제목 설정
		pFileDialog->SetTitle(L"경로를 선택하세요.");

		// 다이얼로그 표시
		hr = pFileDialog->Show(nullptr);
		if (SUCCEEDED(hr)) {
			IShellItem* pItem = nullptr;

			// 선택된 폴더 경로 가져오기
			hr = pFileDialog->GetResult(&pItem);
			if (SUCCEEDED(hr)) {
				PWSTR pszFolderPath = nullptr;
				pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);
				std::wstring folderPath = pszFolderPath;

				// 메모리 해제
				CoTaskMemFree(pszFolderPath);
				pItem->Release();

				pFileDialog->Release();
				CoUninitialize();
				return folderPath;
			}
		}
		pFileDialog->Release();
	}
	CoUninitialize();
	return L""; // 취소 시 빈 문자열 반환
}

void engine::LoadManager::WriteString(std::ofstream& ofs, const std::string& str)
{
	uint32_t len = static_cast<uint32_t>(str.length());
	ofs.write(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
	ofs.write(str.c_str(), len);
}

//=============================================================//
//Version 1 .model File
//[ModelHeader]
//- ModelNameLength(uint32)
//- ModelName(char[])
//
//- MeshCount(uint32)
//
//[MeshData N개]
//[MeshHeader]
//- MeshNameLength(uint32)
//- MeshName(char[])
//- isSkinned(bool)
//- Transform(tx, ty, tz, rx, ry, rz, rw, sx, sy, sz) (float* 10)
//
//- VertexCount(uint32)
//- IndexCount(uint32)
//- MaterialCount(uint32)
//- SubMeshCount(uint32)
//- BoneCount(uint32)
//
//[Vertices]
//- Vertex 구조 * VertexCount
//
//[Indices]
//- uint32 * IndexCount
//
//[MaterialData * N]
//- MaterialNameLength(uint32)
//- MaterialName(char[])
//- MaterialIndex(int32)
//
//[SubMeshData * N]
//- SubMeshNameLength(uint32)
//- SubMeshName(char[])
//- indexOffset(int32)
//- indexCount(int32)
//- MaterialIndex(int32)
//
//[SkinnedData * VertexCount](only if isSkinned)
//- BoneIndices[4](uint32)
//- BoneWeights[4](float)
//
//[BoneData * N]
//- BoneNameLength(uint32)
//- BoneName(char[])
//- Index(int32)
//- parentIndex(int32)
//- Transform(tx, ty, tz, rx, ry, rz, rw, sx, sy, sz) (float* 10)
//- offsetMatrix[16](float[16])
//=============================================================//

void engine::LoadManager::WriteModelDataToFile(const ModelData& model, const std::wstring& path)
{
	std::wstring fileName = path + L"\\" + StringToWString(model.ModelName) + L".model";

	std::ofstream ofs(fileName, std::ios::binary);
	if (!ofs)
	{
		throw std::runtime_error("ERROR : Failed to open file for writing.");
	}

	// Magic Header
	const char header[4] = { 'M', 'O', 'D', 'L' };
	ofs.write(header, 4);

	// Version
	uint32_t version = 1;
	ofs.write(reinterpret_cast<const char*>(&version), sizeof(uint32_t));

	WriteString(ofs, model.ModelName);

	uint32_t meshCount = static_cast<uint32_t>(model.Meshes.size());
	ofs.write(reinterpret_cast<const char*>(&meshCount), sizeof(uint32_t));

	for (const auto& mesh : model.Meshes)
	{
		WriteString(ofs, mesh.MeshName);

		ofs.write(reinterpret_cast<const char*>(&mesh.IsSkinned), sizeof(bool));

		ofs.write(reinterpret_cast<const char*>(&mesh.tx), sizeof(float) * 10); // tx~sz;

		uint32_t vertexCount = static_cast<uint32_t>(mesh.Vertices.size());
		uint32_t indexCount = static_cast<uint32_t>(mesh.Indices.size());
		uint32_t materialCount = static_cast<uint32_t>(mesh.Materials.size());
		uint32_t subMeshCount = static_cast<uint32_t>(mesh.SubMeshes.size());
		uint32_t boneCount = static_cast<uint32_t>(mesh.Bones.size());

		ofs.write(reinterpret_cast<const char*>(&vertexCount), sizeof(uint32_t));
		ofs.write(reinterpret_cast<const char*>(&indexCount), sizeof(uint32_t));
		ofs.write(reinterpret_cast<const char*>(&materialCount), sizeof(uint32_t));
		ofs.write(reinterpret_cast<const char*>(&subMeshCount), sizeof(uint32_t));
		ofs.write(reinterpret_cast<const char*>(&boneCount), sizeof(uint32_t));

		ofs.write(reinterpret_cast<const char*>(mesh.Vertices.data()), sizeof(VTX_MESH) * vertexCount);
		ofs.write(reinterpret_cast<const char*>(mesh.Indices.data()), sizeof(uint32_t) * indexCount);

		for (const auto& mat : mesh.Materials)
		{
			WriteString(ofs, mat.MaterialName);
			ofs.write(reinterpret_cast<const char*>(&mat.MaterialIndex), sizeof(int32_t));
		}

		for (const auto& sub : mesh.SubMeshes)
		{
			WriteString(ofs, sub.SubMeshName);
			ofs.write(reinterpret_cast<const char*>(&sub.IndexOffset), sizeof(int32_t));
			ofs.write(reinterpret_cast<const char*>(&sub.IndexCount), sizeof(int32_t));
			ofs.write(reinterpret_cast<const char*>(&sub.MaterialIndex), sizeof(int32_t));
		}

		if (mesh.IsSkinned)
		{
			ofs.write(reinterpret_cast<const char*>(mesh.SkinnedData.data()), sizeof(SkinnedData) * vertexCount);

			for (const auto& bone : mesh.Bones)
			{
				WriteString(ofs, bone.BoneName);
				ofs.write(reinterpret_cast<const char*>(&bone.Index), sizeof(int32_t));
				ofs.write(reinterpret_cast<const char*>(&bone.parentIndex), sizeof(int32_t));

				ofs.write(reinterpret_cast<const char*>(&bone.tx), sizeof(float) * 10); // tx~sz
				ofs.write(reinterpret_cast<const char*>(bone.offsetMatrix), sizeof(float) * 16);
			}
		}
	}

	ofs.close();
}

//=============================================================//
//Version 1 .anim File
//
//[MagicHeader]     : "ANIM"
//[Version] : uint32_t
//
//[NameLength] : uint32
//[Name] : char[]
//
//[Duration] : double
//
//[TrackCount] : uint32
//
//[For each BoneKeyFrames]
//[BoneIndex] : int32
//[KeyframeCount] : uint32
//
//[For each Keyframe]
//[Time] : double
//[Translation] : float x, y, z
//[Rotation] : float x, y, z, w
//[Scale] : float x, y, z
//
//Version 2 .anim File
//
//[For each AnimEvent]
//[Time] : float
//[EventName] : char[]
//=============================================================//

void engine::LoadManager::WriteAnimationClipDataToFile(const AnimationClip& clip, const std::wstring& path)
{
	std::wstring fileName = path + L"\\" + StringToWString(clip.Name) + L".anim";

	std::ofstream ofs(fileName, std::ios::binary);
	if (!ofs)
	{
		throw std::runtime_error("Failed to open animation file for writing");
	}

	// Magic Header + Version
	ofs.write("ANIM", 4);
	uint32_t version = 2;
	ofs.write(reinterpret_cast<const char*>(&version), sizeof(uint32_t));

	// Animation Name
	WriteString(ofs, clip.Name);

	// Duration
	ofs.write(reinterpret_cast<const char*>(&clip.Duration), sizeof(double));

	// Track count
	uint32_t trackCount = static_cast<uint32_t>(clip.Tracks.size());
	ofs.write(reinterpret_cast<const char*>(&trackCount), sizeof(uint32_t));

	for (const BoneKeyFrames& track : clip.Tracks)
	{
		ofs.write(reinterpret_cast<const char*>(&track.BoneIndex), sizeof(int32_t));

		uint32_t keyframeCount = static_cast<uint32_t>(track.Frames.size());
		ofs.write(reinterpret_cast<const char*>(&keyframeCount), sizeof(uint32_t));

		for (const Keyframe& kf : track.Frames)
		{
			ofs.write(reinterpret_cast<const char*>(&kf.Time), sizeof(double));
			ofs.write(reinterpret_cast<const char*>(&kf.Translation.Value), sizeof(float) * 3);
			ofs.write(reinterpret_cast<const char*>(&kf.Rotation.Value), sizeof(float) * 4);
			ofs.write(reinterpret_cast<const char*>(&kf.Scale.Value), sizeof(float) * 3);
		}
	}

	// ver 2
	uint32_t eventCount = static_cast<uint32_t>(clip.Events.size());
	ofs.write(reinterpret_cast<const char*>(&eventCount), sizeof(uint32_t));

	for (const AnimationEvent& event : clip.Events)
	{
		ofs.write(reinterpret_cast<const char*>(&event.Time), sizeof(float));
		WriteString(ofs, event.EventName);
	}

	ofs.close();
}

std::string engine::LoadManager::ReadString(std::ifstream& ifs)
{
	uint32_t len = 0;
	ifs.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
	std::string str(len, 0);
	if (len > 0)
	{
		ifs.read(&str[0], len);
	}

	return str;
}

engine::ModelData engine::LoadManager::ReadModelDataFromFile(const std::wstring& path)
{
	auto it = m_Models.find(path);

	if (it != m_Models.end())
	{
		return it->second;
	}

	std::ifstream ifs(path, std::ios::binary);

	if (!ifs)
	{
		throw std::runtime_error("ERROR : Failed to open model file : " + WStringToString(path));
	}

	// Magic Header
	char header[4];
	ifs.read(header, 4);
	if (strncmp(header, "MODL", 4) != 0)
	{
		throw std::runtime_error("Invalid model file: wrong magic header");
	}

	// Version
	uint32_t version = 0;
	ifs.read(reinterpret_cast<char*>(&version), sizeof(uint32_t));

	if (version != 1)
	{
		throw std::runtime_error("Unsupported model version: " + std::to_string(version));
	}

	ModelData model;
	model.ModelName = ReadString(ifs);

	uint32_t meshCount = 0;
	ifs.read(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));
	model.Meshes.resize(meshCount);

	for (uint32_t i = 0; i < meshCount; ++i)
	{
		MeshData& mesh = model.Meshes[i];

		mesh.MeshName = ReadString(ifs);

		ifs.read(reinterpret_cast<char*>(&mesh.IsSkinned), sizeof(bool));

		ifs.read(reinterpret_cast<char*>(&mesh.tx), sizeof(float) * 10); // tx~sz

		uint32_t vertexCount, indexCount, materialCount, subMeshCount, boneCount;
		ifs.read(reinterpret_cast<char*>(&vertexCount), sizeof(uint32_t));
		ifs.read(reinterpret_cast<char*>(&indexCount), sizeof(uint32_t));
		ifs.read(reinterpret_cast<char*>(&materialCount), sizeof(uint32_t));
		ifs.read(reinterpret_cast<char*>(&subMeshCount), sizeof(uint32_t));
		ifs.read(reinterpret_cast<char*>(&boneCount), sizeof(uint32_t));

		mesh.Vertices.resize(vertexCount);
		mesh.Indices.resize(indexCount);
		mesh.Materials.resize(materialCount);
		mesh.SubMeshes.resize(subMeshCount);
		mesh.Bones.resize(boneCount);
		if (mesh.IsSkinned)
		{
			mesh.SkinnedData.resize(vertexCount);
		}

		// Vertices, Indices
		ifs.read(reinterpret_cast<char*>(mesh.Vertices.data()), sizeof(VTX_MESH) * vertexCount);
		ifs.read(reinterpret_cast<char*>(mesh.Indices.data()), sizeof(uint32_t) * indexCount);

		// Materials
		for (auto& mat : mesh.Materials)
		{
			mat.MaterialName = ReadString(ifs);
			ifs.read(reinterpret_cast<char*>(&mat.MaterialIndex), sizeof(int32_t));
		}

		// SubMeshes
		for (auto& sub : mesh.SubMeshes)
		{
			sub.SubMeshName = ReadString(ifs);
			ifs.read(reinterpret_cast<char*>(&sub.IndexOffset), sizeof(int32_t));
			ifs.read(reinterpret_cast<char*>(&sub.IndexCount), sizeof(int32_t));
			ifs.read(reinterpret_cast<char*>(&sub.MaterialIndex), sizeof(int32_t));
		}

		// SkinnedData
		if (mesh.IsSkinned)
		{
			ifs.read(reinterpret_cast<char*>(mesh.SkinnedData.data()), sizeof(SkinnedData) * vertexCount);

			// VIBuffer
			auto viBuffer = std::make_shared<VIBuffer>();
			std::vector<VTX_SKINNED_MESH> vertices;
			vertices.reserve(mesh.Vertices.size());
			int vtxIdx = 0;

			for (auto& skinnedData : mesh.SkinnedData)
			{
				VTX_SKINNED_MESH vtx;

				vtx.Position = mesh.Vertices[vtxIdx].Position;
				vtx.Normal = mesh.Vertices[vtxIdx].Normal;
				vtx.TexCoord0 = mesh.Vertices[vtxIdx].TexCoord0;
				vtx.Tangent = mesh.Vertices[vtxIdx].Tangent;
				memcpy(&vtx.BoneIndices, skinnedData.BoneIndices, sizeof(skinnedData.BoneIndices));
				memcpy(&vtx.BoneWeight, skinnedData.BoneWeight, sizeof(skinnedData.BoneWeight));

				vertices.push_back(vtx);
				++vtxIdx;
			}

			viBuffer->NumVertexBuffers = 1;
			viBuffer->VertexStride = sizeof(engine::VTX_SKINNED_MESH);
			viBuffer->NumVertices = vertices.size();
			viBuffer->IndexStride = 4;
			viBuffer->NumIndices = mesh.Indices.size();
			viBuffer->IndexFormat = DXGI_FORMAT_R32_UINT;
			viBuffer->PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

			D3D11_BUFFER_DESC vbDesc = {};
			vbDesc.ByteWidth = viBuffer->VertexStride * viBuffer->NumVertices;
			vbDesc.Usage = D3D11_USAGE_DEFAULT;
			vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vbDesc.StructureByteStride = viBuffer->VertexStride;
			vbDesc.CPUAccessFlags = 0;
			vbDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA vbData = {};
			vbData.pSysMem = vertices.data();

			if (FAILED(engine::D3D11Manager::GetInstance().GetDevice()->CreateBuffer(&vbDesc, &vbData, viBuffer->VertexBuffer.ReleaseAndGetAddressOf())))
			{
				std::cerr << "ERROR : Failed to create Mesh VBuffer ! \n";
			}

			D3D11_BUFFER_DESC ibDesc = {};
			ibDesc.ByteWidth = viBuffer->IndexStride * viBuffer->NumIndices;
			ibDesc.Usage = D3D11_USAGE_DEFAULT;
			ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			ibDesc.StructureByteStride = viBuffer->IndexStride;
			ibDesc.CPUAccessFlags = 0;
			ibDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA ibData = {};
			ibData.pSysMem = mesh.Indices.data();

			if (FAILED(engine::D3D11Manager::GetInstance().GetDevice()->CreateBuffer(&ibDesc, &ibData, viBuffer->IndexBuffer.ReleaseAndGetAddressOf())))
			{
				std::cerr << "ERROR : Failed to create mesh IBuffer ! \n";
			}

			mesh.VIBuffer = viBuffer;

			// Bones
			for (auto& bone : mesh.Bones)
			{
				bone.BoneName = ReadString(ifs);
				ifs.read(reinterpret_cast<char*>(&bone.Index), sizeof(int32_t));
				ifs.read(reinterpret_cast<char*>(&bone.parentIndex), sizeof(int32_t));

				ifs.read(reinterpret_cast<char*>(&bone.tx), sizeof(float) * 10); // tx~sz
				ifs.read(reinterpret_cast<char*>(bone.offsetMatrix), sizeof(float) * 16);
			}

			for (auto& bone : mesh.Bones)
			{
				mesh.BoneMap.emplace(bone.BoneName, bone.Index);
			}
		}

		else
		{
			// VIBuffer
			auto viBuffer = std::make_shared<VIBuffer>();

			viBuffer->NumVertexBuffers = 1;
			viBuffer->VertexStride = sizeof(engine::VTX_MESH);
			viBuffer->NumVertices = mesh.Vertices.size();
			viBuffer->IndexStride = 4;
			viBuffer->NumIndices = mesh.Indices.size();
			viBuffer->IndexFormat = DXGI_FORMAT_R32_UINT;
			viBuffer->PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

			D3D11_BUFFER_DESC vbDesc = {};
			vbDesc.ByteWidth = viBuffer->VertexStride * viBuffer->NumVertices;
			vbDesc.Usage = D3D11_USAGE_DEFAULT;
			vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vbDesc.StructureByteStride = viBuffer->VertexStride;
			vbDesc.CPUAccessFlags = 0;
			vbDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA vbData = {};
			vbData.pSysMem = mesh.Vertices.data();

			if (FAILED(engine::D3D11Manager::GetInstance().GetDevice()->CreateBuffer(&vbDesc, &vbData, viBuffer->VertexBuffer.ReleaseAndGetAddressOf())))
			{
				std::cerr << "ERROR : Failed to create Mesh VBuffer ! \n";
			}

			D3D11_BUFFER_DESC ibDesc = {};
			ibDesc.ByteWidth = viBuffer->IndexStride * viBuffer->NumIndices;
			ibDesc.Usage = D3D11_USAGE_DEFAULT;
			ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			ibDesc.StructureByteStride = viBuffer->IndexStride;
			ibDesc.CPUAccessFlags = 0;
			ibDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA ibData = {};
			ibData.pSysMem = mesh.Indices.data();

			if (FAILED(engine::D3D11Manager::GetInstance().GetDevice()->CreateBuffer(&ibDesc, &ibData, viBuffer->IndexBuffer.ReleaseAndGetAddressOf())))
			{
				std::cerr << "ERROR : Failed to create mesh IBuffer ! \n";
			}

			mesh.VIBuffer = viBuffer;
		}
	}

	m_Models.emplace(path, model);

	return model;
}

engine::AnimationClip engine::LoadManager::ReadAnimationClipDataFromFile(const std::wstring& path)
{
	auto it = m_AnimationClips.find(path);

	if (it != m_AnimationClips.end())
	{
		return it->second;
	}

	std::ifstream ifs(path, std::ios::binary);
	if (!ifs)
	{
		throw std::runtime_error("Failed to open animation file");
	}

	char header[4];
	ifs.read(header, 4);
	if (strncmp(header, "ANIM", 4) != 0)
	{
		throw std::runtime_error("Invalid animation file header");
	}

	uint32_t version = 0;
	ifs.read(reinterpret_cast<char*>(&version), sizeof(uint32_t));

	if (version > 2)
	{
		throw std::runtime_error("Unsupported animation file version");
	}

	AnimationClip clip;
	clip.Name = ReadString(ifs);
	clip.Path = path;

	ifs.read(reinterpret_cast<char*>(&clip.Duration), sizeof(double));

	uint32_t trackCount = 0;
	ifs.read(reinterpret_cast<char*>(&trackCount), sizeof(uint32_t));
	clip.Tracks.resize(trackCount);

	for (uint32_t i = 0; i < trackCount; ++i)
	{
		BoneKeyFrames& track = clip.Tracks[i];
		ifs.read(reinterpret_cast<char*>(&track.BoneIndex), sizeof(int32_t));

		uint32_t keyframeCount = 0;
		ifs.read(reinterpret_cast<char*>(&keyframeCount), sizeof(uint32_t));
		track.Frames.resize(keyframeCount);

		for (uint32_t j = 0; j < keyframeCount; ++j)
		{
			Keyframe& kf = track.Frames[j];
			ifs.read(reinterpret_cast<char*>(&kf.Time), sizeof(double));
			ifs.read(reinterpret_cast<char*>(&kf.Translation.Value), sizeof(float) * 3);
			ifs.read(reinterpret_cast<char*>(&kf.Rotation.Value), sizeof(float) * 4);
			ifs.read(reinterpret_cast<char*>(&kf.Scale.Value), sizeof(float) * 3);
		}
	}

	// ver 2.
	if (version > 1)
	{
		uint32_t eventCount = 0;
		ifs.read(reinterpret_cast<char*>(&eventCount), sizeof(uint32_t));

		clip.Events.resize(eventCount);

		for (uint32_t i = 0; i < eventCount; ++i)
		{
			AnimationEvent& event = clip.Events[i];
			ifs.read(reinterpret_cast<char*>(&event.Time), sizeof(float));
			event.EventName = ReadString(ifs);
		}
	}

	m_AnimationClips.emplace(path, clip);

	return clip;
}

void engine::LoadManager::BuildTriangleAABBs(const std::vector<_float3>& vertices, const std::vector<uint32_t>& indices,
	std::vector<TriangleAABB>& out)
{
	size_t triCnt = indices.size() / 3;
	out.resize(triCnt);

	for (size_t t = 0; t < triCnt; ++t)
	{
		uint32_t i0 = indices[t * 3 + 0];
		uint32_t i1 = indices[t * 3 + 1];
		uint32_t i2 = indices[t * 3 + 2];

		const _float3& v0 = vertices[i0];
		const _float3& v1 = vertices[i1];
		const _float3& v2 = vertices[i2];

		AABB box;
		box.Min = Vector3{ (std::min)({ v0.x, v1.x, v2.x }),
					(std::min)({ v0.y, v1.y, v2.y }),
					(std::min)({ v0.z, v1.z, v2.z }) };

		box.Max = Vector3{ (std::max)({ v0.x, v1.x, v2.x }),
					(std::max)({ v0.y, v1.y, v2.y }),
					(std::max)({ v0.z, v1.z, v2.z }) };

		out[t] = { box,
				   { (v0.x + v1.x + v2.x) * (1.0f / 3.0f),
					 (v0.y + v1.y + v2.y) * (1.0f / 3.0f),
					 (v0.z + v1.z + v2.z) * (1.0f / 3.0f) } };
	}
}

void engine::LoadManager::Release()
{
	m_Models.clear();
	m_AnimationClips.clear();
}

IMPLEMENT_SINGLETON(engine::LoadManager)
