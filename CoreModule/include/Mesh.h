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
        SharedPtr<Mesh> Create();

        void SetVertices(const ComPtr<ID3D11Buffer>& vertex);
    	void SetIndices(const ComPtr<ID3D11Buffer>& index);

    	void Destroy() override;


    private:
        ComPtr<ID3D11Buffer> 	m_VertexBuffer;
        ComPtr<ID3D11Buffer> 	m_IndexBuffer;
        _uint                   m_IndexCount;
    };
}
