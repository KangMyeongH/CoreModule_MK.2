#include "D3D11Manager.h"

#include <iostream>
#include <sstream>

IMPLEMENT_SINGLETON(engine::D3D11Manager)

engine::D3D11Manager::D3D11Manager()
	: m_WinSizeX(0), m_WinSizeY(0)
{
}

engine::D3D11Manager::~D3D11Manager()
{

}

engine::SharedPtr<engine::VIBuffer> engine::D3D11Manager::GetVIBuffer(const VIBufferType type)
{
	auto viBufferIt = m_VIBufferMap.find(type);

	if (viBufferIt != m_VIBufferMap.end())
	{
		return viBufferIt->second;
	}

	return nullptr;
}

HRESULT engine::D3D11Manager::Initialize(HWND hwnd, _bool isWindowed, _uint winSizeX, _uint winSizeY)
{
	_uint	flag = 0;

#ifdef _DEBUG
	flag = D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL		featureLV;

	if (FAILED(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flag,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		m_Device.GetAddressOf(),
		&featureLV,
		m_DeviceContext.GetAddressOf())))
	{
		return E_FAIL;
	}

	if (FAILED(readySwapChain(hwnd, isWindowed, winSizeX, winSizeY)))
	{
		return E_FAIL;
	}

	if (FAILED(readyBackBufferRenderTargetView()))
	{
		return E_FAIL;
	}

	if (FAILED(readyDepthStencilView(winSizeX, winSizeY)))
	{
		return E_FAIL;
	}

	ID3D11RenderTargetView* RTVs[] =
	{
		m_BackBufferRTV.Get()
	};

	m_DeviceContext->OMSetRenderTargets(0, RTVs, m_DepthStencilView.Get());

	D3D11_VIEWPORT		viewPortDesc;
	ZeroMemory(&viewPortDesc, sizeof(D3D11_VIEWPORT));
	viewPortDesc.TopLeftX = 0;
	viewPortDesc.TopLeftY = 0;
	viewPortDesc.Width = static_cast<_float>(winSizeX);
	viewPortDesc.Height = static_cast<_float>(winSizeY);
	viewPortDesc.MinDepth = 0.f;
	viewPortDesc.MaxDepth = 1.f;

	m_DeviceContext->RSSetViewports(1, &viewPortDesc);

	m_WinSizeX = winSizeX;
	m_WinSizeY = winSizeY;

	if (FAILED(readyVIBuffers()))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT engine::D3D11Manager::CreateTexture(const _wstring& path, ComPtr<ID3D11ShaderResourceView>& srv)
{
	if (path.empty() || !FileExists(path))
	{
		return E_FAIL;
	}

	ComPtr<ID3D11ShaderResourceView> pSRV;

	const auto it = m_TextureMap.find(path);

	if (it != m_TextureMap.end())
	{
		srv = it->second;

		return S_OK;
	}

	const _wchar* textureFilePath = path.c_str();
	_wstring ext = path.substr(path.find_last_of(L"."));
	std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

	HRESULT hr;

	if (ext == L".dds")
	{
		hr = DirectX::CreateDDSTextureFromFile(m_Device.Get(), textureFilePath, nullptr, pSRV.GetAddressOf());
	}

	else if (ext == L".tga")
	{
		hr = E_FAIL;
	}

	else
	{
		hr = DirectX::CreateWICTextureFromFile(m_Device.Get(), textureFilePath, nullptr, pSRV.GetAddressOf());
	}

	if (FAILED(hr))
	{
		return hr;
	}

	srv = pSRV;

	m_TextureMap.emplace(path, pSRV);

	return S_OK;
}

