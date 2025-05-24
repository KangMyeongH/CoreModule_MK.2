#include "RenderManager.h"

#include "BasePass.h"
#include "Camera.h"
#include "D3D11Manager.h"
#include "DeferredPass.h"
#include "Effect.h"
#include "EffectPass.h"
#include "FinalPass.h"
#include "GameObject.h"
#include "GlowPass.h"
#include "Light.h"
#include "LightingPass.h"
#include "Material.h"
#include "OutLinePass.h"
#include "PrePass.h"
#include "Renderer.h"
#include "RenderPass.h"
#include "RenderTarget.h"
#include "SkyPass.h"
#include "SkySphere.h"

IMPLEMENT_SINGLETON(engine::RenderManager)

void engine::RenderManager::Initialize(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& context)
{
	m_SkySphere = SkySphere::Create();
	m_SkySphere->Initialize(device, context);

	D3D11_VIEWPORT              viewportDesc{};

	_uint       numViewports = { 1 };

	context->RSGetViewports(&numViewports, &viewportDesc);

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = FALSE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsDesc, m_RTDebugDSState.ReleaseAndGetAddressOf());

	AddRenderTarget("Target_Position", device, context, viewportDesc.Width, viewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f));
	FindRenderTarget("Target_Position")->ReadyDebug(150.f, 150.f, 300.f, 300.f);
	AddRenderTarget("Target_Diffuse", device, context, viewportDesc.Width, viewportDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(1.f, 1.f, 1.f, 0.f));
	FindRenderTarget("Target_Diffuse")->ReadyDebug(450.f, 150.f, 300.f, 300.f);
	AddRenderTarget("Target_Normal", device, context, viewportDesc.Width, viewportDesc.Height, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.f, 1.f, 1.f, 1.f));
	FindRenderTarget("Target_Normal")->ReadyDebug(750.f, 150.f, 300.f, 300.f);
	AddRenderTarget("Target_Depth", device, context, viewportDesc.Width, viewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f));
	FindRenderTarget("Target_Depth")->ReadyDebug(1050.f, 150.f, 300.f, 300.f);

	AddRenderTarget("Target_Outline", device, context, viewportDesc.Width, viewportDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f));
	FindRenderTarget("Target_Outline")->ReadyDebug(150.f, 750.f, 300.f, 300.f);

	AddRenderTarget("Target_Shade", device, context, viewportDesc.Width, viewportDesc.Height, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 0.f));
	FindRenderTarget("Target_Shade")->ReadyDebug(150.f, 450.f, 300.f, 300.f);

	AddRenderTarget("Target_Specular", device, context, viewportDesc.Width, viewportDesc.Height, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 0.f));
	FindRenderTarget("Target_Specular")->ReadyDebug(450.f, 450.f, 300.f, 300.f);

	AddRenderTarget("Target_GlowMap", device, context, viewportDesc.Width, viewportDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f));
	FindRenderTarget("Target_GlowMap")->ReadyDebug(150.f, 150.f, 300.f, 300.f);

	AddRenderTarget("Target_FinalScene", device, context, viewportDesc.Width, viewportDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f));

	AddRenderTarget("Target_L1_4X4", device, context, viewportDesc.Width / 4.f, viewportDesc.Height / 4.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f,0.f));
	AddRenderTarget("Target_L2_6X6", device, context, viewportDesc.Width / 24.f, viewportDesc.Height / 24.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f));
	AddRenderTarget("Target_L3_6X6", device, context, viewportDesc.Width / 144.f, viewportDesc.Height / 144.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f));
	AddRenderTarget("Target_L3_Tmp", device, context, viewportDesc.Width / 144.f, viewportDesc.Height / 144.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f));
	AddRenderTarget("Target_L2_Tmp", device, context, viewportDesc.Width / 24.f, viewportDesc.Height / 24.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f));
	AddRenderTarget("Target_L1_Tmp", device, context, viewportDesc.Width / 4.f, viewportDesc.Height / 4.f, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f));

	AddRenderTarget("Target_Bloom", device, context, viewportDesc.Width, viewportDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f));


	AddMRT("G-Buffer", "Target_Position");
	AddMRT("G-Buffer", "Target_Diffuse");
	AddMRT("G-Buffer", "Target_Normal");
	AddMRT("G-Buffer", "Target_Depth");

	AddMRT("Light-Pass", "Target_Shade");
	AddMRT("Light-Pass", "Target_Specular");

	AddMRT("Outline-Pass", "Target_Outline");

	AddMRT("Effect-Pass", "Target_FinalScene");
	AddMRT("Effect-Pass", "Target_GlowMap");

	AddMRT("Final-Scene", "Target_FinalScene");

	const auto pDevice = device.Get();
	const auto pContext = context.Get();

	// SkyPass Initialize
	m_SkyPass = std::make_unique<engine::SkyPass>();
	m_SkyPass->Initialize(pDevice, pContext);

	// PrePass Initialize
	m_PrePass = std::make_unique<engine::PrePass>();
	m_PrePass->Initialize(pDevice, pContext);

	// BasePass Initialize
	m_BasePass = std::make_unique<engine::BasePass>();
	m_BasePass->Initialize(pDevice, pContext);

	// LightingPass
	m_LightingPass = std::make_unique<engine::LightingPass>();
	m_LightingPass->Initialize(pDevice, pContext);

	// DeferredPass
	m_DeferredPass = std::make_unique<engine::DeferredPass>();
	m_DeferredPass->Initialize(pDevice, pContext);

	// OutlinePass
	m_OutlinePass = std::make_unique<engine::OutLinePass>();
	m_OutlinePass->Initialize(pDevice, pContext);

	// EffectPass
	m_EffectPass = std::make_unique<engine::EffectPass>();
	m_EffectPass->Initialize(pDevice, pContext);

	// GlowPass(Bloom)
	m_GlowPass = std::make_unique<engine::GlowPass>();
	m_GlowPass->Initialize(pDevice, pContext);

	// FinalPass
	m_FinalPass = std::make_unique<engine::FinalPass>();
	m_FinalPass->Initialize(pDevice, pContext);
}

