#pragma once
#include "RenderPass.h"

namespace engine
{
    class COREMODULE_API FinalPass : public RenderPass
    {
	    //======================================//
        //				constructor				//
        //======================================//
    public:
        ~FinalPass() override = default;

        //======================================//
        //				  method				//
        //======================================//
    public:
        HRESULT Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
        HRESULT Render(ID3D11DeviceContext* context, void* data) override;
        HRESULT RenderEditor(ID3D11DeviceContext* context, void* data, _bool isGame) override;
        void Release() override;

        //======================================//
        //				  fields				//
        //======================================//
    private:
        SharedPtr<Shader> m_Shader;
        ComPtr<ID3D11DepthStencilState> m_DSState;
    };
}
