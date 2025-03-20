#include "TextureUI.h"

#include "D3D11Manager.h"
#include "InputManager.h"
#include "Material.h"

DEFINE_REGISTER_COMPONENT(TextureUI)

engine::TextureUI::TextureUI(const SharedPtr<GameObject>& owner)
	: UI(owner), m_TextureScaleMatrix(),
	  m_Width(0), m_Height(0),
	  m_bFlipX(false), m_bFlipY(false)
{
}

engine::TextureUI::TextureUI(const TextureUI& rhs)
	: UI(rhs), m_TextureScaleMatrix(rhs.m_TextureScaleMatrix), m_TexturePath(rhs.m_TexturePath),
	  m_Width(rhs.m_Width), m_Height(rhs.m_Height),
	  m_bFlipX(rhs.m_bFlipX), m_bFlipY(rhs.m_bFlipY)
{
}

void engine::TextureUI::SetTexture(const _wstring& path)
{
	if (m_TexturePath != path)
	{
		m_TexturePath = path;

		ComPtr<ID3D11ShaderResourceView> texture;

		if (FAILED(D3D11Manager::GetInstance().CreateTexture(path, texture)))
		{
			m_Material->SetTexture("g_Texture", texture);

			std::cerr << "FAIL : TextureUI::SetTexture (" << WStringToString(path).c_str() << ")\n";
			return;
		}

		m_Material->SetTexture("g_Texture", texture);

		ComPtr<ID3D11Resource> resource;
		texture->GetResource(resource.GetAddressOf());

		ComPtr<ID3D11Texture2D> texture2D;
		resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(texture2D.GetAddressOf()));

		D3D11_TEXTURE2D_DESC desc;
		texture2D->GetDesc(&desc);

		m_Width = desc.Width;
		m_Height = desc.Height;

		const _matrix mat = DirectX::XMMatrixScaling(static_cast<_float>(m_Width), static_cast<_float>(m_Height), 1.f);
		XMStoreFloat4x4(&m_TextureScaleMatrix, mat);
	}
}

engine::ComPtr<ID3D11ShaderResourceView> engine::TextureUI::GetTexture() const
{
	return nullptr;
}

engine::_bool engine::TextureUI::IsMouseHovered()
{
	const Vector3 mousePos = InputManager::GetInstance().GetMousePos();
	POINT winMousePos;

	winMousePos.x = static_cast<long>(mousePos.Value.x);
	winMousePos.y = static_cast<long>(mousePos.Value.y);

	const _matrix worldMat = XMMatrixMultiply(XMLoadFloat4x4(&m_TextureScaleMatrix), GetTransform()->GetWorldMatrix());

	_vector vScale;
	XMMatrixDecompose(&vScale, nullptr, nullptr, worldMat);

	_float2 scale;
	XMStoreFloat2(&scale, vScale);

	const Vector3 position = GetTransform()->Position();

	const _float halfWinX = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeX()) * 0.5f;
	const _float halfWinY = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeY()) * 0.5f;

	const RECT rect = {
		static_cast<LONG>(position.Value.x + halfWinX - scale.x * 0.5f),
		static_cast<LONG>(position.Value.y - halfWinY + scale.y * 0.5f),
		static_cast<LONG>(position.Value.x + halfWinX + scale.x * 0.5f),
		static_cast<LONG>(position.Value.y - halfWinY - scale.y * 0.5f)
	};
	
	return PtInRect(&rect, winMousePos);
}

engine::_bool engine::TextureUI::IsButtonDown()
{
	return IsMouseHovered() && InputManager::GetInstance().IsKeyDown(VK_LBUTTON);
}

engine::_bool engine::TextureUI::IsButtonHold()
{
	return IsMouseHovered() && InputManager::GetInstance().IsKeyPressed(VK_LBUTTON);
}

engine::_bool engine::TextureUI::IsButtonUp()
{
	return IsMouseHovered() && InputManager::GetInstance().IsKeyUp(VK_LBUTTON);
}

void engine::TextureUI::Update()
{
}

HRESULT engine::TextureUI::InputAssembler(const ComPtr<ID3D11DeviceContext>& context)
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


	std::cerr << "No VIBuffer! \n";

	return E_FAIL;
}

void engine::TextureUI::RenderUI(const ComPtr<ID3D11DeviceContext>& context)
{
	if (m_Material != nullptr)
	{
		_float4X4 worldMat;
		XMStoreFloat4x4(&worldMat, XMMatrixTranspose(XMMatrixMultiply(XMLoadFloat4x4(&m_TextureScaleMatrix), GetTransform()->GetWorldMatrix())));

		m_Material->SetMatrix("g_WorldMatrix", worldMat);

		m_Material->Bind(context.Get());

		if (FAILED(InputAssembler(context)))
		{
			std::cerr << "Failed IA \n";
		}

		context->DrawIndexed(m_VIBuffer->NumIndices, 0, 0);
	}
}

void engine::TextureUI::Destroy()
{
	m_bDestroyed = true;
}

void engine::TextureUI::registerComponent(ApplicationMode mode)
{
	UI::registerComponent();

	m_Material = Material::Create(shared_from_this());
	m_Material->LoadShader(L"..\\Client\\Assets\\Resource\\Shader\\TextureShader.hlsl");
	m_VIBuffer = D3D11Manager::GetInstance().GetVIBuffer(VIBufferType_POSTEX_RECT);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;  			// 고품질 필터링 (텍스처 왜곡 방지)
	samplerDesc.MaxAnisotropy = 16;  							// 최대 16배 이방성 필터링
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 0.0f;  						// 투명한 배경 유지
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 0.0f;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ComPtr<ID3D11SamplerState> sampler;

	D3D11Manager::GetInstance().CreateSampler(samplerDesc, sampler);

	m_Material->SetSampler("g_Sampler", sampler);

	if (!m_TexturePath.empty())
	{
		_wstring path = m_TexturePath;
		m_TexturePath.clear();
		SetTexture(path);
	}
}

void engine::TextureUI::to_json(nlohmann::ordered_json& j)
{
	std::string type = "TextureUI";
	j = nlohmann::ordered_json{
		{"type", type},
		{"instanceID", GetInstanceID()},
		{"enable", m_bEnabled},
		{"sortingOrder", m_SortingOrder},
		{"path", m_TexturePath},
		{"flipX", m_bFlipX},
		{"flipY", m_bFlipY}
	};
}

void engine::TextureUI::from_json(const nlohmann::ordered_json& j)
{
	if (j.contains("instanceID"))
	{
		SetInstanceID(j.at("instanceID").get<_int>());
	}
	if (j.contains("enable"))
	{
		j.at("enable").get_to(m_bEnabled);
	}
	if (j.contains("sortingOrder"))
	{
		j.at("sortingOrder").get_to(m_SortingOrder);
	}
	if (j.contains("path"))
	{
		j.at("path").get_to(m_TexturePath);
	}
	if (j.contains("flipX"))
	{
		j.at("flipX").get_to(m_bFlipX);
	}
	if (j.contains("flipY"))
	{
		j.at("flipY").get_to(m_bFlipY);
	}
}
