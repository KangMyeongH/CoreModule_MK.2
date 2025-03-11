#include "TextureUI.h"

#include "D3D11Manager.h"
#include "InputManager.h"

DEFINE_REGISTER_COMPONENT(TextureUI)

engine::TextureUI::TextureUI(const SharedPtr<GameObject>& owner)
	: UI(owner), m_TextureScaleMatrix(),
	  m_Width(0), m_Height(0),
	  m_bFlipX(false), m_bFlipY(false)
{
}

engine::TextureUI::TextureUI(const TextureUI& rhs)
	: UI(rhs), m_Texture(rhs.m_Texture), m_TextureScaleMatrix(rhs.m_TextureScaleMatrix),
	  m_Width(rhs.m_Width), m_Height(rhs.m_Height),
	  m_bFlipX(rhs.m_bFlipX), m_bFlipY(rhs.m_bFlipY)
{
}

void engine::TextureUI::SetTexture(const _wstring& path)
{
	if (m_Path != path)
	{
		D3D11Manager::GetInstance().CreateTexture(path, m_Texture.GetAddressOf());

		ComPtr<ID3D11Resource> resource = nullptr;
		m_Texture->GetResource(resource.GetAddressOf());

		ComPtr<ID3D11Texture2D> texture2D = nullptr;
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
	return m_Texture;
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

void engine::TextureUI::RenderUI()
{
	if (m_Texture)
	{
		_matrix worldMat = XMMatrixMultiply(XMLoadFloat4x4(&m_TextureScaleMatrix), GetTransform()->GetWorldMatrix());

	}
}

void engine::TextureUI::Destroy()
{
	m_bDestroyed = true;
}

void engine::TextureUI::to_json(nlohmann::ordered_json& j)
{
	std::string type = "TextureUI";
	j = nlohmann::ordered_json{
		{"type", type},
		{"enable", m_bEnabled},
		{"path", m_Path},
		{"flipX", m_bFlipX},
		{"flipY", m_bFlipY}
	};
}

void engine::TextureUI::from_json(const nlohmann::ordered_json& j)
{
	if (j.contains("enable"))
	{
		j.at("enable").get_to(m_bEnabled);
	}
	if (j.contains("path"))
	{
		j.at("path").get_to(m_Path);
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
