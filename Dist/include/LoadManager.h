#pragma once
#include "core_defines.h"

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
    public:
        _bool LoadProject(const std::string& path);

        _bool CreateNewScene(const _string& sceneName, const _wstring& path);
        _bool LoadSceneData(const _wstring& path);
        _bool SaveSceneData();

    	_bool LoadMaterialData(const SharedPtr<Material>& material, const _wstring& path);
        _bool SaveMaterialData(const SharedPtr<Material>& material, const _wstring& path);

        void LoadHierarchyToScene();

        _wstring OpenFileDialog();
        _wstring BrowseFolderDialog();

        //======================================//
        //				  method				//
        //======================================//

        //======================================//
        //				 serialize				//
        //======================================//

        //======================================//
        //				  fields				//
        //======================================//
    };
}
