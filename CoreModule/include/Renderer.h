#pragma once
#include "Component.h"

namespace engine
{
    class Material;

    class Renderer: public Component
    {
    protected:


    protected:
        SharedPtr<Material> m_Material;
    };
}
