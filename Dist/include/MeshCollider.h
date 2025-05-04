#pragma once
#include "Collider.h"

/// <summary>
/// MeshCollider는 정적 오브젝트에만 사용되는 것을 전제하에 구현되어 있습니다.
///
/// **** 반드시 GameObject의 m_bStatic을 true로 하고 사용하세요. 연산량 감당 안됨. ****
/// 관련 최적화 끝냄.
/// </summary>

namespace engine
{
    class COREMODULE_API MeshCollider : public Collider
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

        void SetMesh(const _wstring& modelPath, _int meshIdx);
        MeshData GetMesh() const { return m_Mesh; }

        _wstring GetPath()const { return m_Path; }

        _int GetMeshIdx() const { return m_MeshIdx; }

        SharedPtr<VIBuffer> GetVIBuffer() const { return m_DebugVIBuffer; }

        //======================================//
        //				  method				//
        //======================================//
    public:
        void UpdateCollider() override;
        void Render(ComPtr<ID3D11DeviceContext> context, const SharedPtr<VIBuffer>& buffer) override;
        void Destroy() override;

    private:
        void calcWorldABB() override; // 얘도 따로 쓸거같은데. 아니면 제일 큰 AABB 즉, Lead BVH로 하던가.

        //======================================//
        //				 serialize				//
        //======================================//
    public:
        void to_json(nlohmann::ordered_json& j) override
        {
            _string type = "MeshCollider";
            j = nlohmann::ordered_json{
				{"type", type},
				{"enable", m_bEnabled},
				{"isTrigger", m_bTrigger},
				{"path", m_Path},
				{"meshIdx", m_MeshIdx}
            };
        }
        void from_json(const nlohmann::ordered_json& j) override
        {
	        if (j.contains("enable"))
	        {
                j.at("enable").get_to(m_bEnabled);
	        }

            if (j.contains("isTrigger"))
            {
                j.at("isTrigger").get_to(m_bTrigger);
            }

            if (j.contains("path"))
            {
                j.at("path").get_to(m_Path);
            }

            if (j.contains("meshIdx"))
            {
                j.at("meshIdx").get_to(m_MeshIdx);
            }

            if (m_MeshIdx > -1 && !m_Path.empty())
            {
                SetMesh(m_Path, m_MeshIdx);
            }
        }

        //======================================//
        //				  fields				//
        //======================================//
    private:
        // BVH m_BVH;
        MeshData    				m_Mesh;
        SharedPtr<VIBuffer>         m_DebugVIBuffer;

        _wstring 	m_Path;
        _int    	m_MeshIdx;
        _bool       m_bDirty;
    };
}
