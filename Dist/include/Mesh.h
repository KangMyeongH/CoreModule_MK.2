#pragma once
#include "Object.h"

namespace engine
{
    class Mesh : public Object
    {
    protected:
        Mesh();
        ~Mesh() override;

    public:
        static SharedPtr<Mesh> Create(const SharedPtr<VIBuffer>& viBuffer);
    	void Destroy() override;

        void Bind(const ComPtr<ID3D11DeviceContext>& context);
        void Render(const ComPtr<ID3D11DeviceContext>& context);


    private:
        SharedPtr<VIBuffer> m_VIBuffer;
        std::vector<SubMesh> m_SubMeshes;
    };
}