engine::RenderManager::RenderManager() = default;

engine::RenderManager::~RenderManager() = default;

void engine::RenderManager::AddCamera(const SharedPtr<Camera>& camera)
{
	if (!m_MainCamera.lock())
	{
		m_MainCamera = camera;
	}

	m_Cameras.push_back(camera);
}

void engine::RenderManager::AddRenderer(const SharedPtr<Renderer>& renderer)
{
	m_RegisterQueue.push_back(renderer);
}

void engine::RenderManager::AddLight(const SharedPtr<Light>& light)
{
	m_LightRegisterQueue.push_back(light);
}

void engine::RenderManager::AddEffect(const SharedPtr<Effect>& effect)
{
	m_EffectRegisterQueue.push_back(effect);
}

void engine::RenderManager::UpdateMainCamera()
{
	if (auto mainCam = m_MainCamera.lock())
	{
		mainCam->UpdateCamera(m_ViewMat, m_ProjMat);
		//XMStoreFloat4x4(&m_ViewMat, XMMatrixTranspose(XMLoadFloat4x4(&m_ViewMat)));
		//XMStoreFloat4x4(&m_ProjMat, XMMatrixTranspose(XMLoadFloat4x4(&m_ProjMat)));

	}

	else
	{
		// TODO : MainCam이 없을 때 예외 처리.
	}
}

void engine::RenderManager::RenderSkySphere(const ComPtr<ID3D11DeviceContext>& context)
{
	ComPtr<ID3D11DepthStencilState> prevState;
	UINT ref;

	context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);

	D3D11Manager::GetInstance().SetDepthNoWrite();

	Vector3 camPos = m_MainCamera.lock()->GetTransform()->Position();

	// SkySphere
	_float3 sunDir{};

	for (auto light : m_Lights)
	{
		if (light->GetType() == LightType_Directional)
		{
			_float4 dir = light->GetLightDesc().Dir;
			sunDir = { dir.x, dir.y, dir.z };
		}
	}

	m_SkySphere->Render(context, m_ViewMat, m_ProjMat, camPos, sunDir);

	context->OMSetDepthStencilState(prevState.Get(), ref);
}

void engine::RenderManager::Render(const ComPtr<ID3D11DeviceContext>& context)
{
	PrePass(context);
	BasePass(context);
	LightingPass(context);
	OutlinePass(context);
	DeferredPass(context);
	SkyPass(context);
	EffectPass(context);
	GlowPass(context);
	FinalPass(context);
}

