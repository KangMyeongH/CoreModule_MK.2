#include "Material.h"

#include "D3D11Manager.h"

engine::Material::Material(const SharedPtr<Renderer>& owner)
	: Object("Material"),
	  m_Owner(owner),
	  m_DiffuseTexture(nullptr)
{
}

engine::Material::~Material()
{
	m_Owner.reset();
	SafeRelease(m_DiffuseTexture);
}

engine::Material::Material(const Material& rhs)
	: Object(rhs),
	  m_DiffuseTexture(nullptr)
{
}

void engine::Material::SetDiffuseTexture(const _wstring& path)
{
	SafeRelease(m_DiffuseTexture);

	if (FAILED(D3D11Manager::GetInstance().CreateTexture(path, &m_DiffuseTexture)))
	{
		m_DiffuseTexture = nullptr;
		return;
	}

	m_DiffuseTexture->AddRef();
}

void engine::Material::SetFloat(const std::string& name, const _float value)
{
	for (auto& pair : m_Shader->VSReflect.CBuffers)
	{
		auto& cbDesc = pair.second;
		auto varIt = cbDesc.Variables.find(name);

		if (varIt != cbDesc.Variables.end())
		{
			auto& varInfo = varIt->second;

			auto& dataVec = m_CBufferData[cbDesc.BindPoint];

			if (dataVec.size() < cbDesc.BufferSize)
			{
				dataVec.resize(cbDesc.BufferSize, 0);
			}

			memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float));

			break;
		}
	}

	for (auto& pair : m_Shader->PSReflect.CBuffers)
	{
		auto& cbDesc = pair.second;
		auto varIt = cbDesc.Variables.find(name);

		if (varIt != cbDesc.Variables.end())
		{
			auto& varInfo = varIt->second;

			auto& dataVec = m_CBufferData[cbDesc.BindPoint];

			if (dataVec.size() < cbDesc.BufferSize)
			{
				dataVec.resize(cbDesc.BufferSize, 0);
			}

			memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float));

			break;
		}
	}
}

engine::_float engine::Material::GetFloat(const std::string& name)
{
}

void engine::Material::SetFloat2(const std::string& name, const _float2 value)
{
	for (auto& pair : m_Shader->CBuffers)
	{
		auto& cbDesc = pair.second;
		auto varIt = cbDesc.Variables.find(name);
		if (varIt != cbDesc.Variables.end())
		{
			auto& varInfo = varIt->second;

			auto& dataVec = m_CBufferData[cbDesc.BindPoint];

			if (dataVec.size() < cbDesc.BufferSize)
			{
				dataVec.resize(cbDesc.BufferSize, 0);
			}

			memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float2));

			return;
		}
	}
}

engine::_float2 engine::Material::GetFloat2(const std::string& name)
{

}

void engine::Material::SetFloat3(const std::string& name, const _float3 value)
{
	for (auto& pair : m_Shader->CBuffers)
	{
		auto& cbDesc = pair.second;
		auto varIt = cbDesc.Variables.find(name);
		if (varIt != cbDesc.Variables.end())
		{
			auto& varInfo = varIt->second;

			auto& dataVec = m_CBufferData[cbDesc.BindPoint];

			if (dataVec.size() < cbDesc.BufferSize)
			{
				dataVec.resize(cbDesc.BufferSize, 0);
			}

			memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float3));

			return;
		}
	}
}

engine::_float3 engine::Material::GetFloat3(const std::string& name)
{

}

void engine::Material::SetFloat4(const std::string& name, const _float4 value)
{
	for (auto& pair : m_Shader->CBuffers)
	{
		auto& cbDesc = pair.second;
		auto varIt = cbDesc.Variables.find(name);
		if (varIt != cbDesc.Variables.end())
		{
			auto& varInfo = varIt->second;

			auto& dataVec = m_CBufferData[cbDesc.BindPoint];

			if (dataVec.size() < cbDesc.BufferSize)
			{
				dataVec.resize(cbDesc.BufferSize, 0);
			}

			memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float4));

			return;
		}
	}
}

engine::_float4 engine::Material::GetFloat4(const std::string& name)
{

}

void engine::Material::SetMatrix(const std::string& name, const _float4X4& value)
{
	for (auto& pair : m_Shader->CBuffers)
	{
		auto& cbDesc = pair.second;
		auto varIt = cbDesc.Variables.find(name);
		if (varIt != cbDesc.Variables.end())
		{
			auto& varInfo = varIt->second;

			auto& dataVec = m_CBufferData[cbDesc.BindPoint];

			if (dataVec.size() < cbDesc.BufferSize)
			{
				dataVec.resize(cbDesc.BufferSize, 0);
			}

			memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float4X4));

			return;
		}
	}
}

engine::_float4X4 engine::Material::GetMatrix(const std::string& name)
{

}

void engine::Material::LoadShader(const _wstring& path)
{

}

void engine::Material::Bind(ID3D11DeviceContext* context) const
{
	if (m_Shader)
	{
		m_Shader->Bind(context);
	}
}

engine::SharedPtr<engine::Material> engine::Material::Create(const SharedPtr<Renderer>& renderer)
{
	return {
		new Material(renderer),
		[](const Material* ptr) { delete ptr; }
	};
}

void engine::Material::Destroy()
{
	if (auto owner = m_Owner.lock())
	{
		
	}
}
