#pragma once
#include "core_defines.h"


namespace engine
{
    class ScriptBehaviour;

	using ScriptBehaviours = std::vector<SharedPtr<ScriptBehaviour>>;
    using ScriptBehaviourList = std::list<SharedPtr<ScriptBehaviour>>;

    class ScriptBehaviourManager
    {
    private:
        //======================================//
        //				constructor				//
        //======================================//

        ScriptBehaviourManager() = default;
        ~ScriptBehaviourManager();

    public:
        DECLARE_SINGLETON(ScriptBehaviourManager)

    public:
        //======================================//
        //				 method					//
        //======================================//

        void FixedUpdate() const;
        void Update() const;
        void LateUpdate() const;

    	void AddScriptBehaviour(const SharedPtr<ScriptBehaviour>& scriptBehaviour);

        void RegisterScriptBehaviours();
        void FlushDestroyScriptBehaviours();

        void Release();

    private:
        //======================================//
        //				  fields				//
        //======================================//

        ScriptBehaviours 		m_ScriptBehaviours;
        ScriptBehaviourList 	m_RegisterQueue;
    };
}