//BasePass(context);
//LightingPass(context);

//Vector3 camPos = m_MainCamera.lock()->GetTransform()->Position();
//_float4 finalCamPos = { camPos.Value.x, camPos.Value.y, camPos.Value.z, 1.f };

//for (const auto& renderer : m_Renderers)
//{
//	if (auto owner = renderer->GetGameObject().lock())
//	{
//		if (owner->IsActive())
//		{
//			auto materials = renderer->GetMaterials();

//			for (auto material : materials)
//			{
//				if (material.second->GetShader())
//				{
//					for (auto light : m_Lights)
//					{
//						light->BindLight(material.second);
//					}

//					material.second->SetMatrix("g_ViewMatrix", m_ViewMat);
//					material.second->SetMatrix("g_ProjMatrix", m_ProjMat);
//					material.second->SetFloat4("CameraPosition", finalCamPos);
//				}
//			}
//			renderer->InputAssembler(context.Get());
//			renderer->Bind(context.Get());
//			renderer->Render(context.Get());
//		}
//	}
//}

void engine::RenderManager::RegisterRenderer()
{
	for (auto it = m_RegisterQueue.begin(); it != m_RegisterQueue.end();)
	{
		SharedPtr<Renderer> renderer = *it;
		m_Renderers.push_back(renderer);

		it = m_RegisterQueue.erase(it);
	}

	m_RegisterQueue.clear();
}

