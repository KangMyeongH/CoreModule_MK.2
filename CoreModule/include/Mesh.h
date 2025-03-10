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

    	void Destroy() override;


    private:
        D3D11_BUFFER_DESC       m_BufferDesc;
        D3D11_SUBRESOURCE_DATA  m_InitialDesc;

        _uint                   m_NumVertexBuffers;
        _uint                   m_VertexStride;
        _uint                   m_IndexStride;
        _uint					m_NumIndices;

        ComPtr<ID3D11Buffer> 	m_VertexBuffer;
        ComPtr<ID3D11Buffer> 	m_IndexBuffer;
        _uint                   m_IndexCount;
    };
}
