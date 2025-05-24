#pragma once
#include "RenderPass.h"

namespace engine
{

    class COREMODULE_API GlowPass : public RenderPass
    {
    public:
        enum Level { L0, L1, L2, L3, Count };
	    //======================================//
        //				constructor				//
        //======================================//
    public:
        ~GlowPass() override = default;

        //======================================//
        //				  method				//
        //======================================//
    public:
        HRESULT Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
        HRESULT Render(ID3D11DeviceContext* context, void* data) override;
        HRESULT RenderEditor(ID3D11DeviceContext* context, void* data, _bool isGame) override;

    	void Release() override;

    private:
        void initVP(UINT width, UINT height);
        void setVP(Level lv, UINT w, UINT h);

        //======================================//
        //				 serialize				//
        //======================================//

        //======================================//
        //				  fields				//
        //======================================//
        ComPtr<ID3D11DepthStencilState> m_DSState;
        ComPtr<ID3D11SamplerState> m_LinearClamp;
        ComPtr<ID3D11BlendState> m_AddBlend;
        SharedPtr<Shader> m_4x4Down;
        SharedPtr<Shader> m_6x6Down;
        SharedPtr<Shader> m_6x6Up;
        SharedPtr<Shader> m_4x4Up;
        SharedPtr<Shader> m_BlurH;
        SharedPtr<Shader> m_BlurV;
        SharedPtr<Shader> m_Composite;
        SharedPtr<Shader> m_Final;


        std::array<D3D11_VIEWPORT, Count> m_VP{};


    };
}
