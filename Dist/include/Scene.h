#pragma once
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
        void Release();

    private:
        void setupTransformHierarchy() const;
        void updateGameObjectTag(const SharedPtr<GameObject>& obj, const _string& newTag);
    	void registerGameObject(const SharedPtr<GameObject>& gameObject);

        //======================================//
        //				 serialize				//
        //======================================//

        nlohmann::ordered_json 	To_Json() const;
        void 					From_Json(const nlohmann::ordered_json& j);

    private:
        //======================================//
        //				  fields				//
        //======================================//

        GameObjects 		m_GameObjects;
        GameObjectsTagMap  	m_GameObjectsTagMap;
        _string             m_SceneName;

        friend class GameObject;
    };
}
