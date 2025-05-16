#pragma once
#include "core_defines.h"
#include "EditorCamera.h"

namespace engine
{
	class RenderTarget;
}

namespace engine
{
	namespace editor
	{
        class Grid;
	}
}

namespace engine
{
    namespace editor
    {
        class EditorComponentManager;

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
            void 	SetEditorMode(const _bool editorMode) { m_bEditorMode = editorMode; }
            _bool 	IsEditorMode() const { return m_bEditorMode; }

            ComPtr<ID3D11ShaderResourceView> GetSceneTexture() const { return m_SceneResourceView; }
            ComPtr<ID3D11ShaderResourceView> GetGameTexture() const { return m_GameResourceView; }

            ComPtr<ID3D11DepthStencilView> GetGameDSV() const { return m_GameDepthStencilView; }
            ComPtr<ID3D11DepthStencilView> GetSceneDSV() const { return m_SceneDepthStencilView; }

        	ComPtr<ID3D11RenderTargetView> GetGameRTV() const { return m_GameTargetView; }
            ComPtr<ID3D11RenderTargetView> GetSceneRTV() const { return m_SceneTargetView; }


            SharedPtr<Grid> GetGrid() { return m_Grid; }

            EditorCamera& GetEditorCamera() { return m_EditorCamera; }

        //======================================//
        //				  method				//
        //======================================//
        public:
            HRESULT Initialize(HWND hwnd);

            void Initialization();
            void SceneRender(const ComPtr<ID3D11DeviceContext>& context);
            void Decommissioning();

            void RenderScene(const ComPtr<ID3D11DeviceContext>& context);
            void ReadySceneView(int width, int height);

            void RenderGame(const ComPtr<ID3D11DeviceContext>& context);
            void ReadyGameView(int width, int height);

            void AddRenderTarget(const _string& tag, const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& context, _uint sizeX, _uint sizeY, DXGI_FORMAT pixelFormat, const _float4& clearColor, _bool isGame);
            void AddMRT(const _string& mrtTag, const _string& targetTag, _bool isGame);

            SharedPtr<RenderTarget> FindRenderTarget(const _string& tag, _bool isGame);
            std::list<SharedPtr<RenderTarget>>* FindMRT(const _string& tag, _bool isGame);

            HRESULT BeginMRT(const _string& tag, _bool isGame);
            HRESULT EndMRT(_bool isGame);

        //======================================//
        //				  fields				//
        //======================================//
        private:
            EditorComponentManager* 	m_EditorComponentManager;

            ComPtr<ID3D11RenderTargetView> 		m_SceneTargetView;
            ComPtr<ID3D11ShaderResourceView>	m_SceneResourceView;
            ComPtr<ID3D11DepthStencilView> 		m_SceneDepthStencilView;

            ComPtr<ID3D11RenderTargetView>    m_GameTargetView;
            ComPtr<ID3D11ShaderResourceView>  m_GameResourceView;
            ComPtr<ID3D11DepthStencilView>    m_GameDepthStencilView;

            std::unordered_map<_string, SharedPtr<RenderTarget>> m_SceneRenderTargets;
            std::unordered_map<_string, std::list<SharedPtr<RenderTarget>>> m_SceneMRTs;

            std::unordered_map<_string, SharedPtr<RenderTarget>> m_GameRenderTargets;
            std::unordered_map<_string, std::list<SharedPtr<RenderTarget>>> m_GameMRTs;

            EditorCamera m_EditorCamera;
            SharedPtr<Grid> m_Grid;

            _int m_OffscreenWidth;
            _int m_OffscreenHeight;

            _bool m_bEditorMode;
        };
    }
}
