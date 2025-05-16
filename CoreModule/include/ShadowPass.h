#pragma once
#include "RenderPass.h"

/// <summary>
/// ShadowPass는 목요일에 마무리하는 걸로.
/// 내가 이거 지금 하면 개다 개.
/// 왈왈으르렁 컹컹
/// 얼추 구조 및 렌더 타겟은 구현완료함.
/// 나머지는 문서 정리하면서 구현할 예정
/// </summary>

namespace engine
{
    class COREMODULE_API ShadowPass : public RenderPass
    {
	    //======================================//
        //				constructor				//
        //======================================//
        ~ShadowPass() override = default;

        //======================================//
        //				  method				//
        //======================================//
    public:
        HRESULT Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
        HRESULT Render(ID3D11DeviceContext* context, void* data) override;
        void Release() override;

        //======================================//
        //				 serialize				//
        //======================================//

        //======================================//
        //				  fields				//
        //======================================//
    private:
        ComPtr<ID3D11DepthStencilView> 		m_ShadowDSV[NUM_CASCADES];
        ComPtr<ID3D11Texture2D>         	m_ShadowTex;
        ComPtr<ID3D11ShaderResourceView> 	m_ShadowSRV;
        ComPtr<ID3D11SamplerState>          m_SmpCmpLinear;
        ComPtr<ID3D11RasterizerState>       m_RSDepthBias;

        D3D11_VIEWPORT m_ShadowVP = { 0,0,static_cast<_float>(SHADOW_SIZE), static_cast<_float>(SHADOW_SIZE) , 0.0f, 1.0f };


        _float4X4 	m_LightViewProj[NUM_CASCADES] = {};
        _float4     m_CascadeSplits = {}; // xyz = splitDepth, w = padding
        _float4 	m_ShadowParams = {}; // x = bias, y = slopeBias, z = PCF halfSize, w = unused;
    };
}
