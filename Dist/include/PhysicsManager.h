#pragma once
#include "core_defines.h"

namespace engine
{
    class Rigidbody;

    using Rigidbodies = std::vector<SharedPtr<Rigidbody>>;
    using RigidbodyList = std::list<SharedPtr<Rigidbody>>;

    class COREMODULE_API PhysicsManager
	{
    private:
        //======================================//
        //				constructor				//
        //======================================//

        PhysicsManager() = default;
        ~PhysicsManager();
    public:
        DECLARE_SINGLETON(PhysicsManager)
        //======================================//
        //				  method				//
        //======================================//

        void PhysicsUpdate(float deltaTime) const;
        void AddRigidbody(const SharedPtr<Rigidbody>& rigidbody);

        void RegisterRigidbody();
        void FlushDestroyRigidbody();

        void Release();

    private:
        //======================================//
        //				  fields				//
        //======================================//

        Rigidbodies     m_Rigidbodies;
        RigidbodyList   m_RegisterQueue;
    };
}
