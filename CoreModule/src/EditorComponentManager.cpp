#include "EditorComponentManager.h"

#include "BasePass.h"
#include "Camera.h"
#include "Collider.h"
#include "D3D11Manager.h"
#include "DebugRenderManager.h"
#include "DeferredPass.h"
#include "EditorCore.h"
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
#include "SkyPass.h"
#include "SkySphere.h"
#include "TextUI.h"
#include "UI.h"
#include "UIManager.h"

engine::editor::EditorComponentManager::EditorComponentManager() : m_MaxSort(0), m_MinSort(0), m_ViewMat(), m_ProjMat(),
                                                                   m_DirtyFlag(true)
{
}

engine::editor::EditorComponentManager::~EditorComponentManager()
= default;

IMPLEMENT_SINGLETON(engine::editor::EditorComponentManager)

void engine::editor::EditorComponentManager::Initialize()
{
	AddFont(L"HUGoth150", L"..\\Client\\Assets\\Resource\\Font\\HUGoth150.spritefont");
	m_Batch = UIManager::GetInstance().GetBatch();

	ID3D11Device* pDevice = D3D11Manager::GetInstance().GetDevice().Get();
	ID3D11DeviceContext* pContext = D3D11Manager::GetInstance().GetContext().Get();

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

	m_EffectPass = std::make_unique<engine::EffectPass>();
	m_EffectPass->Initialize(pDevice, pContext);

	m_GlowPass = std::make_unique<engine::GlowPass>();
	m_GlowPass->Initialize(pDevice, pContext);

	m_FinalPass = std::make_unique<engine::FinalPass>();
	m_FinalPass->Initialize(pDevice, pContext);
}

// TODO : Render Pipeline 수정해야함.
// 이거 안하면 컨텐츠도 없는거임.


void engine::editor::EditorComponentManager::Render(const ComPtr<ID3D11DeviceContext>& context, CamData* camData, _bool isGame)
{
	using namespace DirectX;
	_float4 camPos = camData->Position;

	PrePassData preData{};
	preData.ProjMat = camData->ProjMat;
	preData.ViewMat = camData->ViewMat;

	SkyPassData skyData{};
	skyData.ProjMat = camData->ProjMat;
	skyData.ViewMat = camData->ViewMat;
	skyData.CameraPosition = Vector3(camPos.x, camPos.y, camPos.z);
	
	BasePassData baseData{};
	baseData.ProjMat = camData->ProjMat;
	baseData.ViewMat = camData->ViewMat;
	baseData.CameraPosition = camPos;
	baseData.NearFarPlane = camData->NearFarPlane;

	LightPassData lightData{};
	lightData.ProjMat = camData->ProjMat;
	lightData.ViewMat = camData->ViewMat;
	lightData.CameraPosition = camPos;
	XMStoreFloat4x4(&lightData.InvProjMat, XMMatrixInverse(nullptr, XMLoadFloat4x4(&camData->ProjMat)));
	XMStoreFloat4x4(&lightData.InvViewMat, XMMatrixInverse(nullptr, XMLoadFloat4x4(&camData->ViewMat)));
	lightData.NearFarPlane = camData->NearFarPlane;

	EffectPassData effectData{};
	effectData.ProjMat = camData->ProjMat;
	effectData.ViewMat = camData->ViewMat;
	effectData.CamPos = _float3(camData->Position.x, camData->Position.y, camData->Position.z);

	PrePass(context, &preData, isGame);
	BasePass(context, &baseData, isGame);
	LightingPass(context, &lightData, isGame);
	OutlinePass(context, nullptr, isGame);
	DeferredPass(context, nullptr, isGame);
	SkyPass(context, &skyData, isGame);
	EffectPass(context, &effectData, isGame);
	GlowPass(context, nullptr, isGame);
	FinalPass(context, nullptr, isGame);

	RenderCollider(context, camData->ViewMat, camData->ProjMat);

	RenderUIComponent(context);
}

