#include "D3D11Manager.h"

IMPLEMENT_SINGLETON(engine::D3D11Manager)

engine::D3D11Manager::D3D11Manager()
	: m_Device(nullptr),
	m_DeviceContext(nullptr),
	m_SwapChain(nullptr),
	m_BackBufferRTV(nullptr),
	m_DepthStencilView(nullptr)
{
}

engine::D3D11Manager::~D3D11Manager()
{
	Release();
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

	return S_OK;
}

HRESULT engine::D3D11Manager::CreateTexture(const _wstring& path, ID3D11ShaderResourceView** srv)
{
	if (!srv)
	{
		return E_POINTER;
	}

	if (!path.empty())
	{
		return E_FAIL;
	}

	ComPtr<ID3D11ShaderResourceView> pSRV;

	if (m_TextureMap.find(path) != m_TextureMap.end())
	{
		pSRV = m_TextureMap[path];
		*srv = pSRV.Get();

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

	*srv = pSRV.Get();

	m_TextureMap.emplace(path, pSRV);

	return S_OK;
}

HRESULT engine::D3D11Manager::CreateShader(const _wstring& path, const SharedPtr<Shader>& shader)
{
	if (FileExists(path))
	{
		std::vector<std::pair<std::string, std::string>> entryPoints = {
		{ "VSMain", "vs_5_0" },
		{ "PSMain", "ps_5_0" },
		{ "GSMain", "gs_5_0" },
		{ "CSMain", "cs_5_0" },
		{ "HSMain", "hs_5_0" },
		{ "DSMain", "ds_5_0" }
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
						shader->VertexShader = vs;

						ComPtr<ID3D11InputLayout> input;
						std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;

						compileInputLayoutFromReflector(&inputLayoutDesc, reflector);

						hr = m_Device->CreateInputLayout(&inputLayoutDesc[0], static_cast<UINT>(inputLayoutDesc.size()), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), input.GetAddressOf());

						if (SUCCEEDED(hr))
						{
							shader->InputLayout = input;
						}
					}
				}

				else if (shaderModel.find("ps_") == 0)
				{
					ComPtr<ID3D11PixelShader> ps;
					hr = m_Device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, ps.GetAddressOf());

					if (SUCCEEDED(hr))
					{
						shader->PixelShader = ps;
					}
				}

				else if (shaderModel.find("gs_") == 0)
				{
					ComPtr<ID3D11GeometryShader> gs;
					hr = m_Device->CreateGeometryShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, gs.GetAddressOf());

					if (SUCCEEDED(hr))
					{
						shader->GeometryShader = gs;
					}
				}

				else if (shaderModel.find("cs_") == 0)
				{
					ComPtr<ID3D11ComputeShader> cs;
					hr = m_Device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, cs.GetAddressOf());

					if (SUCCEEDED(hr))
					{
						shader->ComputeShader = cs;
					}
				}

				else if (shaderModel.find("hs_") == 0)
				{
					ComPtr<ID3D11HullShader> hs;
					hr = m_Device->CreateHullShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, hs.GetAddressOf());

					if (SUCCEEDED(hr))
					{
						shader->HullShader = hs;
					}
				}

				else if (shaderModel.find("ds_") == 0)
				{
					ComPtr<ID3D11DomainShader> ds;
					hr = m_Device->CreateDomainShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, ds.GetAddressOf());

					if (SUCCEEDED(hr))
					{
						shader->DomainShader = ds;
					}
				}
			}
		}
	}

	return E_FAIL;
}

void engine::D3D11Manager::Release()
{
	m_TextureMap.clear();
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

	if (FAILED(factory->CreateSwapChain(m_Device.Get(), &swapChainDesc, &m_SwapChain)))
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
		backBufferTexture, nullptr, &m_BackBufferRTV)))
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
		depthStencilTexture, nullptr, &m_DepthStencilView)))
	{
		return E_FAIL;
	}

	SafeRelease(depthStencilTexture);

	return S_OK;
}

HRESULT engine::D3D11Manager::compileShaderFromFile(const _wstring& path, const _string& entryPoint, const _string& targetProfile, ComPtr<ID3DBlob>& outBlob)
{
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), targetProfile.c_str(), 0, 0, outBlob.GetAddressOf(), errorBlob.GetAddressOf());

	// TODO : errorBlob 출력 메시지 확인하는거 추가 해야함.

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
