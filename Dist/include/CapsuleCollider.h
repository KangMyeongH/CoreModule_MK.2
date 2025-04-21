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

        // Debug Render
        SharedPtr<Material> m_Material;
        ComPtr<ID3D11Buffer> m_VertexBuffer;
        std::vector<DebugVertex> m_Batch;
    };
}
