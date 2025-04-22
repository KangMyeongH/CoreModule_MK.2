#pragma once
#include "UI.h"

namespace engine
{
    class COREMODULE_API TextUI : public UI
    {
	    DECLARE_REGISTER_COMPONENT(TextUI)
	    //======================================//
        //				constructor				//
        //======================================//
    protected:
        explicit TextUI(const SharedPtr<GameObject>& owner);
        ~TextUI() override = default;
        TextUI(const TextUI& rhs);
        //======================================//
        //				 property				//
        //======================================//
    public:
        void SetFont(const _wstring& font);
        _wstring GetFont() const { return m_FontName; }

        void SetText(const _wstring& text) { m_Text = text; }
        _wstring GetText() const { return m_Text; }

        void SetColor(const _float4& color) { m_Color = color; }
        _float4 GetColor() const { return m_Color; }

        void SetSize(const _float size) { m_Size = size; }
        _float GetSize() const { return m_Size; }

        void SetSorting(_int sort, ApplicationMode mode = CLIENT) override;

        //======================================//
        //				  method				//
        //======================================//
    public:
    	_bool IsMouseHovered() override;

    	_bool IsButtonDown() override;

    	_bool IsButtonHold() override;

    	_bool IsButtonUp() override;

    	void Update() override;

    	HRESULT InputAssembler(const ComPtr<ID3D11DeviceContext>& context) override;

    	void RenderUI(const ComPtr<ID3D11DeviceContext>& context) override;

        void registerComponent(ApplicationMode mode = CLIENT) override;

    	//======================================//
        //				 serialize				//
        //======================================//
    public:
        void to_json(nlohmann::ordered_json& j) override;
        void from_json(const nlohmann::ordered_json& j) override;

        //======================================//
        //				  fields				//
        //======================================//
    private:
        SharedPtr<DirectX::SpriteFont> 	m_Font;
        SharedPtr<DirectX::SpriteBatch> m_Batch;
        _wstring 						m_FontName;
        _wstring 						m_Text;
        _float4 						m_Color;
        _float       					m_Size;
    };
}