HRESULT engine::D3D11Manager::CreateShader(const _wstring& path, SharedPtr<Shader>& shader)
{
	if (FileExists(path))
	{
		const auto it = m_ShaderMap.find(path);

		if (it != m_ShaderMap.end())
		{
			shader = it->second->Clone(m_Device.Get());

			return S_OK;
		}

		SharedPtr<Shader> newShader = std::make_shared<Shader>();

		newShader->Path = path;

		std::vector<std::pair<std::string, std::string>> entryPoints = {
		{ "VS_Main", "vs_5_0" },
		{ "PS_Main", "ps_5_0" },
		{ "GS_Main", "gs_5_0" },
		{ "CS_Main", "cs_5_0" },
		{ "HS_Main", "hs_5_0" },
		{ "DS_Main", "ds_5_0" }
		};

		for (auto& ep : entryPoints)
		{
			const _string& entryName = ep.first;
			const _string& shaderModel = ep.second;

			ComPtr<ID3DBlob> shaderBlob;
			HRESULT hr = compileShaderFromFile(path, entryName, shaderModel, shaderBlob);

			if (SUCCEEDED(hr))
			{
				ComPtr<ID3D11ShaderReflection> reflector;
				D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, reinterpret_cast<void**>(reflector.GetAddressOf()));

				if (shaderModel.find("vs_") == 0)
				{
					ComPtr<ID3D11VertexShader> vs;
					hr = m_Device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, vs.GetAddressOf());

					if (SUCCEEDED(hr))
					{
						newShader->VertexShader = vs;

						ComPtr<ID3D11InputLayout> input;
						std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;

						compileInputLayoutFromReflector(&inputLayoutDesc, reflector);

						hr = m_Device->CreateInputLayout(&inputLayoutDesc[0], static_cast<UINT>(inputLayoutDesc.size()), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), input.GetAddressOf());

						if (SUCCEEDED(hr))
						{
							newShader->InputLayout = input;
						}

						reflectBufferFromReflector(reflector, newShader->Reflects[VS]);
						//createConstantBuffer(newShader->Reflects[VS], newShader->CBuffers[VS]);
					}
				}

				else if (shaderModel.find("ps_") == 0)
				{
					ComPtr<ID3D11PixelShader> ps;
					hr = m_Device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, ps.GetAddressOf());

					if (SUCCEEDED(hr))
					{
						newShader->PixelShader = ps;
						reflectBufferFromReflector(reflector, newShader->Reflects[PS]);
						//createConstantBuffer(newShader->Reflects[PS], newShader->CBuffers[PS]);
					}
				}

				else if (shaderModel.find("gs_") == 0)
				{
					ComPtr<ID3D11GeometryShader> gs;
					hr = m_Device->CreateGeometryShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, gs.GetAddressOf());

					if (SUCCEEDED(hr))
					{
						newShader->GeometryShader = gs;
						reflectBufferFromReflector(reflector, newShader->Reflects[GS]);
						//createConstantBuffer(newShader->Reflects[GS], newShader->CBuffers[GS]);
					}
				}

				else if (shaderModel.find("cs_") == 0)
				{
					ComPtr<ID3D11ComputeShader> cs;
					hr = m_Device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, cs.GetAddressOf());

					if (SUCCEEDED(hr))
					{
						newShader->ComputeShader = cs;
						reflectBufferFromReflector(reflector, newShader->Reflects[CS]);
						//createConstantBuffer(newShader->Reflects[CS], newShader->CBuffers[CS]);
					}
				}

				else if (shaderModel.find("hs_") == 0)
				{
					ComPtr<ID3D11HullShader> hs;
					hr = m_Device->CreateHullShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, hs.GetAddressOf());

					if (SUCCEEDED(hr))
					{
						newShader->HullShader = hs;
						reflectBufferFromReflector(reflector, newShader->Reflects[HS]);
						//createConstantBuffer(newShader->Reflects[HS], newShader->CBuffers[HS]);
					}
				}

				else if (shaderModel.find("ds_") == 0)
				{
					ComPtr<ID3D11DomainShader> ds;
					hr = m_Device->CreateDomainShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, ds.GetAddressOf());

					if (SUCCEEDED(hr))
					{
						newShader->DomainShader = ds;
						reflectBufferFromReflector(reflector, newShader->Reflects[DS]);
						//createConstantBuffer(newShader->Reflects[DS], newShader->CBuffers[DS]);
					}
				}
			}
		}

		m_ShaderMap.emplace(path, newShader);
		shader = newShader->Clone(m_Device.Get());

		return S_OK;
	}

	return E_FAIL;
}

HRESULT engine::D3D11Manager::CreateSampler(const D3D11_SAMPLER_DESC& desc, ComPtr<ID3D11SamplerState>& sampler) const
{
	HRESULT hr = m_Device->CreateSamplerState(&desc, sampler.GetAddressOf());

	if (FAILED(hr))
	{
		std::cerr << "Failed create Sampler! \n";
		return E_FAIL;
	}

	return S_OK;
}

