#pragma once
#include "Collider.h"

/// <summary>
/// MeshCollider는 정적 오브젝트에만 사용되는 것을 전제하에 구현되어 있습니다.
///
/// **** 반드시 GameObject의 m_bStatic을 true로 하고 사용하세요. 연산량 감당 안됨. ****
/// </summary>

namespace engine
{
    class MeshCollider : public Collider
    {
        DECLARE_REGISTER_COMPONENT(MeshCollider)
        //======================================//
        //				constructor				//
        //======================================//
    protected:
        explicit MeshCollider(const SharedPtr<GameObject>& owner, const _string& name = "MeshCollider");
        ~MeshCollider() override = default;
        MeshCollider(const MeshCollider& rhs);

        //======================================//
        //				 property				//
        //======================================//
    public:
        ColliderType GetColliderType() const override { return ColliderType_Mesh; }

        //======================================//
        //				  method				//
        //======================================//
    public:
        void UpdateCollider() override;
        void Render(ComPtr<ID3D11DeviceContext> context) override;
        void Destroy() override;

    private:


        //======================================//
        //				 serialize				//
        //======================================//

        //======================================//
        //				  fields				//
        //======================================//
    };
}
