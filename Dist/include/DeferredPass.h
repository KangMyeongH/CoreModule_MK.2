#pragma once
#include "RenderPass.h"

namespace engine
{
    class COREMODULE_API DeferredPass : public RenderPass
    {
	    //======================================//
        //				constructor				//
        //======================================//
    public:
        ~DeferredPass() override = default;

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
        SharedPtr<Shader> m_DeferredShader;
        ComPtr<ID3D11DepthStencilState> m_DeferredDSState;
    };
}
