#pragma once
#include "core_defines.h"

namespace engine
{
    class GameObject;

    namespace editor
    {
        class COREMODULE_API Hierarchy
        {
        //======================================//
        //				constructor				//
        //======================================//
        private:
            Hierarchy();
            ~Hierarchy();
        public:
            DECLARE_SINGLETON(Hierarchy)

        //======================================//
        //				 property				//
        //======================================//
        public:
            void        SetCurrentSceneName(const _string& name);
            _string 	GetCurrentSceneName() const;

            std::list<SharedPtr<GameObject>>* GetGameObjectList() { return &m_GameObjects; }

        //======================================//
        //				  method				//
        //======================================//
        public:
        	void 		AddGameObject();
            void 		AddGameObject(const SharedPtr<GameObject>& gameObject);
            void        RemoveGameObject(const SharedPtr<GameObject>& gameObject);
            void 		FlushDestroyGameObject();

            void        Release();

        //======================================//
        //				 serialize				//
        //======================================//
        public:
            nlohmann::ordered_json ToJson() const;
            void FromJson(const nlohmann::ordered_json& j);

		//======================================//
        //				  fields				//
        //======================================//
        private:
            std::list<SharedPtr<GameObject>> m_GameObjects;
            _string m_CurrentSceneName;
        };
    }

}