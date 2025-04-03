#pragma once
#include "core_defines.h"

namespace engine
{
	class Prefab;
}

namespace engine
{
    class COREMODULE_API LoadManager
    {
        //======================================//
        //				constructor				//
        //======================================//
    private:
        LoadManager();
        ~LoadManager();
    public:
        DECLARE_SINGLETON(LoadManager)
        //======================================//
        //				 property				//
        //======================================//

        //======================================//
        //				  method				//
        //======================================//
    public:
        _bool LoadProject(const std::string& path);

        _bool CreateNewScene(const _string& sceneName, const _wstring& path);
        _bool LoadSceneData(const _wstring& path);
        _bool SaveSceneData();

        _bool LoadMaterialData(const SharedPtr<Material>& material, const _wstring& path);
        _bool SaveMaterialData(const SharedPtr<Material>& material, const _wstring& path);
        
        _bool LoadPrefab(Prefab& prefab, const _wstring& path);
        _bool SavePrefab(Prefab& prefab, const _wstring& path);

        void LoadHierarchyToScene();

        _wstring OpenFileDialog();
        _wstring BrowseFolderDialog();

        void WriteString(std::ofstream& ofs, const std::string& str);
        void WriteModelDataToFile(const ModelData& model, const std::wstring& path);
        void WriteAnimationClipDataToFile(const AnimationClip& clip, const std::wstring& path);

        std::string ReadString(std::ifstream& ifs);
        ModelData ReadModelDataFromFile(const std::wstring& path);
        AnimationClip ReadAnimationClipDataFromFile(const std::wstring& path);

     //   _bool SaveStaticMesh(std::ofstream& ofs, const MeshData& meshData);
     //   _bool SaveMaterial(std::ofstream& ofs, const std::vector<MaterialData>& material);
    	//_bool SaveSubMesh(std::ofstream& ofs, const std::vector<SubMeshData>& subMesh);

     //   _bool LoadStaticMesh(std::ifstream& ifs, ApplicationMode mode = CLIENT);
     //   _bool LoadMaterial(std::ifstream& ifs, ApplicationMode mode);
     //   _bool LoadSubMesh(std::ifstream& ifs, ApplicationMode mode);

        //======================================//
        //				 serialize				//
        //======================================//

        //======================================//
        //				  fields				//
        //======================================//
    };
}
