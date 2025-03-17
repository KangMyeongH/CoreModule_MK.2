#pragma once
#include "UI.h"

namespace engine
{
	class Material;
}

namespace engine
{
    class COREMODULE_API TextureUI : public UI
    {
        DECLARE_REGISTER_COMPONENT(TextureUI)

    protected:
	    //======================================//
        //				constructor				//
        //======================================//

        explicit TextureUI(const SharedPtr<GameObject>& owner);
        ~TextureUI() override = default;
        TextureUI(const TextureUI& rhs);

    public:
        //======================================//
        //				 property				//
        //======================================//

        void SetTexture(const _wstring& path);
        ComPtr<ID3D11ShaderResourceView> GetTexture() const;

    public:
        //======================================//
        //				  method				//
        //======================================//

        _bool IsMouseHovered() override;
        _bool IsButtonDown() override;
        _bool IsButtonHold() override;
        _bool IsButtonUp() override;

        void Update() override;
        HRESULT InputAssembler(const ComPtr<ID3D11DeviceContext>& context) override;
        void RenderUI(const ComPtr<ID3D11DeviceContext>& context) override;

        void Destroy() override;

    protected:
        void registerComponent() override;

    public:
        //======================================//
        //				 serialize				//
        //======================================//

        void to_json(nlohmann::ordered_json& j) override;
        void from_json(const nlohmann::ordered_json& j) override;

    private:
        //======================================//
        //				  fields				//
        //======================================//

        _float4X4                           m_TextureScaleMatrix;
        _wstring                            m_TexturePath;
        _uint                               m_Width;
        _uint                               m_Height;
        _bool                               m_bFlipX;
        _bool                               m_bFlipY;
    };
}
