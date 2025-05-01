#include "Material.h"

#include "D3D11Manager.h"
#include "LoadManager.h"

engine::Material::Material(const SharedPtr<Object>& owner, const _string& name)
	: Object(name), m_Owner(owner), m_TextureWidth(0), m_TextureHeight(0)
{
}

engine::Material::~Material() = default;

engine::Material::Material(const Material& rhs)
	: Object(rhs), m_ShaderPath(rhs.m_ShaderPath), m_MaterialPath(rhs.m_MaterialPath), m_TextureWidth(rhs.m_TextureWidth),
	  m_TextureHeight(rhs.m_TextureHeight)
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

void engine::Material::SetMatrix(const _string& name, const _matrix& matrix)
{
	_float4X4 mat;
	XMStoreFloat4x4(&mat, matrix);
	SetMatrix(name, mat);
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

void engine::Material::SetValue(const std::vector<_float4X4>& value)
{
	auto& cbr = m_Shader->CBuffers[VS]["Bones"];
	auto& dataVec = cbr->LocalData;

	if (dataVec.size() < sizeof(_float4X4) * value.size())
	{
		dataVec.resize(sizeof(_float4X4) * value.size(), 0);
	}

	memcpy(dataVec.data(), value.data(), sizeof(_float4X4) * value.size());

	cbr->DirtyFlag = true;
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

			ComPtr<ID3D11Resource> resource;
			texture->GetResource(resource.GetAddressOf());

			ComPtr<ID3D11Texture2D> texture2D;
			resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(texture2D.GetAddressOf()));

			D3D11_TEXTURE2D_DESC desc;
			texture2D->GetDesc(&desc);

			m_TextureWidth = desc.Width;
			m_TextureHeight = desc.Height;

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

			ComPtr<ID3D11Resource> resource;
			texture->GetResource(resource.GetAddressOf());

			ComPtr<ID3D11Texture2D> texture2D;
			resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(texture2D.GetAddressOf()));

			D3D11_TEXTURE2D_DESC desc;
			texture2D->GetDesc(&desc);

			m_TextureWidth = desc.Width;
			m_TextureHeight = desc.Height;

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
	LoadManager::GetInstance().LoadMaterialData(clone, m_MaterialPath);

	return clone;
}

void engine::Material::Destroy()
{
}

void engine::Material::to_json(nlohmann::ordered_json& j)
{
	std::string shaderPath = WStringToString(m_ShaderPath);

	j = nlohmann::ordered_json{
		{"name", GetName()},
		{"shaderPath", shaderPath},
		{"textureMaps", nlohmann::ordered_json::array() },
		{"properties", nlohmann::ordered_json::array()}
	};

	for (auto& reflectResult : m_Shader->Reflects)
	{
		for (auto& textureInfo : reflectResult.Textures)
		{
			std::string texturePath;

			for (auto& textureRuntime : m_Shader->Textures)
			{
				auto it = textureRuntime.find(textureInfo.first);

				if (it != textureRuntime.end())
				{
					texturePath = WStringToString(it->second->TexturePath);

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
						_string xName = var.first + "_x";

						nlohmann::ordered_json propertyJson = nlohmann::ordered_json{
							{xName.c_str(), value}
						};

						j["properties"].push_back(propertyJson);
					}

					else if (var.second.Size == sizeof(_float2))
					{
						_float2 value = GetFloat2(var.first);
						_string xName = var.first + "_x";
						_string yName = var.first + "_y";

						nlohmann::ordered_json propertyJson = nlohmann::ordered_json{
							{xName.c_str(), value.x},
							{yName.c_str(), value.y}
						};

						j["properties"].push_back(propertyJson);
					}

					else if (var.second.Size == sizeof(_float3))
					{
						_float3 value = GetFloat3(var.first);
						_string xName = var.first + "_x";
						_string yName = var.first + "_y";
						_string zName = var.first + "_z";

						nlohmann::ordered_json propertyJson = nlohmann::ordered_json{
							{xName.c_str(), value.x},
							{yName.c_str(), value.y},
							{zName.c_str(), value.z}
						};

						j["properties"].push_back(propertyJson);
					}

					else if (var.second.Size == sizeof(_float4))
					{
						_float4 value = GetFloat4(var.first);
						_string xName = var.first + "_x";
						_string yName = var.first + "_y";
						_string zName = var.first + "_z";
						_string wName = var.first + "_w";

						nlohmann::ordered_json propertyJson = nlohmann::ordered_json{
							{xName.c_str(), value.x},
							{yName.c_str(), value.y},
							{zName.c_str(), value.z},
							{wName.c_str(), value.w}
						};

						j["properties"].push_back(propertyJson);
					}
				}
			}
		}
	}
}

void engine::Material::from_json(const nlohmann::ordered_json& j)
{
	SetName(j.at("name").get<_string>());

	_string shaderPath = j.value("shaderPath", "");
	m_ShaderPath = StringToWString(shaderPath);
	LoadShader(m_ShaderPath);

	const auto& textureArray = j["textureMaps"];
	for (const auto& texJson : textureArray)
	{
		for (auto it = texJson.begin(); it != texJson.end(); ++it)
		{
			_string texName = it.key();
			_wstring texPath = StringToWString(it.value());

			SetTexture(texName, texPath);
		}
	}

	std::unordered_map<std::string, std::vector<float>> floatProperties;

	const auto& propertyArray = j["properties"];
	for (const auto& propJson : propertyArray)
	{
		for (auto it = propJson.begin(); it != propJson.end(); ++it)
		{
			const std::string& key = it.key();
			float value = it.value();

			size_t pos = key.rfind('_');
			if (pos == std::string::npos) continue;;

			std::string baseName = key.substr(0, pos);
			std::string suffix = key.substr(pos + 1);

			int index = -1;
			if (suffix == "x")
			{
				index = 0;
			}

			else if (suffix == "y")
			{
				index = 1;
			}

			else if (suffix == "z")
			{
				index = 2;
			}

			else if (suffix == "w")
			{
				index = 3;
			}
			else
				continue;

			auto& vec = floatProperties[baseName];
			if (vec.size() < index + 1)
			{
				vec.resize(index + 1);
			}
			vec[index] = value;
		}
	}

	for (const auto& property : floatProperties)
	{
		auto& name = property.first;
		auto& values = property.second;

		switch (values.size())
		{
		case 1: SetFloat(name, values[0]);
			break;
		case 2: SetFloat2(name, { values[0], values[1] });
			break;
		case 3: SetFloat3(name, { values[0], values[1], values[2] });
			break;
		case 4: SetFloat4(name, { values[0], values[1], values[2], values[3] });
			break;
		default: 
			break;
		}
	}
}
