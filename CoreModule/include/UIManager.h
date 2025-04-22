#pragma once
#include "core_defines.h"

namespace engine
{
	class TextUI;
}

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

        UIManager();
        ~UIManager();
    public:
        DECLARE_SINGLETON(UIManager)

        //======================================//
        //				 property				//
    	//======================================//

        void SetDirty(const _bool dirty) { m_bDirty = dirty; }
        _bool GetDirty() const { return m_bDirty; }

        SharedPtr<DirectX::SpriteFont> GetFont(const _wstring& name);

        SharedPtr<DirectX::SpriteBatch> GetBatch() const { return m_Batch; }

        //======================================//
        //				  method				//
        //======================================//

        void Initialize();

        void UpdateUI();
        void Render(const ComPtr<ID3D11DeviceContext>& context);

        void AddUI(const SharedPtr<UI>& ui, _bool isText);

        void AddFont(const _wstring& name, const _wstring& path);

        void OnSortingChanged(const SharedPtr<UI>& ui, _int oldSort, _int newSort, _bool isText);

        void RegisterUI();
        void FlushDestroyUI();

        void Release();

    private:
        //======================================//
        //				  fields				//
        //======================================//

        std::map<_int, std::vector<SharedPtr<UI>>> m_UIMap;
        std::map<_int, std::vector<SharedPtr<UI>>> m_TextUIMap;

        UIList  m_RegisterQueue;
        std::list<SharedPtr<UI>> m_RegisterTextQueue;

        std::unordered_map<_wstring, SharedPtr<DirectX::SpriteFont>> m_Fonts;
        SharedPtr<DirectX::SpriteBatch> m_Batch;

        _int    m_MaxSort;
        _int    m_MinSort;
        _bool   m_bDirty;
    };
}
