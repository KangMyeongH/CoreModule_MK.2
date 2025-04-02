#pragma once
#include "core_defines.h"
#include "Scene.h"

namespace engine
{
	class GameObject;
	class Prefab;
}

namespace engine
{
    class COREMODULE_API PrefabManager
    {
        //======================================//
        //				constructor				//
        //======================================//
    private:
        PrefabManager();
        ~PrefabManager();
    public:
        DECLARE_SINGLETON(PrefabManager)


        //======================================//
        //				 property				//
        //======================================//
    public:
        Prefab& GetPrefab(const _wstring& path);

        //======================================//
        //				  method				//
        //======================================//
    public:
        Prefab& AddPrefab(const _wstring& path);
        void MakePrefab(const SharedPtr<GameObject>& gameObject, const _wstring& path);

        void AddTempGameObject(const SharedPtr<GameObject>& gameObject);

        void Release();

        //======================================//
        //				 serialize				//
        //======================================//

        //======================================//
        //				  fields				//
        //======================================//
    private:
        std::unordered_map<_wstring, Prefab> m_PrefabMap;
        GameObjects                          m_TempGameObjects;
    };
}
