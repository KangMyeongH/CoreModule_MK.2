#pragma once
#include "core_defines.h"

namespace engine
{
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
        //				  method				//
        //======================================//

        ID3D11Device* 			GetDevice() const { return m_Device; }

        ID3D11DeviceContext* 	GetContext() const { return m_DeviceContext; }

        IDXGISwapChain* 		GetSwapChain() const { return m_SwapChain; }

        ID3D11RenderTargetView* GetMainRTV() const { return m_BackBufferRTV; }

        ID3D11DepthStencilView* GetDepthStencilView() const { return m_DepthStencilView; }

        HRESULT 				Initialize(HWND hwnd, _bool isWindowed, _uint winSizeX, _uint winSizeY);

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
    };
}
