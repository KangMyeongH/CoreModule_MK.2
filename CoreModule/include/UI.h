#pragma once
#include "Behaviour.h"

namespace engine
{
    class COREMODULE_API UI : public Behaviour
    {
    protected:
        //======================================//
        //				constructor				//
        //======================================//

        explicit UI(const SharedPtr<GameObject>& owner);
        ~UI() override = default;
        UI(const UI& rhs);

    public:
        //======================================//
        //				 property				//
        //======================================//

        _uint GetSorting() const;
        void SetSorting( _uint sort);

        //======================================//
        //				  method				//
        //======================================//

        virtual _bool IsMouseHovered() = 0;
        virtual _bool IsButtonDown() = 0;
        virtual _bool IsButtonHold() = 0;
        virtual _bool IsButtonUp() = 0;

        virtual void Update() = 0;
        virtual void RenderUI(const ComPtr<ID3D11DeviceContext>& context) = 0;

        void Destroy() override;
        SharedPtr<Component> Clone() const override = 0;

    protected:
        void registerComponent() override;

    public:
        //======================================//
        //				 serialize				//
        //======================================//

        void to_json(nlohmann::ordered_json& j) override = 0;
        void from_json(const nlohmann::ordered_json& j) override = 0;

    protected:
        //======================================//
        //				  fields				//
        //======================================//

        _uint m_SortingOrder;
    };
}
