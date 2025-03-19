#pragma once
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

    namespace editor
    {
        using Components = std::vector<SharedPtr<Component>>;
        using Renderers = std::vector<SharedPtr<Renderer>>;
        using UIs = std::vector<SharedPtr<UI>>;

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

        //======================================//
        //				  method				//
        //======================================//
        public:
            void Render(const ComPtr<ID3D11DeviceContext>& context);
            void RenderUIComponent(const ComPtr<ID3D11DeviceContext>& context);
            void AddComponent(const SharedPtr<GameObject>& owner, const SharedPtr<Component>& component);

            void FlushDestroyComponent();

        //======================================//
        //				  fields				//
        //======================================//
        private:
            Components          m_Components;
            Renderers 			m_Renderers;
            UIs                 m_UIs;
            Cameras 			m_Cameras;

            EditorCamera        m_EditorCamera;
            WeakPtr<Camera> 	m_MainCamera;

            _float4X4           m_ViewMat;
            _float4X4           m_ProjMat;

        	bool                m_DirtyFlag;
        };
    }

}