void engine::D3D11Manager::Release()
{
	m_TextureMap.clear();
	m_VIBufferMap.clear();
	m_ShaderMap.clear();
}

HRESULT engine::D3D11Manager::readySwapChain(const HWND hWnd, const _bool isWindowed, const _uint winSizeX, const _uint winSizeY)
{
	IDXGIDevice* device = nullptr;
	if (FAILED(m_Device->QueryInterface(
		__uuidof(IDXGIDevice), reinterpret_cast<void**>(&device))))
	{
		return E_FAIL;
	}

	IDXGIAdapter* adapter = nullptr;
	if (FAILED(device->GetParent(
		__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&adapter))))
	{
		return E_FAIL;
	}

	IDXGIFactory* factory = nullptr;
	if (FAILED(adapter->GetParent(
		__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory))))
	{
		return E_FAIL;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc.Width = winSizeX;
	swapChainDesc.BufferDesc.Height = winSizeY;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;

	swapChainDesc.BufferDesc.RefreshRate.Numerator = 144;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = isWindowed;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (FAILED(factory->CreateSwapChain(m_Device.Get(), &swapChainDesc, m_SwapChain.GetAddressOf())))
	{
		return E_FAIL;
	}

	SafeRelease(factory);
	SafeRelease(adapter);
	SafeRelease(device);

	return S_OK;
}

HRESULT engine::D3D11Manager::readyBackBufferRenderTargetView()
{
	if (nullptr == m_Device)
	{
		return E_FAIL;
	}

	ID3D11Texture2D* backBufferTexture = nullptr;

	if (FAILED(m_SwapChain->GetBuffer(
		0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBufferTexture))))
	{
		return E_FAIL;
	}

	if (FAILED(m_Device->CreateRenderTargetView(
		backBufferTexture, nullptr, m_BackBufferRTV.GetAddressOf())))
	{
		return E_FAIL;
	}

	SafeRelease(backBufferTexture);

	return S_OK;
}

HRESULT engine::D3D11Manager::readyDepthStencilView(const _uint winSizeX, const _uint winSizeY)
{
	if (nullptr == m_Device)
	{
		return E_FAIL;
	}

	ID3D11Texture2D* depthStencilTexture = nullptr;

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	textureDesc.Width = winSizeX;
	textureDesc.Height = winSizeY;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	textureDesc.SampleDesc.Quality = 0;
	textureDesc.SampleDesc.Count = 1;

	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	if (FAILED(m_Device->CreateTexture2D(
		&textureDesc, nullptr, &depthStencilTexture)))
	{
		return E_FAIL;
	}

	if (FAILED(m_Device->CreateDepthStencilView(
		depthStencilTexture, nullptr, m_DepthStencilView.GetAddressOf())))
	{
		return E_FAIL;
	}

	SafeRelease(depthStencilTexture);

	return S_OK;
}

HRESULT engine::D3D11Manager::compileShaderFromFile(const _wstring& path, const _string& entryPoint, const _string& targetProfile, ComPtr<ID3DBlob>& outBlob)
{
	ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), targetProfile.c_str(), 0, 0, outBlob.GetAddressOf(), errorBlob.GetAddressOf());

	if (FAILED(hr))
	{
		std::stringstream ss;
		ss << "Failed to compile shader from file : " << WStringToString(path) << "\n";
		std::cerr << ss.str().c_str();

		if (errorBlob)
		{
			std::cerr << static_cast<const char*>(errorBlob->GetBufferPointer());
		}

		//std::wstringstream wss;
		//wss << L"Failed to compile shader from file\n File path : " << path << L"\n";

		//if (errorBlob)
		//{
		//	std::stringstream ss;
		//	ss << "Shader Compilation Error : " << static_cast<const char*>(errorBlob->GetBufferPointer());

		//	_string s = ss.str();
		//	wss << StringToWString(s);
		//}

		//_wstring message = wss.str();

		//MessageBoxW(nullptr, message.c_str(), L"Shader Error", MB_OK);


		return hr;
	}

	return hr;
}

