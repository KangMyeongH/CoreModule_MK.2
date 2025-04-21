#pragma once
#include "Behaviour.h"

namespace engine
{
    class COREMODULE_API Collider : public Behaviour
    {
        //======================================//
        //				constructor				//
        //======================================//
    protected:
        explicit Collider(const SharedPtr<GameObject>& owner, const _string& name = "Collider");
        ~Collider() override = default;
        Collider(const Collider& rhs);

        //======================================//
        //				 property				//
        //======================================//
    public:
        virtual ColliderType 	GetColliderType() const = 0;

    	_bool       			IsTrigger() const { return m_bTrigger;}
        void        			SetTrigger(const _bool isTrigger) { m_bTrigger = isTrigger; }
        AABB            		GetWorldAABB() const { return m_AABB; }

        //======================================//
        //				  method				//
        //======================================//
    public:
        // 와이어 프레임 디버깅용 Render
        virtual void UpdateCollider() = 0;
    	virtual void Render(ComPtr<ID3D11DeviceContext> context, const SharedPtr<VIBuffer>& buffer) = 0;

        void Destroy() override = 0;
    	SharedPtr<Component> Clone() const override = 0;

    protected:
        virtual void calcWorldABB() = 0;

        void registerComponent(ApplicationMode mode = CLIENT) override;

        //======================================//
        //				 serialize				//
        //======================================//
    public:
        void to_json(nlohmann::ordered_json& j) override = 0;
        void from_json(const nlohmann::ordered_json& j) override = 0;

        //======================================//
        //				  fields				//
        //======================================//
    protected:
        _bool           m_bTrigger;
        _bool           m_bHit;
        // BroadPhase일 때 사용할 Bounding Box
        AABB            m_AABB;
    };
}
