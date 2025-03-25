#include "Material.h"

#include "D3D11Manager.h"

engine::Material::Material(const SharedPtr<Object>& owner, const _string& name)
	: Object(name), m_Owner(owner)
{

}

engine::Material::~Material() = default;

engine::Material::Material(const Material& rhs)
	: Object(rhs), m_ShaderPath(rhs.m_ShaderPath)
{

}

void engine::Material::SetFloat(const std::string& name, const _float value)
{
	for (_uint i = 0; i < ShaderTypeEnd; ++i)
	{
		for (auto& pair : m_Shader->Reflects[i].CBuffers)
		{
			auto& cbDesc = pair.second;
			auto varIt = cbDesc.Variables.find(name);

			if (varIt != cbDesc.Variables.end())
			{
				auto& varInfo = varIt->second;
				auto& cbr = m_Shader->CBuffers[i][cbDesc.Name];
				auto& dataVec = cbr->LocalData;

				if (varInfo.Size != sizeof(_float))
				{
					std::cerr << "ERROR : Type mismatch in SetValue. (Class Material) " <<
						"Property : " << name.c_str() << ", Method : SetFloat() \n";

					return;
				}

				if (dataVec.size() < cbDesc.BufferSize)
				{
					dataVec.resize(cbDesc.BufferSize, 0);
				}

				memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float));

				cbr->DirtyFlag = true;

				break;
			}
		}
	}
}

engine::_float engine::Material::GetFloat(const std::string& name)
{
	for (_uint i = 0; i < ShaderTypeEnd; ++i)
	{
		for (const auto& pair : m_Shader->Reflects[i].CBuffers)
		{
			const auto& cbDesc = pair.second;
			auto varIt = cbDesc.Variables.find(name);

			if (varIt != cbDesc.Variables.end())
			{
				const auto& varInfo = varIt->second;

				const auto& cbr = m_Shader->CBuffers[i].at(cbDesc.Name);
				const auto& dataVec = cbr->LocalData;

				if (varInfo.Size != sizeof(_float))
				{
					std::cerr << "ERROR : Property " << name.c_str() << " is not Float. (Class Material) \n";

					return 0.0f;
				}

				_float value = 0.0f;
				memcpy(&value, dataVec.data() + varInfo.StartOffset, sizeof(_float));
				return value;
			}
		}
	}

	return 0.f;
}

void engine::Material::SetFloat2(const std::string& name, const _float2 value)
{
	for (_uint i = 0; i < ShaderTypeEnd; ++i)
	{
		for (auto& pair : m_Shader->Reflects[i].CBuffers)
		{
			auto& cbDesc = pair.second;
			auto varIt = cbDesc.Variables.find(name);

			if (varIt != cbDesc.Variables.end())
			{
				auto& varInfo = varIt->second;
				auto& cbr = m_Shader->CBuffers[i][cbDesc.Name];
				auto& dataVec = cbr->LocalData;

				if (varInfo.Size != sizeof(_float2))
				{
					std::cerr << "ERROR : Type mismatch in SetValue. (Class Material) " <<
						"Property : " << name.c_str() << ", Method : SetFloat2() \n";

					return;
				}

				if (dataVec.size() < cbDesc.BufferSize)
				{
					dataVec.resize(cbDesc.BufferSize, 0);
				}

				memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float2));

				cbr->DirtyFlag = true;

				break;
			}
		}
	}
}

engine::_float2 engine::Material::GetFloat2(const std::string& name)
{
	for (_uint i = 0; i < ShaderTypeEnd; ++i)
	{
		for (const auto& pair : m_Shader->Reflects[i].CBuffers)
		{
			const auto& cbDesc = pair.second;
			auto varIt = cbDesc.Variables.find(name);
			if (varIt != cbDesc.Variables.end())
			{
				const auto& varInfo = varIt->second;
				const auto& cbr = m_Shader->CBuffers[i].at(cbDesc.Name);
				const auto& dataVec = cbr->LocalData;

				if (varInfo.Size != sizeof(_float2))
				{
					std::cerr << "ERROR : Property " << name.c_str() << " is not Float2. (Class Material) \n";

					return { 0.0f, 0.0f };
				}

				_float2 value;
				memcpy(&value, dataVec.data() + varInfo.StartOffset, sizeof(_float2));
				return value;
			}
		}
	}

	return {0.0f, 0.0f};
}

