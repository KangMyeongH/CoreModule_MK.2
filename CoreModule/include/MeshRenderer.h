#pragma once
#include "core_defines.h"
#include "Material.h"
#include "Renderer.h"

namespace engine
{
	class Mesh;
}

namespace engine
{
    class COREMODULE_API MeshRenderer : public Renderer
    {
	    //DECLARE_REGISTER_COMPONENT(MeshRenderer)
	    //======================================//
        //				constructor				//
        //======================================//
    protected:
        explicit MeshRenderer(const SharedPtr<GameObject>& owner, const _string& name = "MeshRenderer");
        ~MeshRenderer() override;
        MeshRenderer(const MeshRenderer& rhs);

        //======================================//
        //				 property				//
        //======================================//
    public:
        SharedPtr<Mesh> GetMesh() { return m_Mesh; }
        void SetMesh(const SharedPtr<Mesh>& mesh) { m_Mesh = mesh; }

        //======================================//
        //				  method				//
        //======================================//
    public:
        void Bind(const ComPtr<ID3D11DeviceContext>& context) override;
        void Render(const ComPtr<ID3D11DeviceContext>& context) override;

        void Destroy() override;

        SharedPtr<Component> Clone() const override
        {
            SharedPtr<MeshRenderer> clone(CLONE_SHARED_PTR(MeshRenderer));

            for (auto& pair : m_Material)
            {
                clone->m_Material.emplace(pair.first, pair.second->Clone(clone));
            }

            return clone;
        }

    protected:
        void registerComponent(ApplicationMode mode = CLIENT) override;

    private:
        static SharedPtr<MeshRenderer> create()
        {
            return SharedPtr<MeshRenderer>(new MeshRenderer(nullptr), []
            (const MeshRenderer* ptr) {delete ptr; });
        }

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

        static ComponentRegistrar registrar_MeshRenderer;
    };
}