void engine::D3D11Manager::compileInputLayoutFromReflector(std::vector<D3D11_INPUT_ELEMENT_DESC>* inputDesc,
                                                              const ComPtr<ID3D11ShaderReflection>& reflector)
{
	D3D11_SHADER_DESC shaderDesc;
	reflector->GetDesc(&shaderDesc);

	for (unsigned inputIndex = 0; inputIndex < shaderDesc.InputParameters; inputIndex++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		reflector->GetInputParameterDesc(inputIndex, &paramDesc);

		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			}
		}

		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			}
		}

		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
		}

		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			}
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
		}

		inputDesc->push_back(elementDesc);
	}
}

void engine::D3D11Manager::reflectBufferFromReflector(const ComPtr<ID3D11ShaderReflection>& reflector, ReflectResult& outResult)
{
	D3D11_SHADER_DESC shaderDesc;
	reflector->GetDesc(&shaderDesc);

	for (_uint i = 0; i < shaderDesc.BoundResources; ++i)
	{
		D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		reflector->GetResourceBindingDesc(i, &bindDesc);

		// bindDesc.Name		: 리소스 이름
		// bindDesc.BindPoint	: register 번호
		// bindDesc.Type		: D3D_SIT_CBUFFER, D3D_SIT_TEXTURE, D3D_SIT_SAMPLER 등

		if (bindDesc.Type == D3D_SIT_CBUFFER)
		{
			ConstantBufferDesc cbDest;
			cbDest.Name = bindDesc.Name;
			cbDest.BindPoint = bindDesc.BindPoint;

			ID3D11ShaderReflectionConstantBuffer* cbuf = reflector->GetConstantBufferByName(bindDesc.Name);
			
			if (cbuf)
			{
				D3D11_SHADER_BUFFER_DESC desc;
				cbuf->GetDesc(&desc);

				cbDest.BufferSize = desc.Size;

				for (_uint varIndex = 0; varIndex < desc.Variables; ++varIndex)
				{
					ID3D11ShaderReflectionVariable* varRef = cbuf->GetVariableByIndex(varIndex);
					D3D11_SHADER_VARIABLE_DESC varDesc;
					varRef->GetDesc(&varDesc);

					ShaderVarDesc varInfo;
					varInfo.Name = varDesc.Name;      // 예: "_Color", "_Metallic"
					varInfo.StartOffset = varDesc.StartOffset;
					varInfo.Size = varDesc.Size;

					cbDest.Variables[varInfo.Name] = varInfo;
				}
			}

			outResult.CBuffers[cbDest.Name] = cbDest;
		}

		else if (bindDesc.Type == D3D_SIT_TEXTURE)
		{
			TextureInfo tInfo;
			tInfo.Name = bindDesc.Name;
			tInfo.BindPoint = bindDesc.BindPoint;

			outResult.Textures[tInfo.Name] = tInfo;
		}

		else if (bindDesc.Type == D3D_SIT_SAMPLER)
		{
			SamplerInfo sInfo;
			sInfo.Name = bindDesc.Name;
			sInfo.BindPoint = bindDesc.BindPoint;
			outResult.Samplers[sInfo.Name] = sInfo;
		}
	}
}

bool engine::D3D11Manager::createConstantBuffer(const ReflectResult& reflectResult, std::unordered_map<_string, SharedPtr<CBufferRuntime>>& outResult)
{
	outResult.reserve(reflectResult.CBuffers.size());

	for (auto& pair : reflectResult.CBuffers)
	{
		const auto& cbName = pair.first;
		const auto& cbDesc = pair.second;

		SharedPtr<CBufferRuntime> cBufferRuntime = std::make_shared<CBufferRuntime>();

		cBufferRuntime->BindPoint = cbDesc.BindPoint;
		cBufferRuntime->Size = cbDesc.BufferSize;
		cBufferRuntime->LocalData.resize(cbDesc.BufferSize, 0);

		D3D11_BUFFER_DESC bd {};
		bd.ByteWidth = cbDesc.BufferSize;
		bd.Usage = D3D11_USAGE_DYNAMIC; // 예: DEFAULT
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;                   // D3D11_USAGE_DYNAMIC이면 D3D11_CPU_ACCESS_WRITE
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;

		HRESULT hr = m_Device->CreateBuffer(&bd, nullptr, cBufferRuntime->Buffer.GetAddressOf());

		if (FAILED(hr))
		{
			std::cerr << "Failed to create Constant Buffer : " << cbName << "\n";
			return false;
		}

		outResult.emplace(cbDesc.Name, cBufferRuntime);
	}

	return true;
}

