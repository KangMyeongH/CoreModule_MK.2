#pragma once
#include <condition_variable>

#include "core_defines.h"

namespace engine
{
    class GameObject;
    using GameObjects 		= std::list<SharedPtr<GameObject>>;
    using GameObjectsTagMap = std::unordered_map<std::string, std::unordered_set<SharedPtr<GameObject>>>;

    class COREMODULE_API Scene
    {
    private:
        //======================================//
        //				constructor				//
        //======================================//

        Scene();
        ~Scene();
    public:
        DECLARE_SINGLETON(Scene)

        //======================================//
        //				  method				//
        //======================================//

        GameObjects* GetGameObjects();

        _bool Initialize(const _wstring& path);
        void ChangeScene(const _wstring& sceneName);

    	SharedPtr<GameObject> Find(const _string& name);
        std::vector<SharedPtr<GameObject>> FindGameObjectsWithTag(const _string& tag);
        SharedPtr<GameObject> FindWithTag(const _string& tag);

        void FlushDestroyGameObjects();
        void RegisterNextScene();

    	void Release();

    private:
        void setupTransformHierarchy() const;
        void updateGameObjectTag(const SharedPtr<GameObject>& obj, const _string& newTag);
    	void registerGameObject(const SharedPtr<GameObject>& gameObject);
        void loadSceneInBackGround(const std::wstring& nextScene);

        _bool loadSceneData(const _wstring& path);

        //======================================//
        //				 serialize				//
        //======================================//
    public:
        nlohmann::ordered_json 	To_Json() const;
        void 					From_Json(const nlohmann::ordered_json& j);

    private:
        //======================================//
        //				  fields				//
        //======================================//

        GameObjects 		m_GameObjects;
        GameObjectsTagMap  	m_GameObjectsTagMap;
        _string             m_SceneName;
        _wstring            m_NextScene;

        std::atomic<bool> 			m_bSceneLoaded{ false };
        std::condition_variable 	m_CV;
        std::mutex 					m_LoadingMutex;
        std::thread 				m_LoadingThread;

        friend class GameObject;
    };
}
