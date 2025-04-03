#pragma once
#include "core_defines.h"
#include "Scene.h"

namespace engine
{
	class GameObject;
}

namespace engine
{
    class COREMODULE_API Prefab
    {
        //======================================//
        //				constructor				//
        //======================================//
    public:
        Prefab();
        explicit Prefab(const SharedPtr<GameObject>& root);
        Prefab(const Prefab& rhs);
        Prefab& operator=(const Prefab& rhs);

        //======================================//
        //				 property				//
        //======================================//
        SharedPtr<GameObject> GetRoot() const { return m_TemplateRoot; }

        //======================================//
        //				  method				//
        //======================================//

        //======================================//
        //				 serialize				//
        //======================================//
        void to_json(nlohmann::ordered_json& j);
        void from_json(const nlohmann::ordered_json& j);

        //======================================//
        //				  fields				//
        //======================================//
    private:
        SharedPtr<GameObject> 	m_TemplateRoot;
    };
}
