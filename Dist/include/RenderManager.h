#pragma once
#include "core_defines.h"

/*
1. SkyPass (SkyAtmosphere, Cloud)
2. PrePass (optional Depth Prepass)
3. BasePass (GBuffer 채우기)
4. VelocityPass (TAA용 모션벡터)
5. ShadowPass (Light별 ShadowMap 생성)
6. LightingPass (GBuffer 기반 조명 계산)
7. SSAO / SSR / ReflectionPass
8. TranslucencyShadowPass
9. TranslucencyPass (Forward+AlphaBlend)
10. CustomDepthPass (Outlining용 등)
11. PostProcessingPasses (Bloom, Tonemap, etc.)
12. AfterTranslucency (Distortion 등)
13. Overlay/UI Pass
 */

namespace engine
{
	class RenderTarget;
	class Camera;
    class Renderer;
    class Light;
    class SkySphere;
    class RenderPass;

    using Renderers = std::vector<SharedPtr<Renderer>>;
    using RendererList = std::list<SharedPtr<Renderer>>;
    using Cameras = std::list<SharedPtr<Camera>>;
    using Models = std::unordered_map<_wstring, ModelData>;
    using Lights = std::vector<SharedPtr<Light>>;
    using LightList = std::list<SharedPtr<Light>>;

    class COREMODULE_API RenderManager
    {
    private:
        //======================================//
        //				constructor				//
        //======================================//

        RenderManager();
        ~RenderManager();
    public:
        DECLARE_SINGLETON(RenderManager)

        //======================================//
        //				 property				//
        //======================================//
        void                Initialize(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& context);

        void                SetMainCamera(const SharedPtr<Camera>& camera) { m_MainCamera = camera; }
        SharedPtr<Camera> 	GetMainCamera() const { return m_MainCamera.lock(); }

        _float4X4           GetViewMat() const { return m_ViewMat; }
        _float4X4           GetProjMat() const { return m_ProjMat; }

        SharedPtr<SkySphere> GetSkySphere() const { return m_SkySphere; }

        void                AddCamera(const SharedPtr<Camera>& camera);

        void                AddRenderer(const SharedPtr<Renderer>& renderer);

        void                AddLight(const SharedPtr<Light>& light);

        //======================================//
        //				  method				//
        //======================================//

        void UpdateMainCamera();

        void RenderSkySphere(const ComPtr<ID3D11DeviceContext>& context);
    	void Render(const ComPtr<ID3D11DeviceContext>& context);

        void RegisterRenderer();
        void FlushDestroyRenderer();

        void RegisterLight();
        void FlushDestroyLight();

        void FlushDestroyCamera();

        void AddRenderTarget(const _string& tag, const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& context, _uint sizeX, _uint sizeY, DXGI_FORMAT pixelFormat, const _float4& clearColor);
        void AddMRT(const _string& mrtTag, const _string& targetTag);

        SharedPtr<RenderTarget> FindRenderTarget(const _string& tag);
        std::list<SharedPtr<RenderTarget>>* FindMRT(const _string& tag);

        HRESULT BeginMRT(const _string& tag);
        HRESULT EndMRT();

        HRESULT SkyPass(const ComPtr<ID3D11DeviceContext>& context);
        HRESULT PrePass(const ComPtr<ID3D11DeviceContext>& context);
        HRESULT BasePass(const ComPtr<ID3D11DeviceContext>& context);
        HRESULT LightingPass(const ComPtr<ID3D11DeviceContext>& context);
        HRESULT DeferredPass(const ComPtr<ID3D11DeviceContext>& context);
        HRESULT OutlinePass(const ComPtr<ID3D11DeviceContext>& context);

        HRESULT DebugRender(const ComPtr<ID3D11DeviceContext>& context);

        void Release();

    private:
        //======================================//
        //				  fields				//
        //======================================//

        Renderers           m_Renderers;
        RendererList        m_RegisterQueue;

        Lights              m_Lights;
        LightList           m_LightRegisterQueue;

    	WeakPtr<Camera> 	m_MainCamera;
        Cameras 			m_Cameras;
        Models              m_Models;

        SharedPtr<SkySphere> 	m_SkySphere;
        _float4             	m_AmbientColor;

        std::unordered_map<_string, SharedPtr<RenderTarget>> m_RenderTargets;
        std::unordered_map<_string, std::list<SharedPtr<RenderTarget>>> m_MRTs;

        SharedPtr<Shader> 				m_PrePassShader;
        ComPtr<ID3D11DepthStencilState> m_PrePassDSState;

        ComPtr<ID3D11DepthStencilState> m_RTDebugDSState;

        _float4X4           m_ViewMat;
        _float4X4           m_ProjMat;

        std::unique_ptr<RenderPass> m_SkyPass;
        std::unique_ptr<RenderPass> m_PrePass;
        std::unique_ptr<RenderPass> m_BasePass;
        std::unique_ptr<RenderPass> m_LightingPass;
        std::unique_ptr<RenderPass> m_DeferredPass;
        std::unique_ptr<RenderPass> m_OutlinePass;
    };
}
