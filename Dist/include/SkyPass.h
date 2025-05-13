#pragma once
#include "RenderPass.h"
#include "SkySphere.h"

namespace engine
{
    class COREMODULE_API SkyPass : public RenderPass
    {
	    //======================================//
        //				constructor				//
        //======================================//
    public:
        ~SkyPass() override = default;

        //======================================//
        //				 property				//
        //======================================//
    public:
        SharedPtr<SkySphere> GetSkySphere() const { return m_SkySphere; }

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
        SharedPtr<SkySphere> m_SkySphere;
        ComPtr<ID3D11DepthStencilState> m_SkyPassDSState;
    };
}
