#pragma once
#include "Component.h"
#include "ComponentFactory.h"
#include "core_defines.h"
#include "EditorCamera.h"
#include "RenderManager.h"

namespace engine
{
	class GameObject;
	class Camera;
	class Component;
	class UI;
	class Renderer;
    class Effect;

    namespace editor
    {
        using Components = std::vector<SharedPtr<Component>>;
        using Renderers = std::vector<SharedPtr<Renderer>>;
        using Effects = std::vector<SharedPtr<Effect>>;
        using UIs = std::vector<SharedPtr<UI>>;

        using Colliders = std::vector<SharedPtr<Collider>>;

        class COREMODULE_API EditorComponentManager
        {
        //======================================//
        //				constructor				//
        //======================================//
        private:
            EditorComponentManager();
            ~EditorComponentManager();
        public:
            DECLARE_SINGLETON(EditorComponentManager)

        //======================================//
        //				 property				//
        //======================================//
        public:
            void SetDirty(const bool dirty) { m_DirtyFlag = dirty; }

            SharedPtr<Camera> GetMainCam() const { return m_MainCamera.lock(); }

        //======================================//
        //				  method				//
        //======================================//
        public:
            void Initialize();

            void Render(const ComPtr<ID3D11DeviceContext>& context, CamData* camData, _bool isGame);
            void RenderUIComponent(const ComPtr<ID3D11DeviceContext>& context);
            void RenderCollider(const ComPtr<ID3D11DeviceContext>& context, const _float4X4& viewMat, const _float4X4& projMat) const;
            void AddComponent(const SharedPtr<GameObject>& owner, const SharedPtr<Component>& component);
            void AddComponent(const SharedPtr<Component>& component);
            void AddFont(const _wstring& name, const _wstring& path);

            void OnSortingChanged(const SharedPtr<UI>& ui, _int oldSort, _int newSort, _bool isText);

            void RenderSkySphere(const ComPtr<ID3D11DeviceContext>& context);

            HRESULT SkyPass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame);
            HRESULT PrePass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame);
            HRESULT BasePass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame);
            HRESULT LightingPass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame);
            HRESULT DeferredPass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame);
            HRESULT OutlinePass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame);
            HRESULT EffectPass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame);
            HRESULT GlowPass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame);
        	HRESULT FinalPass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame);

            //template <typename T>
            //SharedPtr<T> CreateComponent(const SharedPtr<GameObject>& owner)
            //{
            //    static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");

            //    const _string typeName = StripMsvcClassName(typeid(T).name());

            //    SharedPtr<Component> component = ComponentFactory::GetInstance().CreateComponent(typeName);

            //    if (component)
            //    {
            //        AddComponent(owner, component);
            //    }

            //    return std::static_pointer_cast<T>(component);
            //}

            void FlushDestroyComponent();

            void Release();

        //======================================//
        //				  fields				//
        //======================================//
        private:
            Components          m_Components;
            Renderers 			m_Renderers;
            Effects             m_Effects;

            std::map<_int, std::vector<SharedPtr<UI>>> m_UIMap;
            std::map<_int, std::vector<SharedPtr<UI>>> m_TextUIMap;

            std::unordered_map<_wstring, SharedPtr<DirectX::SpriteFont>> m_Fonts;
            SharedPtr<DirectX::SpriteBatch> m_Batch;

            _int    m_MaxSort;
            _int    m_MinSort;

            Cameras 			m_Cameras;

            Colliders           m_Colliders;

            std::vector<SharedPtr<Light>> m_Lights;

            WeakPtr<Camera> 	m_MainCamera;

            _float4X4           m_ViewMat;
            _float4X4           m_ProjMat;

        	bool                m_DirtyFlag;

            std::unique_ptr<RenderPass> m_SkyPass;
            std::unique_ptr<RenderPass> m_PrePass;
            std::unique_ptr<RenderPass> m_BasePass;
            std::unique_ptr<RenderPass> m_LightingPass;
            std::unique_ptr<RenderPass> m_DeferredPass;
            std::unique_ptr<RenderPass> m_OutlinePass;
            std::unique_ptr<RenderPass> m_EffectPass;
            std::unique_ptr<RenderPass> m_GlowPass;
            std::unique_ptr<RenderPass> m_FinalPass;
        };
    }

}