void engine::Material::SetFloat3(const std::string& name, const _float3 value)
{
	for (_uint i = 0; i < ShaderTypeEnd; ++i)
	{
		for (auto& pair : m_Shader->Reflects[i].CBuffers)
		{
			auto& cbDesc = pair.second;
			auto varIt = cbDesc.Variables.find(name);

			if (varIt != cbDesc.Variables.end())
			{
				auto& varInfo = varIt->second;
				auto& cbr = m_Shader->CBuffers[i][cbDesc.Name];
				auto& dataVec = cbr->LocalData;

				if (varInfo.Size != sizeof(_float3))
				{
					std::cerr << "ERROR : Type mismatch in SetValue. (Class Material) " <<
						"Property : " << name.c_str() << ", Method : SetFloat3() \n";

					return;
				}

				if (dataVec.size() < cbDesc.BufferSize)
				{
					dataVec.resize(cbDesc.BufferSize, 0);
				}

				memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float3));

				cbr->DirtyFlag = true;

				break;
			}
		}
	}
}

engine::_float3 engine::Material::GetFloat3(const std::string& name)
{
	for (_uint i = 0; i < ShaderTypeEnd; ++i)
	{
		for (const auto& pair : m_Shader->Reflects[i].CBuffers)
		{
			const auto& cbDesc = pair.second;
			auto varIt = cbDesc.Variables.find(name);
			if (varIt != cbDesc.Variables.end())
			{
				const auto& varInfo = varIt->second;
				const auto& cbr = m_Shader->CBuffers[i].at(cbDesc.Name);
				const auto& dataVec = cbr->LocalData;

				if (varInfo.Size != sizeof(_float3))
				{
					std::cerr << "ERROR : Property " << name.c_str() << " is not Float3. (Class Material) \n";

					return { 0.0f, 0.0f, 0.0f };
				}

				_float3 value;
				memcpy(&value, dataVec.data() + varInfo.StartOffset, sizeof(_float3));
				return value;
			}
		}
	}

	return {0.0f, 0.0f, 0.0f};
}

void engine::Material::SetFloat4(const std::string& name, const _float4 value)
{
	for (_uint i = 0; i < ShaderTypeEnd; ++i)
	{
		for (auto& pair : m_Shader->Reflects[i].CBuffers)
		{
			auto& cbDesc = pair.second;
			auto varIt = cbDesc.Variables.find(name);

			if (varIt != cbDesc.Variables.end())
			{
				auto& varInfo = varIt->second;
				auto& cbr = m_Shader->CBuffers[i][cbDesc.Name];
				auto& dataVec = cbr->LocalData;

				if (varInfo.Size != sizeof(_float4))
				{
					std::cerr << "ERROR : Type mismatch in SetValue. (Class Material) " <<
						"Property : " << name.c_str() << ", Method : SetFloat4() \n";

					return;
				}

				if (dataVec.size() < cbDesc.BufferSize)
				{
					dataVec.resize(cbDesc.BufferSize, 0);
				}

				memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float4));

				cbr->DirtyFlag = true;

				break;
			}
		}
	}
}

