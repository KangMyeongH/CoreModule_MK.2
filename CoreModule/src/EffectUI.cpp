#include "EffectUI.h"

#include "D3D11Manager.h"
#include "EditorComponentManager.h"
#include "Material.h"
#include "UIManager.h"

DEFINE_REGISTER_COMPONENT(EffectUI)

engine::EffectUI::EffectUI(const SharedPtr<GameObject>& owner, const _string& name)
	: UI(owner, name), m_TextureSize()
{
}

engine::EffectUI::EffectUI(const EffectUI& rhs)
	: UI(rhs), m_TextureSize(rhs.m_TextureSize)
{
}

void engine::EffectUI::Update()
{

}

HRESULT engine::EffectUI::InputAssembler(const ComPtr<ID3D11DeviceContext>& context)
{
	if (m_VIBuffer)
	{
		ID3D11Buffer* vertexBuffers[] = {
			m_VIBuffer->VertexBuffer.Get()
		};

		_uint		vertexStrides[] = {
			m_VIBuffer->VertexStride
		};

		_uint		offsets[] = {
			0,
		};

		context->IASetVertexBuffers(0, m_VIBuffer->NumVertexBuffers, vertexBuffers, vertexStrides, offsets);
		context->IASetIndexBuffer(m_VIBuffer->IndexBuffer.Get(), m_VIBuffer->IndexFormat, 0);
		context->IASetPrimitiveTopology(m_VIBuffer->PrimitiveTopology);

		return S_OK;
	}

	return E_FAIL;
}

void engine::EffectUI::RenderUI(const ComPtr<ID3D11DeviceContext>& context)
{
	if (m_Material != nullptr)
	{
		if (m_Material->GetShader())
		{
			if (m_TextureSize.x == 0.f && m_TextureSize.y == 0.f)
			{
				_float width = static_cast<_float>(m_Material->GetTextureWidth());
				_float height = static_cast<_float>(m_Material->GetTextureHeight());
				m_TextureSize.x = width;
				m_TextureSize.y = height;
			}

			const _matrix textureScaleMat = DirectX::XMMatrixScaling(m_TextureSize.x, m_TextureSize.y, 1.f);
			_float4X4 worldMat;
			XMStoreFloat4x4(&worldMat, XMMatrixMultiply(textureScaleMat, GetTransform()->GetWorldMatrix()));

			m_Material->SetMatrix("g_WorldMatrix", worldMat);

			m_Material->Bind(context.Get());

			if (FAILED(InputAssembler(context)))
			{
				std::cerr << "Failed IA \n";
			}

			context->DrawIndexed(m_VIBuffer->NumIndices, 0, 0);
		}
	}
}

void engine::EffectUI::Destroy()
{
	m_bDestroyed = true;
}

void engine::EffectUI::registerComponent(ApplicationMode mode)
{
	if (mode == CLIENT)
	{
		UIManager::GetInstance().AddUI(std::static_pointer_cast<UI>(shared_from_this()), false);
	}

	if (mode == EDITOR)
	{
		editor::EditorComponentManager::GetInstance().AddComponent(std::static_pointer_cast<UI>(shared_from_this()));
	}

	if (!m_Material)
	{
		m_Material = Material::Create(shared_from_this());
	}

	m_VIBuffer = D3D11Manager::GetInstance().GetVIBuffer(VIBufferType_POSTEX_RECT);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;  			// 고품질 필터링 (텍스처 왜곡 방지)
	samplerDesc.MaxAnisotropy = 16;  							// 최대 16배 이방성 필터링
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 0.0f;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	D3D11Manager::GetInstance().CreateSampler(samplerDesc, m_Sampler);
}
