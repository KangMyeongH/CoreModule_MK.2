#include "Mesh.h"

engine::Mesh::Mesh()
{
}

engine::Mesh::~Mesh()
{
}

engine::SharedPtr<engine::Mesh> engine::Mesh::Create(const SharedPtr<VIBuffer>& viBuffer)
{
	SharedPtr<Mesh> mesh{ new Mesh(), [](const Mesh* ptr) { delete ptr; } };
	mesh->m_VIBuffer = viBuffer;

	return mesh;
}

void engine::Mesh::Destroy()
{
}

void engine::Mesh::SetSubMesh(const std::vector<SubMesh>& subMeshes)
{
	m_SubMeshes = subMeshes;
}

void engine::Mesh::Bind(const ComPtr<ID3D11DeviceContext>& context)
{
	if (m_VIBuffer)
	{
		ID3D11Buffer* vertexBuffers[] = {
			m_VIBuffer->VertexBuffer.Get()
		};

		_uint vertexStrides[] = {
			m_VIBuffer->VertexStride
		};

		_uint offsets[] = {
			0
		};

		context->IASetVertexBuffers(0, m_VIBuffer->NumVertexBuffers, vertexBuffers, vertexStrides, offsets);
		context->IASetIndexBuffer(m_VIBuffer->IndexBuffer.Get(), m_VIBuffer->IndexFormat, 0);
		context->IASetPrimitiveTopology(m_VIBuffer->PrimitiveTopology);
	}
}

void engine::Mesh::Render(const ComPtr<ID3D11DeviceContext>& context)
{
	for (auto i = 0; i < m_SubMeshes.size(); ++i)
	{
		context->DrawIndexed(m_SubMeshes[i].IndexCount, m_SubMeshes[i].IndexOffset, 0);
	}
}


//// 공유된 VertexBuffer, IndexBuffer 설정
//context->IASetVertexBuffers(...);
//context->IASetIndexBuffer(...);
//
//// SubMesh 1 (Material A)
//context->PSSetShaderResources(0, 1, &materialA_TextureSRV);
//context->DrawIndexed(500, 0, 0);
//
//// SubMesh 2 (Material B)
//context->PSSetShaderResources(0, 1, &materialB_TextureSRV);
//context->DrawIndexed(400, 500, 0);