void engine::editor::EditorComponentManager::RenderUIComponent(const ComPtr<ID3D11DeviceContext>& context)
{
	m_MaxSort = INT_MIN;
	m_MinSort = INT_MAX;

	if (!m_UIMap.empty())
	{
		m_MaxSort = std::max(m_MaxSort, m_UIMap.rbegin()->first);
	}

	if (!m_TextUIMap.empty())
	{
		m_MaxSort = std::max(m_MaxSort, m_TextUIMap.rbegin()->first);
	}

	if (!m_UIMap.empty())
	{
		m_MinSort = std::min(m_MinSort, m_UIMap.begin()->first);
	}

	if (!m_TextUIMap.empty())
	{
		m_MinSort = std::min(m_MinSort, m_TextUIMap.begin()->first);
	}

	ComPtr<ID3D11BlendState>   blendState;
	FLOAT                      blendFactor[4] = {};
	UINT                       sampleMask = 0;

	ComPtr<ID3D11DepthStencilState> backupDepthState;
	UINT stencilRef = 0;

	context->OMGetBlendState(blendState.GetAddressOf(), blendFactor, &sampleMask);
	context->OMGetDepthStencilState(backupDepthState.GetAddressOf(), &stencilRef);

	D3D11Manager::GetInstance().SetUIAlphaBlendMode();

	const _float winSizeX = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeX());
	const _float winSizeY = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeY());

	_float4X4 viewMat, projMat;
	XMStoreFloat4x4(&viewMat, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&projMat, DirectX::XMMatrixOrthographicLH(winSizeX, winSizeY, -1.f, 1.f));

	for (_int sort = m_MinSort; sort <= m_MaxSort; ++sort)
	{
		auto uiIter = m_UIMap.find(sort);

		if (uiIter != m_UIMap.end())
		{
			for (const auto& ui : uiIter->second)
			{
				if (ui->IsEnabled())
				{
					auto material = ui->GetMaterial();

					if (material && material->GetShader())
					{
						material->SetMatrix("g_ViewMatrix", viewMat);
						material->SetMatrix("g_ProjMatrix", projMat);
					}

					ui->RenderUI(context);
				}
			}
		}

		auto textIter = m_TextUIMap.find(sort);

		if (textIter != m_TextUIMap.end() && !textIter->second.empty())
		{
			ComPtr<ID3D11BlendState>   prevBlendState;
			FLOAT                      prevBlendFactor[4] = {};
			UINT                       prevSampleMask = 0;

			ComPtr<ID3D11DepthStencilState> prevDepthState;
			UINT                       prevStencilRef = 0;

			context->OMGetBlendState(prevBlendState.GetAddressOf(), prevBlendFactor, &prevSampleMask);
			context->OMGetDepthStencilState(prevDepthState.GetAddressOf(), &prevStencilRef);

			m_Batch->Begin();

			for (const auto& text : textIter->second)
			{
				if (text->IsEnabled())
				{
					text->RenderUI(context);
				}
			}

			m_Batch->End();

			context->OMSetBlendState(prevBlendState.Get(), prevBlendFactor, prevSampleMask);
			context->OMSetDepthStencilState(prevDepthState.Get(), prevStencilRef);
		}
	}

	context->OMSetBlendState(blendState.Get(), blendFactor, sampleMask);
	context->OMSetDepthStencilState(backupDepthState.Get(), stencilRef);
}

void engine::editor::EditorComponentManager::RenderCollider(const ComPtr<ID3D11DeviceContext>& context,
	const _float4X4& viewMat, const _float4X4& projMat) const
{
	for (const auto& col : m_Colliders)
	{
		if (col->IsEnabled())
		{
			col->UpdateCollider();
		}
	}

	DebugRenderManager::GetInstance().RenderCollider(m_Colliders, context, viewMat, projMat);
}

void engine::editor::EditorComponentManager::AddComponent(const SharedPtr<GameObject>& owner, const SharedPtr<Component>& component)
{
	if (auto ui = std::dynamic_pointer_cast<UI>(component))
	{
		_int sort = ui->GetSorting();

		if (auto text = std::dynamic_pointer_cast<TextUI>(ui))
		{
			m_TextUIMap[sort].push_back(text);
		}

		else
		{
			m_UIMap[sort].push_back(ui);
		}
	}

	else if (auto renderer = std::dynamic_pointer_cast<Renderer>(component))
	{
		m_Renderers.push_back(renderer);
	}

	else if (auto camera = std::dynamic_pointer_cast<Camera>(component))
	{
		if (!m_MainCamera.lock())
		{
			m_MainCamera = camera;
		}

		m_Cameras.push_back(camera);
	}

	else if (auto collider = std::dynamic_pointer_cast<Collider>(component))
	{
		m_Colliders.push_back(collider);
	}

	else if (auto effect = std::dynamic_pointer_cast<Effect>(component))
	{
		m_Effects.push_back(effect);
	}

	else
	{
		m_Components.push_back(component);
	}

	component->SetOwner(owner);
	auto& components = owner->GetComponents();
	if (components.find(typeid(*component)) == components.end())
	{
		components[typeid(*component)].push_back(component);
	}
	component->registerComponent(EDITOR);
}

