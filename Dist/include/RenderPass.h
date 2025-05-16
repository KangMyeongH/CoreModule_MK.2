#pragma once
#include "core_defines.h"

namespace engine
{
    class COREMODULE_API RenderPass
    {
        //======================================//
        //				constructor				//
        //======================================//
    public:
        virtual ~RenderPass() = default;

        //======================================//
        //				  method				//
        //======================================//
    public:
        virtual HRESULT 	Initialize(ID3D11Device* device, ID3D11DeviceContext* context) = 0;
        virtual HRESULT 	Render(ID3D11DeviceContext* context, void* data) = 0;
        virtual HRESULT     RenderEditor(ID3D11DeviceContext* context, void* data, _bool isGame) = 0;
        virtual void 		Release() = 0;
    };
}
