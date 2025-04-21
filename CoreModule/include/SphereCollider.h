#pragma once
#include "Collider.h"

namespace engine
{
    class COREMODULE_API SphereCollider final : public Collider
    {
        DECLARE_REGISTER_COMPONENT(SphereCollider)
        //======================================//
        //				constructor				//
        //======================================//
    protected:
        explicit SphereCollider(const SharedPtr<GameObject>& owner, const _string& name = "SphereCollider");
        ~SphereCollider() override = default;
        SphereCollider(const SphereCollider& rhs);

        //======================================//
        //				 property				//
        //======================================//
    public:
        ColliderType GetColliderType() const override { return ColliderType_Sphere; }

    	Vector3 	GetCenter() const { return m_Center; }
        void 		SetCenter(const Vector3& center) { m_Center = center; }

        _float 		GetRadius() const { return m_Radius; }
        void 		SetRadius(const _float radius) { m_Radius = radius; }

        //======================================//
        //				  method				//
        //======================================//
    public:
        void UpdateCollider() override;

        void Render(ComPtr<ID3D11DeviceContext> context, const SharedPtr<VIBuffer>& buffer) override;
        void Destroy() override;

    private:
        Sphere  calcWorldSphere();
        void    calcWorldABB() override;

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
        Sphere 		m_Sphere;
        Vector3 	m_Center;
        _float  	m_Radius;
    };
}
