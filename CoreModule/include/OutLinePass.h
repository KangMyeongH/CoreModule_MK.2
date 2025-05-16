#pragma once
#include "RenderPass.h"

namespace engine
{
    class COREMODULE_API OutLinePass : public RenderPass
    {
	    //======================================//
        //				constructor				//
        //======================================//
    public:
        ~OutLinePass() override = default;

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
        SharedPtr<Shader> m_Shader;
        ComPtr<ID3D11BlendState> m_BlendState;
        ComPtr<ID3D11DepthStencilState> m_DepthStencilState;

        _float2 m_InvScreen = { 0.f, 0.f };

    };
}
