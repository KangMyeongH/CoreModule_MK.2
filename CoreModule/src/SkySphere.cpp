#include "SkySphere.h"

#include "LoadManager.h"
#include "Material.h"
#include "Mesh.h"
#include "TimeManager.h"

engine::SkySphere::SkySphere(const _string& name)
	: Object(name), m_ColorA(), m_ColorB(), m_HorizonColor(), m_ZenithColor(), m_ScrollSpeed(), m_Time(0)
{
}

engine::SkySphere::SkySphere(const SkySphere& rhs)
	: Object(rhs), m_ColorA(rhs.m_ColorA), m_ColorB(rhs.m_ColorB), m_HorizonColor(rhs.m_HorizonColor), m_ZenithColor(rhs.m_ZenithColor), m_ScrollSpeed(rhs.m_ScrollSpeed), m_Time(0.f)
{
}

void engine::SkySphere::Initialize(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& context)
{
	//m_ColorA = { 0.263, 0.383, 0.765 };
	m_ColorA = { 1.f, 1.f, 1.f };
	//m_ColorB = { 0.282, 0.423, 0.795 };
	m_ColorB = { 0.f, 0.f, 0.f };
	//m_HorizonColor = { 0.208, 0.403, 0.865 };
	m_HorizonColor = { 0.07f, 0.30f, 0.56f };
	//m_ZenithColor = { 0.170, 0.291, 0.510 };
	m_ZenithColor = { 0.34f, 0.57f, 1.0f };
	m_ScrollSpeed = { 0.002f, 0.000f };

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;         // 선형 보간
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;            // U 방향 반복
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;            // V 방향 반복
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;           // 2D는 필요 없음
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, m_Sampler.ReleaseAndGetAddressOf());

	m_Material = Material::Create(nullptr);
	m_Material->SetName("SkySphere");
	m_Material->LoadShader(L"..\\Client\\Assets\\Resource\\Shader\\SkySphere.hlsl");
	_wstring path = L"..\\Client\\Assets\\Resource\\SkyBox\\T_Cloud_middle_M.png";

	m_Material->SetTexture("MaskTexture", path);
	m_Material->SetSampler("Sampler", m_Sampler);

	ModelData modelData = LoadManager::GetInstance().ReadModelDataFromFile(L"..\\Client\\Assets\\Resource\\SkyBox\\SM_SkySphere.model");

	m_Mesh = Mesh::Create(modelData.Meshes[0].VIBuffer);

	std::vector<SubMesh> subMeshes;

	for (auto& subMeshData : modelData.Meshes[0].SubMeshes)
	{
		SubMesh subMesh;
		subMesh.IndexOffset = subMeshData.IndexOffset;
		subMesh.IndexCount = subMeshData.IndexCount;
		subMesh.MaterialIndex = subMeshData.MaterialIndex;

		subMeshes.push_back(subMesh);
	}

	m_Mesh->SetSubMesh(subMeshes);

	m_Rotation = Quaternion::Euler(-90.f, 0.f, 0.f);
	m_Scale = Vector3(0.0001f, 0.0001f, 0.0001f);
}

void engine::SkySphere::Render(const ComPtr<ID3D11DeviceContext>& context, const _float4X4& view, const _float4X4& proj, const Vector3& camPos, const _float3& sunDir)
{
	if (m_Material->GetShader())
	{
		m_Time += TimeManager::GetInstance().GetDeltaTime();

		const _matrix matScale = DirectX::XMMatrixScalingFromVector(m_Scale.ToVector());
		const _matrix matRot = DirectX::XMMatrixRotationQuaternion(m_Rotation.ToVector());
		const _matrix matTrans = DirectX::XMMatrixTranslationFromVector(camPos.ToVector());

		_matrix worldMat = matScale * matRot * matTrans;

		_float3 originSunDir = sunDir;

		// TODO : 빛 방향이랑 해 방향이 안맞는 문제 해결해야함.

		_float3 finalSunDir;
		_vector vSunDir = DirectX::XMVector3Normalize(XMLoadFloat3(&originSunDir));
		_matrix rotMat = DirectX::XMMatrixRotationQuaternion(Quaternion::Euler(0.f, 0.f, 0.f).ToVector());
		_vector rotatedSunDir = DirectX::XMVector3TransformNormal(vSunDir, rotMat);
		DirectX::XMStoreFloat3(&finalSunDir, rotatedSunDir);


		m_Material->SetFloat3("SunLightDir", finalSunDir);
		m_Material->SetFloat("SunRadius", 1.f);

		m_Material->SetMatrix("WorldRot", matRot);
		m_Material->SetFloat3("ColorA", m_ColorA);
		m_Material->SetFloat3("ColorB", m_ColorB);
		m_Material->SetFloat3("HorizonColor", m_HorizonColor);
		m_Material->SetFloat3("ZenithColor", m_ZenithColor);
		m_Material->SetFloat2("ScrollSpeed", m_ScrollSpeed);
		m_Material->SetFloat("Time", m_Time);

		m_Material->SetMatrix("g_WorldMatrix", worldMat);
		m_Material->SetMatrix("g_ViewMatrix", view);
		m_Material->SetMatrix("g_ProjMatrix", proj);
	}

	m_Mesh->Bind(context.Get());
	m_Material->Bind(context.Get());
	auto& subMeshes = m_Mesh->GetSubMeshes();

	for (auto i = 0; i < subMeshes.size(); ++i)
	{
		context->DrawIndexed(subMeshes[i].IndexCount, subMeshes[i].IndexOffset, 0);
	}


}

engine::SharedPtr<engine::SkySphere> engine::SkySphere::Create()
{
	return {
		new SkySphere(),
		[](const SkySphere* ptr) { delete ptr; }
	};
}
