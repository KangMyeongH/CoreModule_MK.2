#pragma once
#include "core_defines.h"

namespace engine::editor
{
	class EditorComponentManager;
}

namespace editor
{
    namespace engine
    {
        class COREMODULE_API EditorCore
        {
        //======================================//
        //				constructor				//
        //======================================//
        private:
            EditorCore();
            ~EditorCore();
        public:
            EditorCore(const EditorCore&) = delete;
            EditorCore(EditorCore&&) = delete;
            EditorCore& operator=(const EditorCore&) = delete;
            EditorCore& operator=(EditorCore&&) = delete;

            static EditorCore& GetInstance() { static EditorCore s_Instance; return s_Instance; }

        //======================================//
        //				 property				//
        //======================================//
        public:
            


        //======================================//
        //				  method				//
        //======================================//
        public:
            HRESULT Initialize(HWND hwnd);

            void Initialization();
            void SceneRender(const ::engine::ComPtr<ID3D11DeviceContext>& context);
            void Decommissioning();

        private:
            void renderScene(const ::engine::ComPtr<ID3D11DeviceContext>& context);
            void readySceneView(int width, int height);

            void renderGame(const ::engine::ComPtr<ID3D11DeviceContext>& context);
            void readyGameView(int width, int height);

        //======================================//
        //				 serialize				//
        //======================================//

        //======================================//
        //				  fields				//
        //======================================//
        private:
            ::engine::editor::EditorComponentManager* 	m_EditorComponentManager;

            ::engine::ComPtr<ID3D11RenderTargetView> 	m_SceneTargetView;
            ::engine::ComPtr<ID3D11ShaderResourceView>	m_SceneResourceView;
            ::engine::ComPtr<ID3D11DepthStencilView> 	m_SceneDepthStencilView;

            ::engine::ComPtr<ID3D11RenderTargetView>    m_GameTargetView;
            ::engine::ComPtr<ID3D11ShaderResourceView>  m_GameResourceView;
            ::engine::ComPtr<ID3D11DepthStencilView>    m_GameDepthStencilView;

            ::engine::_int m_OffscreenWidth;
            ::engine::_int m_OffscreenHeight;

            ::engine::_bool m_bEditorMode;
        };
    }
}