void engine::editor::EditorComponentManager::AddComponent(const SharedPtr<Component>& component)
{
	if (auto ui = std::dynamic_pointer_cast<UI>(component))
	{
		_int sort = ui->GetSorting();

		if (auto text = std::dynamic_pointer_cast<TextUI>(ui))
		{
			m_TextUIMap[sort].push_back(text);
		}

		else
		{
			m_UIMap[sort].push_back(ui);
		}
	}

	else if (auto renderer = std::dynamic_pointer_cast<Renderer>(component))
	{
		m_Renderers.push_back(renderer);
	}

	else if (auto camera = std::dynamic_pointer_cast<Camera>(component))
	{
		if (!m_MainCamera.lock())
		{
			m_MainCamera = camera;
		}

		m_Cameras.push_back(camera);
	}

	else if (auto collider = std::dynamic_pointer_cast<Collider>(component))
	{
		m_Colliders.push_back(collider);
	}

	else if (auto light = std::dynamic_pointer_cast<Light>(component))
	{
		m_Lights.push_back(light);
	}

	else if (auto effect = std::dynamic_pointer_cast<Effect>(component))
	{
		m_Effects.push_back(effect);
	}

	else
	{
		m_Components.push_back(component);
	}
}

void engine::editor::EditorComponentManager::AddFont(const _wstring& name, const _wstring& path)
{
	SharedPtr<DirectX::SpriteFont> font = std::make_shared<DirectX::SpriteFont>(D3D11Manager::GetInstance().GetDevice().Get(), path.c_str());

	m_Fonts.emplace(name, font);
}

void engine::editor::EditorComponentManager::OnSortingChanged(const SharedPtr<UI>& ui, const _int oldSort, const _int newSort,
                                                              const _bool isText)
{
	if (!isText)
	{
		auto& oldVec = m_UIMap[oldSort];
		oldVec.erase(std::remove(oldVec.begin(), oldVec.end(), ui), oldVec.end());
		if (oldVec.empty())
		{
			m_UIMap.erase(oldSort);
		}

		m_UIMap[newSort].push_back(ui);
	}

	else
	{
		auto& oldVec = m_TextUIMap[oldSort];
		oldVec.erase(std::remove(oldVec.begin(), oldVec.end(), ui), oldVec.end());
		if (oldVec.empty())
		{
			m_TextUIMap.erase(oldSort);
		}

		m_TextUIMap[newSort].push_back(ui);
	}
}

void engine::editor::EditorComponentManager::RenderSkySphere(const ComPtr<ID3D11DeviceContext>& context)
{
	if (!m_MainCamera.lock())
	{
		return;
	}

	ComPtr<ID3D11DepthStencilState> prevState;
	UINT ref;

	auto skySphere = RenderManager::GetInstance().GetSkySphere();

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

	skySphere->Render(context, m_ViewMat, m_ProjMat, camPos, sunDir);

	context->OMSetDepthStencilState(prevState.Get(), ref);
}

HRESULT engine::editor::EditorComponentManager::SkyPass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame)
{
	auto passData = static_cast<SkyPassData*>(data);

	for (const auto& light : m_Lights)
	{
		if (light->GetType() == LightType_Directional)
		{
			_float4 dir = light->GetLightDesc().Dir;
			passData->SunDir = { dir.x, dir.y, dir.z };
		}
	}

	m_SkyPass->RenderEditor(context.Get(), passData, isGame);

	return S_OK;
}

HRESULT engine::editor::EditorComponentManager::PrePass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame)
{
	auto passData = static_cast<PrePassData*>(data);
	passData->Renderers = &m_Renderers;
	m_PrePass->RenderEditor(context.Get(), passData, isGame);
	return S_OK;
}

