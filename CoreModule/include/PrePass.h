#pragma once
#include "RenderPass.h"

namespace engine
{
    class COREMODULE_API PrePass : public RenderPass
    {
	    //======================================//
        //				constructor				//
        //======================================//
    public:
        ~PrePass() override = default;

        //======================================//
        //				 property				//
        //======================================//

        //======================================//
        //				  method				//
        //======================================//
    public:
        HRESULT Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
        HRESULT Render(ID3D11DeviceContext* context, void* data) override;
        void Release() override;

    private:
        void setMatrix(const _string& name, const _float4X4& value);

        //======================================//
        //				  fields				//
        //======================================//
    private:
        SharedPtr<Shader> 				m_PrePassShader;
        SharedPtr<Shader>               m_PrePassSkinnedShader;
        ComPtr<ID3D11DepthStencilState> m_PrePassDSState;
        ComPtr<ID3D11BlendState>        m_PrePassBlendState;
        ComPtr<ID3D11RenderTargetView>  m_DummyRTV;
    };
}
