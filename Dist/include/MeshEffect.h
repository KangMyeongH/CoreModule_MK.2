#pragma once
#include "Renderer.h"
#include "Material.h"

namespace engine
{
	class Mesh;

	class COREMODULE_API MeshEffect : public Renderer
    {
        //======================================//
        //				constructor				//
        //======================================//
    protected:
        explicit MeshEffect(const SharedPtr<GameObject>& owner, const _string& name = "MeshEffect")
	        : Renderer(owner, name), m_MeshIdx(0), m_bBillboard(false)
        {
        }

        ~MeshEffect() override = default;
        MeshEffect(const MeshEffect& rhs)
	        : Renderer(rhs), m_ModelPath(rhs.m_ModelPath), m_MeshIdx(rhs.m_MeshIdx), m_bBillboard(rhs.m_bBillboard)
        {

        }

        //======================================//
        //				 property				//
        //======================================//
    public:
        SharedPtr<Mesh> GetMesh() { return m_Mesh; }
        void SetMesh(const SharedPtr<Mesh>& mesh) { m_Mesh = mesh; }
        void SetMesh(const _wstring& modelPath, _int meshIdx);

        //======================================//
        //				  method				//
        //======================================//
	public:
        void InputAssembler(ID3D11DeviceContext* context) override;
        void Bind(ID3D11DeviceContext* context) override;
        void Render(ID3D11DeviceContext* context) override;
        void PreRender(ID3D11DeviceContext* context, const _float4X4& viewMat, const _float4X4& projMat) override;


        void Destroy() override;

        SharedPtr<Component> Clone() const override
        {
            SharedPtr<MeshEffect> clone(CLONE_SHARED_PTR(MeshEffect));

            for (auto& pair : m_Material)
            {
                clone->m_Material.emplace(pair.first, pair.second->Clone(clone));
            }

            return clone;
        }

	protected:
        void registerComponent(ApplicationMode mode) override;
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
        SharedPtr<Mesh> m_Mesh;
        _wstring        m_ModelPath;
        _int            m_MeshIdx;
        _bool           m_bBillboard;
    };
}
