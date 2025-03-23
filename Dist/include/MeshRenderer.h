#pragma once
#include "Renderer.h"

namespace engine
{
	class Mesh;
}

namespace engine
{
    class MeshRenderer : public Renderer
    {
	    DECLARE_REGISTER_COMPONENT(MeshRenderer)
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

        //======================================//
        //				  method				//
        //======================================//
    public:
        void Bind(const ComPtr<ID3D11DeviceContext>& context) override;
        void Render(const ComPtr<ID3D11DeviceContext>& context) override;

        void Destroy() override;

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
    };
}
