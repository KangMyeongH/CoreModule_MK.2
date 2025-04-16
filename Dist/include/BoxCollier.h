#pragma once
#include "Collider.h"

/// <summary>
/// 1. AABB으로 먼저 OBB충돌 검사를 해줄 객체들을 고른다.
/// 
/// 2. OBB를 구한다.
///
///	3. OBB OBB 충돌 검사 및, OBB Sphere 충돌 검사
/// </summary>

namespace engine
{
    class COREMODULE_API BoxCollier final : public Collider
    {
        DECLARE_REGISTER_COMPONENT(BoxCollier)
        //======================================//
        //				constructor				//
        //======================================//
    protected:
        explicit BoxCollier(const SharedPtr<GameObject>& owner, const _string& name = "BoxCollider");
        ~BoxCollier() override = default;
        BoxCollier(const BoxCollier& rhs);


        //======================================//
        //				 property				//
        //======================================//
    public:
    	ColliderType GetColliderType() const override { return ColliderType_Box; }

        //======================================//
        //				  method				//
        //======================================//
    public:
        void 	UpdateCollider() override;

    	void 	Render(ComPtr<ID3D11DeviceContext> context) override;
        void 	Destroy() override;

    private:
        OBB 	calcWorldOBB();
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
        OBB     m_OBB;
        Vector3 m_Center;
        Vector3 m_Size;
    };
}
