#pragma once
#include "core_defines.h"
namespace engine
{
    class UI;
    using UIs = std::vector<SharedPtr<UI>>;
    using UIList = std::list<SharedPtr<UI>>;

    class COREMODULE_API UIManager
    {
    private:
        //======================================//
        //				constructor				//
        //======================================//

        UIManager() = default;
        ~UIManager();
    public:
        DECLARE_SINGLETON(UIManager)

        //======================================//
        //				 property				//
        //======================================//


        //======================================//
        //				  method				//
        //======================================//

        void UpdateUI();
        void Render(const ComPtr<ID3D11DeviceContext>& context);

        void AddUI(const SharedPtr<UI>& ui);
        void RegisterUI();
        void FlushDestroyUI();

        void Release();

    private:
        //======================================//
        //				  fields				//
        //======================================//

    	UIs     m_UIs;
        UIList  m_RegisterQueue;
    };
}
