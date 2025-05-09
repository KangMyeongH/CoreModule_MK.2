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
	        : Renderer(owner, name)
        {
	        
        }
        ~MeshEffect() override = default;
        MeshEffect(const MeshEffect& rhs) = default;

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
        void Bind(const ComPtr<ID3D11DeviceContext>& context) override;
        void Render(const ComPtr<ID3D11DeviceContext>& context) override;

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

        //======================================//
        //				  fields				//
        //======================================//
	private:
        SharedPtr<Mesh> m_Mesh;
        _bool           m_bBillboard;
    };
}
