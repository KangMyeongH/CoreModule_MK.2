#pragma once
#include "Component.h"

namespace engine
{
    class Material;

    class COREMODULE_API Renderer: public Component
    {
    protected:
        //======================================//
        //				constructor				//
        //======================================//

        explicit Renderer(const SharedPtr<GameObject>& owner, const _string& name = "Renderer");
        ~Renderer() override = default;

    public:
        //======================================//
        //				 property				//
        //======================================//

        void SetMaterial(const SharedPtr<Material>& material) { m_Material = material; }
    	SharedPtr<Material> GetMaterial() { return m_Material; }

        //======================================//
        //				  method				//
        //======================================//

        virtual void Bind(const ComPtr<ID3D11DeviceContext>& context) = 0;
        virtual void Render(const ComPtr<ID3D11DeviceContext>& context) = 0;

        void Destroy() override = 0;
        SharedPtr<Component> Clone() const override = 0;

    protected:
        void registerComponent() final;

        //======================================//
        //				 serialize				//
        //======================================//

        void to_json(nlohmann::ordered_json& j) override = 0;
        void from_json(const nlohmann::ordered_json& j) override = 0;

        //======================================//
        //				  fields				//
        //======================================//

        SharedPtr<Material> m_Material;
    };
}
