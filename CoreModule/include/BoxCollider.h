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
    class COREMODULE_API BoxCollider final : public Collider
    {
        DECLARE_REGISTER_COMPONENT(BoxCollider)
        //======================================//
        //				constructor				//
        //======================================//
    protected:
        explicit BoxCollider(const SharedPtr<GameObject>& owner, const _string& name = "BoxCollider");
        ~BoxCollider() override = default;
        BoxCollider(const BoxCollider& rhs);

        //======================================//
        //				 property				//
        //======================================//
    public:
    	ColliderType GetColliderType() const override { return ColliderType_Box; }

        Vector3 GetCenter() const { return m_Center; }
        void SetCenter(const Vector3& center) { m_Center = center; }

        Vector3 GetSize() const { return m_Size; }
        void SetSize(const Vector3& size) { m_Size = size; }

        OBB GetOBB() const { return m_OBB; }

        //======================================//
        //				  method				//
        //======================================//
    public:
        void 	UpdateCollider() override;

    	void 	Render(ComPtr<ID3D11DeviceContext> context, const SharedPtr<VIBuffer>& buffer) override;
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