HRESULT engine::editor::EditorComponentManager::BasePass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame)
{
	auto passData = static_cast<BasePassData*>(data);
	passData->Renderers = &m_Renderers;
	m_BasePass->RenderEditor(context.Get(), passData, isGame);

	return S_OK;
}

HRESULT engine::editor::EditorComponentManager::LightingPass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame)
{
	using namespace DirectX;
	auto passData = static_cast<LightPassData*>(data);
	passData->Lights = &m_Lights;
	
	m_LightingPass->RenderEditor(context.Get(), passData, isGame);

	return S_OK;
}

HRESULT engine::editor::EditorComponentManager::DeferredPass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame)
{
	m_DeferredPass->RenderEditor(context.Get(), nullptr, isGame);

	return S_OK;
}

HRESULT engine::editor::EditorComponentManager::OutlinePass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame)
{
	m_OutlinePass->RenderEditor(context.Get(), nullptr, isGame);
	return S_OK;
}

HRESULT engine::editor::EditorComponentManager::EffectPass(const ComPtr<ID3D11DeviceContext>& context, void* data,
	_bool isGame)
{
	auto passData = static_cast<EffectPassData*>(data);
	passData->Effects = &m_Effects;
	m_EffectPass->RenderEditor(context.Get(), passData, isGame);

	return S_OK;
}
HRESULT engine::editor::EditorComponentManager::GlowPass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame)
{
	m_GlowPass->RenderEditor(context.Get(), nullptr, isGame);

	return S_OK;
}


HRESULT engine::editor::EditorComponentManager::FinalPass(const ComPtr<ID3D11DeviceContext>& context, void* data, _bool isGame)
{
	m_FinalPass->RenderEditor(context.Get(), nullptr, isGame);

	return S_OK;
}


void engine::editor::EditorComponentManager::FlushDestroyComponent()
{
	for (auto it = m_Renderers.begin(); it != m_Renderers.end();)
	{
		const auto renderer = *it;

		if (renderer->IsDestroyed())
		{
			if (const auto owner = (*it)->GetGameObject().lock())
			{
				owner->RemoveComponent(*it);
			}

			it = m_Renderers.erase(it);
		}

		else
		{
			++it;
		}
	}

	for (auto it = m_Colliders.begin(); it != m_Colliders.end();)
	{
		const auto collider = *it;
		if(collider->IsDestroyed())
		{
			if (const auto owner = (*it)->GetGameObject().lock())
			{
				owner->RemoveComponent(*it);
			}

			it = m_Colliders.erase(it);
		}

		else
		{
			++it;
		}
	}

	for (auto& pair : m_UIMap)
	{
		for (auto it = pair.second.begin(); it != pair.second.end();)
		{
			const auto ui = *it;

			if (ui->IsDestroyed())
			{
				if (const auto owner = (*it)->GetGameObject().lock())
				{
					owner->RemoveComponent(*it);
				}

				it = pair.second.erase(it);

				SetDirty(true);
			}

			else
			{
				++it;
			}
		}
	}

	for (auto& pair : m_TextUIMap)
	{
		for (auto it = pair.second.begin(); it != pair.second.end();)
		{
			const auto ui = *it;

			if (ui->IsDestroyed())
			{
				if (const auto owner = (*it)->GetGameObject().lock())
				{
					owner->RemoveComponent(*it);
				}

				it = pair.second.erase(it);

				SetDirty(true);
			}

			else
			{
				++it;
			}
		}
	}

	for (auto it = m_Cameras.begin(); it != m_Cameras.end();)
	{
		const auto camera = *it;

		if (camera->IsDestroyed())
		{
			if (const auto owner = (*it)->GetGameObject().lock())
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

	for (auto it = m_Effects.begin(); it != m_Effects.end();)
	{
		const auto effect = *it;

		if (effect->IsDestroyed())
		{
			if (const auto owner = (*it)->GetGameObject().lock())
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


	for (auto it = m_Components.begin(); it != m_Components.end();)
	{
		const auto component = *it;

		if (component->IsDestroyed())
		{
			if (const auto owner = (*it)->GetGameObject().lock())
			{
				owner->RemoveComponent(component);
			}

			it = m_Components.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::editor::EditorComponentManager::Release()
{
	m_UIMap.clear();
	m_TextUIMap.clear();
	m_Renderers.clear();
	m_Cameras.clear();
	m_Components.clear();
	m_Lights.clear();
	m_Effects.clear();
}