HRESULT engine::D3D11Manager::readyVIBuffers()
{
	// TODO : 아 모르겠다 그냥 하드코딩해 시부레...
	// 근데 mesh는 객체 마다 다른댜


	//======Create VIBufferType_POSTEX_RECT=======//
#pragma region VIBufferType_POSTEX_RECT
	auto viPosTex = std::make_shared<VIBuffer>();
	viPosTex->NumVertexBuffers = 1;
	viPosTex->VertexStride = sizeof(VTX_TEXTURE_UI);
	viPosTex->NumVertices = 4;
	viPosTex->IndexStride = 2;
	viPosTex->NumIndices = 6;
	viPosTex->IndexFormat = DXGI_FORMAT_R16_UINT;
	viPosTex->PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	D3D11_BUFFER_DESC vbDesc;
	ZeroMemory(&vbDesc, sizeof(vbDesc));
	vbDesc.ByteWidth = viPosTex->VertexStride * viPosTex->NumVertices;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.StructureByteStride = viPosTex->VertexStride;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;

	VTX_TEXTURE_UI* vtxPosTexRect = new VTX_TEXTURE_UI[viPosTex->NumVertices];
	ZeroMemory(vtxPosTexRect, sizeof(VTX_TEXTURE_UI) * viPosTex->NumVertices);

	vtxPosTexRect[0].Position  = _float3(-0.5f, 0.5f, 0.f);
	vtxPosTexRect[0].TexCoord0 = _float2(0.f, 0.f);

	vtxPosTexRect[1].Position  = _float3(0.5f, 0.5f, 0.f);
	vtxPosTexRect[1].TexCoord0 = _float2(1.f, 0.f);

	vtxPosTexRect[2].Position  = _float3(0.5f, -0.5f, 0.f);
	vtxPosTexRect[2].TexCoord0 = _float2(1.f, 1.f);

	vtxPosTexRect[3].Position  = _float3(-0.5f, -0.5f, 0.f);
	vtxPosTexRect[3].TexCoord0 = _float2(0.f, 1.f);

	D3D11_SUBRESOURCE_DATA initDesc;
	ZeroMemory(&initDesc, sizeof(initDesc));
	initDesc.pSysMem = vtxPosTexRect;

	if (FAILED(m_Device->CreateBuffer(&vbDesc, &initDesc, viPosTex->VertexBuffer.GetAddressOf())))
	{
		std::cerr << "Failed to create VBufferType_POSTEX_RECT ! \n";
		return E_FAIL;
	}

	SafeDeleteArray(vtxPosTexRect);

	D3D11_BUFFER_DESC ibDesc;
	ZeroMemory(&ibDesc, sizeof(ibDesc));
	ibDesc.ByteWidth = viPosTex->IndexStride * viPosTex->NumIndices;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.StructureByteStride = viPosTex->IndexStride;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;

	_ushort* idxPosTexRect = new _ushort[viPosTex->NumIndices];
	ZeroMemory(idxPosTexRect, sizeof(_ushort) * viPosTex->NumIndices);

	idxPosTexRect[0] = 0;
	idxPosTexRect[1] = 1;
	idxPosTexRect[2] = 2;

	idxPosTexRect[3] = 0;
	idxPosTexRect[4] = 2;
	idxPosTexRect[5] = 3;

	ZeroMemory(&initDesc, sizeof(initDesc));
	initDesc.pSysMem = idxPosTexRect;

	if (FAILED(m_Device->CreateBuffer(&ibDesc, &initDesc, viPosTex->IndexBuffer.GetAddressOf())))
	{
		std::cerr << "Failed to create IBufferType_POSTEX_RECT ! \n";
		return E_FAIL;
	}

	SafeDeleteArray(idxPosTexRect);

	m_VIBufferMap.emplace(VIBufferType_POSTEX_RECT, viPosTex);
#pragma endregion
	//============================================//




	return S_OK;
}