engine::_float4 engine::Material::GetFloat4(const std::string& name)
{
	for (_uint i = 0; i < ShaderTypeEnd; ++i)
	{
		for (const auto& pair : m_Shader->Reflects[i].CBuffers)
		{
			const auto& cbDesc = pair.second;
			auto varIt = cbDesc.Variables.find(name);
			if (varIt != cbDesc.Variables.end())
			{
				const auto& varInfo = varIt->second;
				const auto& cbr = m_Shader->CBuffers[i].at(cbDesc.Name);
				const auto& dataVec = cbr->LocalData;

				if (varInfo.Size != sizeof(_float4))
				{
					std::cerr << "ERROR : Property " << name.c_str() << " is not Float4. (Class Material) \n";

					return { 0.0f, 0.0f, 0.0f, 0.0f };
				}

				_float4 value;
				memcpy(&value, dataVec.data() + varInfo.StartOffset, sizeof(_float4));
				return value;
			}
		}
	}

	return {0.0f, 0.0f, 0.0f, 0.0f};
}

void engine::Material::SetMatrix(const std::string& name, const _float4X4& value)
{
	for (_uint i = 0; i < ShaderTypeEnd; ++i)
	{
		for (auto& pair : m_Shader->Reflects[i].CBuffers)
		{
			auto& cbDesc = pair.second;
			auto varIt = cbDesc.Variables.find(name);

			if (varIt != cbDesc.Variables.end())
			{
				auto& varInfo = varIt->second;
				auto& cbr = m_Shader->CBuffers[i][cbDesc.Name];
				auto& dataVec = cbr->LocalData;

				if (varInfo.Size != sizeof(_float4X4))
				{
					std::cerr << "ERROR : Type mismatch in SetValue. (Class Material) " <<
						"Property : " << name.c_str() << ", Method : SetFloat4X4() \n";

					return;
				}


				if (dataVec.size() < cbDesc.BufferSize)
				{
					dataVec.resize(cbDesc.BufferSize, 0);
				}

				memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float4X4));

				cbr->DirtyFlag = true;

				break;
			}
		}
	}
}

engine::_float4X4 engine::Material::GetMatrix(const std::string& name)
{
	_float4X4 mat;
	return mat;
}

void engine::Material::SetColor(const _string& name, const _float4 value)
{
	for (_uint i = 0; i < ShaderTypeEnd; ++i)
	{
		for (auto& pair : m_Shader->Reflects[i].CBuffers)
		{
			auto& cbDesc = pair.second;
			auto varIt = cbDesc.Variables.find(name);

			if (varIt != cbDesc.Variables.end())
			{
				auto& varInfo = varIt->second;
				auto& cbr = m_Shader->CBuffers[i][cbDesc.Name];
				auto& dataVec = cbr->LocalData;

				if (dataVec.size() < cbDesc.BufferSize)
				{
					dataVec.resize(cbDesc.BufferSize, 0);
				}

				memcpy(dataVec.data() + varInfo.StartOffset, &value, sizeof(_float4));

				cbr->DirtyFlag = true;

				break;
			}
		}
	}
}

engine::_float4 engine::Material::GetColor(const _string& name)
{
	return GetFloat4(name);
}

void engine::Material::SetTexture(const _string& name, const ComPtr<ID3D11ShaderResourceView>& texture)
{
	for (_uint i = 0; i < ShaderTypeEnd; ++i)
	{
		auto& texMap = m_Shader->Reflects[i].Textures;
		auto texIt = texMap.find(name);
		if (texIt != texMap.end())
		{
			SharedPtr<TextureRuntime> textureRuntime = std::make_shared<TextureRuntime>();

			textureRuntime->Texture = texture;
			textureRuntime->BindPoint = texIt->second.BindPoint;

			m_Shader->Textures[i][name] = textureRuntime;

			break;
		}
	}
}

