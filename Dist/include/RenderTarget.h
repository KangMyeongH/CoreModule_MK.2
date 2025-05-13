#pragma once
#include "Object.h"

namespace engine
{
    class COREMODULE_API RenderTarget : public Object
    {
        //======================================//
        //				constructor				//
        //======================================//
    private:
        RenderTarget();
        ~RenderTarget() override = default;
        RenderTarget(const RenderTarget& rhs);

        //======================================//
        //				 property				//
        //======================================//
    public:
        ComPtr<ID3D11ShaderResourceView> GetSRV() { return m_SRV; }
        ComPtr<ID3D11RenderTargetView> GetRTV() { return m_RTV; }

        //======================================//
        //				  method				//
        //======================================//
    public:
        HRESULT Initialize(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& context, _uint sizeX, _uint sizeY, DXGI_FORMAT pixelFormat, const _float4& clearColor);

        void Clear(ID3D11DeviceContext* context);

    	static SharedPtr<RenderTarget> Create(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& context, _uint sizeX, _uint sizeY, DXGI_FORMAT pixelFormat, const _float4& clearColor);



    	void Destroy() override {}

        //======================================//
        //				  fields				//
        //======================================//
    private:
        ComPtr<ID3D11Texture2D>     		m_Texture2D;
        ComPtr<ID3D11RenderTargetView> 		m_RTV;
        ComPtr<ID3D11ShaderResourceView>	m_SRV;
        _float4                             m_ClearColor;
    };
}
