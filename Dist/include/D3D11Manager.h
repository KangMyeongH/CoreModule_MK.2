#pragma once
#include "core_defines.h"

namespace engine
{
    using TextureMap = std::unordered_map<_wstring, ID3D11ShaderResourceView*>;

    class COREMODULE_API D3D11Manager
    {
    private:
        //======================================//
        //				constructor				//
        //======================================//

        D3D11Manager();
        ~D3D11Manager();
    public:
        DECLARE_SINGLETON(D3D11Manager)

        //======================================//
        //				 property				//
        //======================================//

        ID3D11Device* 			GetDevice() const { return m_Device; }
        ID3D11DeviceContext* 	GetContext() const { return m_DeviceContext; }
        IDXGISwapChain* 		GetSwapChain() const { return m_SwapChain; }
        ID3D11RenderTargetView* GetMainRTV() const { return m_BackBufferRTV; }
        ID3D11DepthStencilView* GetDepthStencilView() const { return m_DepthStencilView; }

        //======================================//
        //				  method				//
        //======================================//

        HRESULT	Initialize(HWND hwnd, _bool isWindowed, _uint winSizeX, _uint winSizeY);

        HRESULT CreateTexture(const _wstring& path, ID3D11ShaderResourceView** srv);


    	void 					Release();

    private:
        HRESULT 				readySwapChain(HWND hWnd, _bool isWindowed, _uint winSizeX, _uint winSizeY);
        HRESULT 				readyBackBufferRenderTargetView();
        HRESULT 				readyDepthStencilView(_uint winSizeX, _uint winSizeY);

    private:
        ID3D11Device* 				m_Device;
        ID3D11DeviceContext* 		m_DeviceContext;
        IDXGISwapChain* 			m_SwapChain;
        ID3D11RenderTargetView* 	m_BackBufferRTV;
        ID3D11DepthStencilView* 	m_DepthStencilView;

        TextureMap                  m_TextureMap;
    };
}
