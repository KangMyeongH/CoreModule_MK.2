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

        void BuildTriangleAABBs(const std::vector<_float3>& vertices, const std::vector<uint32_t>& indices, std::vector<TriangleAABB>& out);


        void Release();
        //======================================//
        //				 serialize				//
        //======================================//

        //======================================//
        //				  fields				//
        //======================================//
    private:
        std::unordered_map<_wstring, ModelData> m_Models;
        std::unordered_map<_wstring, AnimationClip> m_AnimationClips;
    };
}