void engine::Material::SetTexture(const _string& name, const _wstring& path)
{
	for (_uint i = 0; i < ShaderTypeEnd; ++i)
	{
		auto& texMap = m_Shader->Reflects[i].Textures;
		auto texIt = texMap.find(name);
		if (texIt != texMap.end())
		{
			SharedPtr<TextureRuntime> textureRuntime = std::make_shared<TextureRuntime>();

			ComPtr<ID3D11ShaderResourceView> texture;

			D3D11Manager::GetInstance().CreateTexture(path, texture);

			textureRuntime->Texture = texture;
			textureRuntime->TexturePath = path;
			textureRuntime->BindPoint = texIt->second.BindPoint;

			m_Shader->Textures[i][name] = textureRuntime;

			break;
		}
	}
}

void engine::Material::SetSampler(const _string& name, const ComPtr<ID3D11SamplerState>& sampler)
{
	for (_uint i = 0; i < ShaderTypeEnd; ++i)
	{
		auto& texMap = m_Shader->Reflects[i].Samplers;
		auto texIt = texMap.find(name);
		if (texIt != texMap.end())
		{
			SharedPtr<SamplerRuntime> samplerRuntime(new SamplerRuntime);

			samplerRuntime->Sampler = sampler;
			samplerRuntime->BindPoint = texIt->second.BindPoint;

			m_Shader->Samplers[i][name] = samplerRuntime;

			break;
		}
	}
}

void engine::Material::LoadShader(const _wstring& path)
{
	if (FAILED(D3D11Manager::GetInstance().CreateShader(path, m_Shader)))
	{
		std::cerr << "Failed to create Shader : " << WStringToString(path).c_str() << "\n";
		m_ShaderPath.clear();

		return;
	}

	m_ShaderPath = path;
}

void engine::Material::Bind(ID3D11DeviceContext* context) const
{
	if (m_Shader)
	{
		m_Shader->Bind(context);
	}
}

engine::SharedPtr<engine::Material> engine::Material::Create(const SharedPtr<Object>& renderer)
{
	return {
		new Material(renderer),
		[](const Material* ptr) { delete ptr; }
	};
}

engine::SharedPtr<engine::Material> engine::Material::Clone(const SharedPtr<Object>& renderer) const
{
	SharedPtr<Material> clone(CLONE_SHARED_PTR(Material));
	clone->SetOwner(renderer);
	clone->LoadShader(clone->m_ShaderPath);

	return clone;
}

void engine::Material::Destroy()
{
}

void engine::Material::to_json(nlohmann::ordered_json& j)
{
	j = nlohmann::ordered_json{
		{"shaderPath", m_ShaderPath},
		{"textureMaps", nlohmann::ordered_json::array() },
		{"properties", nlohmann::ordered_json::array()}
	};

	for (auto& reflectResult : m_Shader->Reflects)
	{
		for (auto& textureInfo : reflectResult.Textures)
		{
			std::wstring texturePath;

			for (auto& textureRuntime : m_Shader->Textures)
			{
				auto it = textureRuntime.find(textureInfo.first);

				if (it != textureRuntime.end())
				{
					texturePath = it->second->TexturePath;
					nlohmann::ordered_json textureJson = nlohmann::ordered_json{
						{textureInfo.first.c_str(), texturePath}
					};

					j["textureMaps"].push_back(textureJson);

					break;
				}
			}
		}

		// TODO : 유니티처럼 내용 직렬화 해서 보관 해야됨!!!
		// 하다 말았음.

		for (auto& cbufferDesc : reflectResult.CBuffers)
		{
			if (cbufferDesc.first == "Property")
			{
				for (auto& var : cbufferDesc.second.Variables)
				{
					if (var.second.Size == sizeof(_float))
					{
						_float value = GetFloat(var.first);

						nlohmann::ordered_json propertyJson = nlohmann::ordered_json{
							{var.first.c_str(), value}
						};
					}

					else if (var.second.Size == sizeof(_float2))
					{
						_float2 value = GetFloat2(var.first);
					}

					else if (var.second.Size == sizeof(_float3))
					{

					}

					else if (var.second.Size == sizeof(_float4))
					{

					}
				}
			}
		}
	}
}

void engine::Material::from_json(const nlohmann::ordered_json& j)
{

}
