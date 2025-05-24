#pragma once
#include "RenderPass.h"

namespace engine
{
    class COREMODULE_API EffectPass : public RenderPass
    {
	    //======================================//
        //				constructor				//
        //======================================//
    public:
        ~EffectPass() override = default;
        
        //======================================//
        //				  method				//
        //======================================//
    public:
        HRESULT Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
        HRESULT Render(ID3D11DeviceContext* context, void* data) override;
        HRESULT RenderEditor(ID3D11DeviceContext* context, void* data, _bool isGame) override;

    	void Release() override;

    private:
        _float calcDepth(const Vector3& worldPos, const Vector3& cameraPos);

        //======================================//
        //				  fields				//
        //======================================//

    private:
        SharedPtr<Shader> 				m_PassShader;
        ComPtr<ID3D11DepthStencilState> m_PassDSState;
        ComPtr<ID3D11BlendState> 		m_PassBlendState;
        ComPtr<ID3D11RasterizerState> 	m_RasterState;
        ComPtr<ID3D11SamplerState>      m_WrapSamplerState;
        ComPtr<ID3D11SamplerState>      m_ClampSamplerState;
    };
}
