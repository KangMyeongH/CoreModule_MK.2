#pragma once
#include "RenderPass.h"

namespace engine
{
	class RenderTarget;
}

namespace engine
{
    class COREMODULE_API BasePass : public RenderPass
    {
	    //======================================//
        //				constructor				//
        //======================================//
    public:
        ~BasePass() override = default;

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
        ComPtr<ID3D11DepthStencilState> m_BasePassDSState;
    };
}
