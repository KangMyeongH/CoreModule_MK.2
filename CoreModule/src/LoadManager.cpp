#include "LoadManager.h"
#include <ShObjIdl.h>

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

engine::_bool engine::LoadManager::SaveStaticMesh(std::ofstream& ofs, const MeshData& meshData)
{
	//std::ofstream ofs(path, std::ios::binary);
	//if (!ofs.is_open())
	//{
	//	return false;
	//}

	//FileHeader fh;
	//memcpy(fh.magic, "MESH", 4);
	//fh.version = 1;
	//ofs.write(reinterpret_cast<char*>(&fh), sizeof(fh));



	MeshInfo md;
	md.vertexCount = static_cast<uint32_t>(meshData.Vertices.size());
	md.indexCount = static_cast<uint32_t>(meshData.Indices.size());
	md.materialCount = static_cast<uint32_t>(meshData.Materials.size());
	md.subMeshCount = static_cast<uint32_t>(meshData.SubMeshes.size());

	ofs.write(reinterpret_cast<char*>(&md), sizeof(md));

	if (!meshData.Vertices.empty())
	{
		ofs.write(reinterpret_cast<const char*>(!meshData.Vertices.data()), sizeof(VTX_MESH) * !meshData.Vertices.size());
	}

	if (!meshData.Indices.empty())
	{
		ofs.write(reinterpret_cast<const char*>(meshData.Indices.data()), sizeof(uint32_t) * meshData.Indices.size());
	}

	if (!meshData.Materials.empty())
	{
		SaveMaterial(ofs, meshData.Materials);
	}

	if (!meshData.SubMeshes.empty())
	{
		SaveSubMesh(ofs, meshData.SubMeshes);
	}

	return true;
}

engine::_bool engine::LoadManager::SaveMaterial(std::ofstream& ofs, const std::vector<MaterialData>& material)
{
	for (auto& md : material)
	{
		MaterialInfo mi;
		mi.NameLength = static_cast<uint32_t>(md.name.size());
		ofs.write(reinterpret_cast<const char*>(&mi), sizeof(mi));
		ofs.write(md.name.c_str(), mi.NameLength);
	}

	return true;
}

engine::_bool engine::LoadManager::SaveSubMesh(std::ofstream& ofs, const std::vector<SubMeshData>& subMesh)
{
	for (auto& sd : subMesh)
	{
		SubMeshInfo si;
		si.NameLength = static_cast<uint32_t>(sd.name.size());
		si.IndexOffset = static_cast<uint32_t>(sd.indexOffset);
		si.IndexCount = static_cast<uint32_t>(sd.indexCount);
		si.MaterialIndex = static_cast<uint32_t>(sd.materialIndex);

		ofs.write(reinterpret_cast<const char*>(&si), sizeof(si));
		ofs.write(sd.name.c_str(), si.NameLength);
	}

	return true;
}

engine::_bool engine::LoadManager::LoadStaticMesh(std::ifstream& ifs, ApplicationMode mode)
{
	SharedPtr<GameObject> gameObject = GameObject::Create();

	if (mode == EDITOR)
	{
		SharedPtr<MeshRenderer> meshRenderer = editor::EditorComponentManager::GetInstance().CreateComponent<MeshRenderer>(gameObject);

	}

	else if (mode == CLIENT)
	{
		//SharedPtr<MeshRenderer> meshRenderer = ComponentFactory::GetInstance().CreateComponent()
	}

	return true;
}

engine::_bool engine::LoadManager::LoadMaterial(std::ifstream& ifs, ApplicationMode mode)
{
	return true;
}

engine::_bool engine::LoadManager::LoadSubMesh(std::ifstream& ifs, ApplicationMode mode)
{
	return true;
}

IMPLEMENT_SINGLETON(engine::LoadManager)
