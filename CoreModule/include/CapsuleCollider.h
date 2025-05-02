#pragma once
#include "Collider.h"

namespace engine
{
    class COREMODULE_API CapsuleCollider final : public Collider
    {
        DECLARE_REGISTER_COMPONENT(CapsuleCollider)
        //======================================//
        //				constructor				//
        //======================================//
    protected:
        explicit CapsuleCollider(const SharedPtr<GameObject>& owner, const _string& name = "CapsuleCollider");
        ~CapsuleCollider() override = default;
        CapsuleCollider(const CapsuleCollider& rhs);

        //======================================//
        //				 property				//
        //======================================//
    public:
        ColliderType GetColliderType() const override { return ColliderType_Capsule; }

        Vector3 GetCenter() const { return m_Center; }
        void SetCenter(const Vector3& center) { m_Center = center; }

        _float GetRadius() const { return m_Radius; }
        void SetRadius(const _float radius) { m_Radius = radius; }

        _float GetHeight() const { return m_Height; }
        void SetHeight(const _float height) { m_Height = height; }

        _int GetDir() const { return m_Dir; }
        void SetDir(const _int dir) { m_Dir = dir; }

        Capsule GetCapsule() const { return m_Capsule; }

        //======================================//
        //				  method				//
        //======================================//
    public:
        void 	UpdateCollider() override;

        void 	Render(ComPtr<ID3D11DeviceContext> context, const SharedPtr<VIBuffer>& buffer) override;
        void 	Destroy() override;

    private:
        Capsule calcWorldCapsule();
        void 	calcWorldABB() override;

        //======================================//
        //				 serialize				//
        //======================================//
    public:
        void to_json(nlohmann::ordered_json& j) override;
        void from_json(const nlohmann::ordered_json& j) override;

        //======================================//
        //				  fields				//
        //======================================//
    private:
        Capsule m_Capsule;
        Vector3 m_Center;
        _float  m_Radius;
        _float  m_Height;
        _int    m_Dir;
    };
}
