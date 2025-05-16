#pragma once
#include "RenderPass.h"

namespace engine
{
    class COREMODULE_API LightingPass : public RenderPass
    {
        //======================================//
        //				constructor				//
        //======================================//
    public:
        ~LightingPass() override = default;

        //======================================//
        //				  method				//
        //======================================//
    public:
        HRESULT Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
        HRESULT Render(ID3D11DeviceContext* context, void* data) override;
        void Release() override;

        //======================================//
        //				  fields				//
        //======================================//
    private:
        SharedPtr<Shader> m_DirLight;
        SharedPtr<Shader> m_PointLight;
        ComPtr<ID3D11DepthStencilState> m_LightDSState;
        ComPtr<ID3D11BlendState> m_BlendState;
    };
}