void engine::RenderManager::FlushDestroyRenderer()
{
	for (auto it = m_Renderers.begin(); it != m_Renderers.end();)
	{
		SharedPtr<Renderer> renderer = *it;
		if (renderer->IsDestroyed())
		{
			if (const auto owner = renderer->GetGameObject().lock())
			{
				owner->RemoveComponent(renderer);
			}

			it = m_Renderers.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::RenderManager::RegisterEffect()
{
	for (auto it = m_EffectRegisterQueue.begin(); it != m_EffectRegisterQueue.end();)
	{
		SharedPtr<Effect> effect = *it;
		m_Effects.push_back(effect);

		it = m_EffectRegisterQueue.erase(it);
	}

	m_EffectRegisterQueue.clear();
}

void engine::RenderManager::FlushDestroyEffect()
{
	for (auto it = m_Effects.begin(); it != m_Effects.end();)
	{
		SharedPtr<Effect> effect = *it;
		if (effect->IsDestroyed())
		{
			if (const auto& owner = effect->GetGameObject().lock())
			{
				owner->RemoveComponent(effect);
			}

			it = m_Effects.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::RenderManager::RegisterLight()
{
	for (auto it = m_LightRegisterQueue.begin(); it != m_LightRegisterQueue.end();)
	{
		SharedPtr<Light> light = *it;
		m_Lights.push_back(light);

		it = m_LightRegisterQueue.erase(it);
	}

	m_LightRegisterQueue.clear();
}

void engine::RenderManager::FlushDestroyLight()
{
	for (auto it = m_Lights.begin(); it != m_Lights.end();)
	{
		SharedPtr<Light> light = *it;
		if (light->IsDestroyed())
		{
			if (const auto owner = light->GetGameObject().lock())
			{
				owner->RemoveComponent(light);
			}

			it = m_Lights.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::RenderManager::FlushDestroyCamera()
{
	for (auto it = m_Cameras.begin(); it != m_Cameras.end();)
	{
		const auto camera = *it;

		if (camera->IsDestroyed())
		{
			if (const auto owner = camera->GetGameObject().lock())
			{
				owner->RemoveComponent(camera);
			}

			it = m_Cameras.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::RenderManager::AddRenderTarget(const _string& tag, const ComPtr<ID3D11Device>& device,
	const ComPtr<ID3D11DeviceContext>& context, _uint sizeX, _uint sizeY, DXGI_FORMAT pixelFormat,
	const _float4& clearColor)
{
	const auto& rt = RenderTarget::Create(device, context, sizeX, sizeY, pixelFormat, clearColor);

	if (nullptr == rt)
	{
		return;
	}

	m_RenderTargets.emplace(tag, rt);
}

void engine::RenderManager::AddMRT(const _string& mrtTag, const _string& targetTag)
{
	const auto& renderTarget = FindRenderTarget(targetTag);
	if (renderTarget == nullptr)
	{
		return;
	}

	std::list<SharedPtr<RenderTarget>>* mrt = FindMRT(mrtTag);

	if (mrt == nullptr)
	{
		std::list<SharedPtr<RenderTarget>> mrtList;
		mrtList.push_back(renderTarget);
		m_MRTs.emplace(mrtTag, mrtList);
	}

	else
	{
		mrt->push_back(renderTarget);
	}
}

engine::SharedPtr<engine::RenderTarget> engine::RenderManager::FindRenderTarget(const _string& tag)
{
	const auto iter = m_RenderTargets.find(tag);

	if (iter == m_RenderTargets.end())
	{
		return nullptr;
	}

	return iter->second;
}

std::list<engine::SharedPtr<engine::RenderTarget>>* engine::RenderManager::FindMRT(const _string& tag)
{
	const auto iter = m_MRTs.find(tag);

	if (iter == m_MRTs.end())
	{
		return nullptr;
	}

	return &iter->second;
}

void engine::RenderManager::ClearRenderTarget(const ComPtr<ID3D11DeviceContext>& context)
{
	for (const auto& pair : m_RenderTargets)
	{
		pair.second->Clear(context.Get());
	}
}

HRESULT engine::RenderManager::BeginMRT(const _string& tag)
{
	auto iter = m_MRTs.find(tag);

	if (iter == m_MRTs.end())
	{
		return E_FAIL;
	}

	std::list<SharedPtr<RenderTarget>>* mtvs = &(iter->second);

	ID3D11DeviceContext* context = D3D11Manager::GetInstance().GetContext().Get();

	_uint numRenderTargets = 0;

	ID3D11RenderTargetView* renderTargets[8] = {};

	for (auto& renderTarget : *mtvs)
	{
		//renderTarget->Clear(context);

		renderTargets[numRenderTargets++] = renderTarget->GetRTV().Get();
	}
	ID3D11DepthStencilView* dsv = D3D11Manager::GetInstance().GetDepthStencilView().Get();

	context->OMSetRenderTargets(numRenderTargets, renderTargets, dsv);

	return S_OK;
}

HRESULT engine::RenderManager::EndMRT()
{
	ID3D11DeviceContext* context = D3D11Manager::GetInstance().GetContext().Get();
	ID3D11RenderTargetView* mainRTV = D3D11Manager::GetInstance().GetMainRTV().Get();
	ID3D11DepthStencilView* dsv = D3D11Manager::GetInstance().GetDepthStencilView().Get();

	context->OMSetRenderTargets(1, &mainRTV, dsv);

	return S_OK;
}

HRESULT engine::RenderManager::SkyPass(const ComPtr<ID3D11DeviceContext>& context)
{
	SkyPassData data{};
	Vector3 camPos = m_MainCamera.lock()->GetTransform()->Position();
	_float3 sunDir{};
	for (auto light : m_Lights)
	{
		if (light->GetType() == LightType_Directional)
		{
			_float4 dir = light->GetLightDesc().Dir;
			sunDir = { dir.x, dir.y, dir.z };
		}
	}
	data.ProjMat = m_ProjMat;
	data.ViewMat = m_ViewMat;
	data.CameraPosition = camPos;
	data.SunDir = sunDir;

	m_SkyPass->Render(context.Get(), &data);

	return S_OK;
}

HRESULT engine::RenderManager::PrePass(const ComPtr<ID3D11DeviceContext>& context)
{
	PrePassData data;
	data.Renderers = &m_Renderers;
	data.ViewMat = m_ViewMat;
	data.ProjMat = m_ProjMat;

	m_PrePass->Render(context.Get(), &data);

	return S_OK;
}

HRESULT engine::RenderManager::BasePass(const ComPtr<ID3D11DeviceContext>& context)
{
	_float3 camPos{};
	_float4 nearFarPlane{};

	if (const auto& mainCam = m_MainCamera.lock())
	{
		camPos = mainCam->GetTransform()->Position().Value;
		nearFarPlane = _float4{ mainCam->GetNearPlane(), mainCam->GetFarPlane(), 0.f, 0.f };
	}

	BasePassData data{};
	data.Renderers = &m_Renderers;
	data.ViewMat = m_ViewMat;
	data.ProjMat = m_ProjMat;
	data.CameraPosition = _float4(camPos.x, camPos.y, camPos.z, 1.f);
	data.NearFarPlane = nearFarPlane;

	m_BasePass->Render(context.Get(), &data);

	return S_OK;
}

HRESULT engine::RenderManager::LightingPass(const ComPtr<ID3D11DeviceContext>& context)
{
	using namespace DirectX;
	_float3 camPos{};
	_float4 nearFarPlane{};

	_matrix viewMat = XMLoadFloat4x4(&m_ViewMat);
	_matrix projMat = XMLoadFloat4x4(&m_ProjMat);
	_float4X4 invViewMat;
	XMStoreFloat4x4(&invViewMat, XMMatrixInverse(nullptr, viewMat));
	_float4X4 invProjMat;
	XMStoreFloat4x4(&invProjMat, XMMatrixInverse(nullptr, projMat));

	if (const auto& mainCam = m_MainCamera.lock())
	{
		camPos = mainCam->GetTransform()->Position().Value;
		nearFarPlane = _float4{ mainCam->GetNearPlane(), mainCam->GetFarPlane(), 0.f, 0.f };
	}

	LightPassData data{};
	data.Lights = &m_Lights;
	data.ViewMat = m_ViewMat;
	data.ProjMat = m_ProjMat;
	data.InvViewMat = invViewMat;
	data.InvProjMat = invProjMat;
	data.CameraPosition = _float4(camPos.x, camPos.y, camPos.z, 1.f);
	data.NearFarPlane = nearFarPlane;

	m_LightingPass->Render(context.Get(), &data);

	return S_OK;
}

HRESULT engine::RenderManager::DeferredPass(const ComPtr<ID3D11DeviceContext>& context)
{
	m_DeferredPass->Render(context.Get(), nullptr);

	return S_OK;
}

HRESULT engine::RenderManager::OutlinePass(const ComPtr<ID3D11DeviceContext>& context)
{
	m_OutlinePass->Render(context.Get(), nullptr);
	return S_OK;
}

HRESULT engine::RenderManager::EffectPass(const ComPtr<ID3D11DeviceContext>& context)
{
	if (m_Effects.empty())
		return S_OK;

	using namespace DirectX;
	_float3 camPos{};

	if (const auto& mainCam = m_MainCamera.lock())
	{
		camPos = mainCam->GetTransform()->Position().Value;
	}

	EffectPassData data;
	data.Effects = &m_Effects;
	data.ViewMat = m_ViewMat;
	data.ProjMat = m_ProjMat;
	data.CamPos = camPos;

	m_EffectPass->Render(context.Get(), &data);

	return S_OK;
}

HRESULT engine::RenderManager::GlowPass(const ComPtr<ID3D11DeviceContext>& context)
{
	m_GlowPass->Render(context.Get(), nullptr);
	return S_OK;
}


HRESULT engine::RenderManager::FinalPass(const ComPtr<ID3D11DeviceContext>& context)
{
	m_FinalPass->Render(context.Get(), nullptr);
	return S_OK;
}


HRESULT engine::RenderManager::DebugRender(const ComPtr<ID3D11DeviceContext>& context)
{
	context->OMSetDepthStencilState(m_RTDebugDSState.Get(), 0);

	//auto mrt = FindMRT("G-Buffer");
	//for (const auto& rt : *mrt)
	//{
	//	rt->Render(context.Get());
	//}

	//mrt = FindMRT("Light-Pass");
	//for (const auto& rt : *mrt)
	//{
	//	rt->Render(context.Get());
	//}

	//mrt = FindMRT("Outline-Pass");
	//for (const auto& rt : *mrt)
	//{
	//	rt->Render(context.Get());
	//}

	return S_OK;
}

void engine::RenderManager::Release()
{
	m_Renderers.clear();
	m_RegisterQueue.clear();

	m_Effects.clear();
	m_EffectRegisterQueue.clear();

	m_MainCamera.reset();
	m_Cameras.clear();

	m_Lights.clear();
	m_LightRegisterQueue.clear();
	//m_SkySphere.reset();

	m_Models.clear();

	//m_SkyPass.release();
	//m_SkyPass.reset();
	//m_PrePass.release();
	//m_PrePass.reset();
	//m_BasePass.release();
	//m_BasePass.reset();
	//m_LightingPass.release();
	//m_LightingPass.reset();

	m_RenderTargets.clear();
	m_MRTs.clear();
}
