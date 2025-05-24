#include "BasePass.h"

#include "EditorCore.h"
#include "Material.h"
#include "Renderer.h"
#include "RenderManager.h"

HRESULT engine::BasePass::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	device->CreateDepthStencilState(&dsDesc, m_BasePassDSState.ReleaseAndGetAddressOf());

	return S_OK;
}

HRESULT engine::BasePass::Render(ID3D11DeviceContext* context, void* data)
{
	const BasePassData* passData = static_cast<BasePassData*>(data);

	ComPtr<ID3D11DepthStencilState> prevState;
	UINT ref;

	context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);

	const auto& viewMat = passData->ViewMat;
	const auto& projMat = passData->ProjMat;
	const auto& camPos = passData->CameraPosition;
	const auto& nearFar = passData->NearFarPlane;

	RenderManager::GetInstance().BeginMRT("G-Buffer");

	context->OMSetDepthStencilState(m_BasePassDSState.Get(), 0);

	for (const auto& renderer : *passData->Renderers)
	{
		if (const auto& owner = renderer->GetGameObject().lock())
		{
			if (owner->IsActive())
			{
				const auto& materials = renderer->GetMaterials();

				for (const auto& materialPair : materials)
				{
					if (materialPair.second->GetShader())
					{
						const auto& material = materialPair.second;
						material->SetMatrix("g_ViewMatrix", viewMat);
						material->SetMatrix("g_ProjMatrix", projMat);
						material->SetFloat4("CameraPosition", camPos);
						material->SetFloat4("NearFarPlane", nearFar);
					}
				}
				renderer->InputAssembler(context);
				renderer->Bind(context);
				renderer->Render(context);
			}
		}
	}

	RenderManager::GetInstance().EndMRT();

	context->OMSetDepthStencilState(prevState.Get(), ref);

	return S_OK;
}

HRESULT engine::BasePass::RenderEditor(ID3D11DeviceContext* context, void* data, _bool isGame)
{
	if (!isGame)
	{
		const BasePassData* passData = static_cast<BasePassData*>(data);

		ComPtr<ID3D11DepthStencilState> prevState;
		UINT ref;

		context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);

		const auto& viewMat = passData->ViewMat;
		const auto& projMat = passData->ProjMat;
		const auto& camPos = passData->CameraPosition;
		const auto& nearFar = passData->NearFarPlane;
		editor::EditorCore::GetInstance().BeginMRT("G-Buffer", isGame);

		context->OMSetDepthStencilState(m_BasePassDSState.Get(), 0);

		for (const auto& renderer : *passData->Renderers)
		{
			if (const auto& owner = renderer->GetGameObject().lock())
			{
				if (owner->IsActive())
				{
					const auto& materials = renderer->GetMaterials();

					for (const auto& materialPair : materials)
					{
						if (materialPair.second->GetShader())
						{
							const auto& material = materialPair.second;
							material->SetMatrix("g_ViewMatrix", viewMat);
							material->SetMatrix("g_ProjMatrix", projMat);
							material->SetFloat4("CameraPosition", camPos);
							material->SetFloat4("NearFarPlane", nearFar);
						}
					}
					renderer->InputAssembler(context);
					renderer->Bind(context);
					renderer->Render(context);
				}
			}
		}

		editor::EditorCore::GetInstance().EndMRT(isGame);

		context->OMSetDepthStencilState(prevState.Get(), ref);

		return S_OK;
	}

	else
	{
		const BasePassData* passData = static_cast<BasePassData*>(data);

		ComPtr<ID3D11DepthStencilState> prevState;
		UINT ref;

		context->OMGetDepthStencilState(prevState.ReleaseAndGetAddressOf(), &ref);

		const auto& viewMat = passData->ViewMat;
		const auto& projMat = passData->ProjMat;
		const auto& camPos = passData->CameraPosition;
		const auto& nearFar = passData->NearFarPlane;
		editor::EditorCore::GetInstance().BeginMRT("G-Buffer", isGame);

		context->OMSetDepthStencilState(m_BasePassDSState.Get(), 0);

		for (const auto& renderer : *passData->Renderers)
		{
			if (const auto& owner = renderer->GetGameObject().lock())
			{
				if (owner->IsActive())
				{
					const auto& materials = renderer->GetMaterials();

					for (const auto& materialPair : materials)
					{
						if (materialPair.second->GetShader())
						{
							const auto& material = materialPair.second;
							material->SetMatrix("g_ViewMatrix", viewMat);
							material->SetMatrix("g_ProjMatrix", projMat);
							material->SetFloat4("CameraPosition", camPos);
							material->SetFloat4("NearFarPlane", nearFar);
						}
					}
					renderer->InputAssembler(context);
					renderer->Bind(context);
					renderer->Render(context);
				}
			}
		}

		editor::EditorCore::GetInstance().EndMRT(isGame);

		context->OMSetDepthStencilState(prevState.Get(), ref);

		return S_OK;
	}
}

void engine::BasePass::Release()
{
	m_BasePassDSState.Reset();
}
