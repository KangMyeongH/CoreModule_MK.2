#pragma once
#include "core_defines.h"

namespace engine
{
    class UI;
	class Camera;
	class Renderer;

    namespace editor
    {
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


        //======================================//
        //				  method				//
        //======================================//



        //======================================//
        //				  fields				//
        //======================================//
        private:
            Renderers 			m_Renderers;
            UIs                 m_UIs;


        };
    }

}
