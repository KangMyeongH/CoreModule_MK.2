#pragma once
#include "Component.h"

namespace engine
{
    class COREMODULE_API Rigidbody : public Component
    {
        friend class PhysicsManager;
    private:
        //======================================//
        //				constructor				//
        //======================================//

        explicit Rigidbody(const SharedPtr<GameObject>& owner, const _string& name = "Rigidbody");
        ~Rigidbody() override = default;

    public:
        //======================================//
        //				 property				//
        //======================================//

        void SetVelocity(const Vector3& velocity) { m_Velocity = velocity; }
        Vector3 GetVelocity() const { return m_Velocity; }
        Vector3& Velocity() { return m_Velocity; }

        void SetMass(const _float mass) { m_Mass = mass; }
        _float GetMass() const { return m_Mass; }

        void SetDrag(const _float drag) { m_Drag = drag; }
        _float GetDrag() const { return m_Drag; }

        void SetUseGravity(const _bool useGravity) { m_UseGravity = useGravity; }
        _bool IsUseGravity() const { return m_UseGravity; }

        void SetIsKinematic(const _bool isKinematic) { m_IsKinematic = isKinematic; }
        _bool IsKinematic() const { return m_IsKinematic; }

        //======================================//
        //				 method					//
        //======================================//

        void AddForce(const Vector3& force);

        void Destroy() override;

    protected:
        void registerComponent() override;

    private:
        void rigidbodyUpdate(float deltaTime);

        //======================================//
        //				 serialize				//
        //======================================//

        void to_json(nlohmann::ordered_json& j) override;
        void from_json(const nlohmann::ordered_json& j) override;

    private:
        //======================================//
        //				  fields				//
        //======================================//

        Vector3  m_Velocity;
        _float   m_Mass;
        _float   m_Drag;
        _bool    m_UseGravity;
        _bool    m_IsKinematic;

        DECLARE_REGISTER_COMPONENT(Rigidbody)
    };
}
