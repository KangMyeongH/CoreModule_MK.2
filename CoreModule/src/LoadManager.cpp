#include "LoadManager.h"
#include <ShObjIdl.h>
#include "Hierarchy.h"
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

	std::wstring basePath = L"../Client/Assets/Scenes/";
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

IMPLEMENT_SINGLETON(engine::LoadManager)
