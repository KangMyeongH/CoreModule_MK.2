#pragma once
#include "core_defines.h"

namespace engine
{
    using TextureMap = std::unordered_map<_wstring, ComPtr<ID3D11ShaderResourceView>>;

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

        ComPtr<ID3D11Device> 			GetDevice() const { return m_Device; }
        ComPtr<ID3D11DeviceContext> 	GetContext() const { return m_DeviceContext; }
        ComPtr<IDXGISwapChain> 		    GetSwapChain() const { return m_SwapChain; }
        ComPtr<ID3D11RenderTargetView>  GetMainRTV() const { return m_BackBufferRTV; }
        ComPtr<ID3D11DepthStencilView>  GetDepthStencilView() const { return m_DepthStencilView; }

        _uint                           GetWinSizeX() const { return m_WinSizeX; }
        _uint                           GetWinSizeY() const { return m_WinSizeY; }


        //======================================//
        //				  method				//
        //======================================//

        HRESULT	Initialize(HWND hwnd, _bool isWindowed, _uint winSizeX, _uint winSizeY);

        HRESULT CreateTexture(const _wstring& path, ID3D11ShaderResourceView** srv);
        HRESULT CreateShader(const _wstring& path, const SharedPtr<Shader>& shader);
        HRESULT CreateMesh(const _wstring& path)


    	void 	Release();

    private:
        HRESULT readySwapChain(HWND hWnd, _bool isWindowed, _uint winSizeX, _uint winSizeY);
        HRESULT readyBackBufferRenderTargetView();
        HRESULT readyDepthStencilView(_uint winSizeX, _uint winSizeY);
        HRESULT compileShaderFromFile(const _wstring& path, const _string& entryPoint, const _string& targetProfile, ComPtr<ID3DBlob>& outBlob);
        void	compileInputLayoutFromReflector(std::vector<D3D11_INPUT_ELEMENT_DESC>* inputDesc, const ComPtr<ID3D11ShaderReflection>& reflector);
        void    reflectBufferFromReflector()

    private:
        //======================================//
        //				  fields				//
        //======================================//

        ComPtr<ID3D11Device> 		    m_Device;
        ComPtr<ID3D11DeviceContext> 	m_DeviceContext;
        ComPtr<IDXGISwapChain> 			m_SwapChain;
        ComPtr<ID3D11RenderTargetView> 	m_BackBufferRTV;
        ComPtr<ID3D11DepthStencilView> 	m_DepthStencilView;

        TextureMap                      m_TextureMap;

        _uint                           m_WinSizeX;
        _uint                           m_WinSizeY;
		
    };
}
