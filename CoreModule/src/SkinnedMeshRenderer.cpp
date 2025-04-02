#include "SkinnedMeshRenderer.h"

#include "D3D11Manager.h"
#include "Material.h"
#include "Mesh.h"
#include "TimeManager.h"

DEFINE_REGISTER_COMPONENT(SkinnedMeshRenderer)

engine::SkinnedMeshRenderer::SkinnedMeshRenderer(const SharedPtr<GameObject>& owner, const _string& name)
	: Renderer(owner, name), m_CurrentTime(0)
{
}

engine::SkinnedMeshRenderer::~SkinnedMeshRenderer()
{

}

engine::SkinnedMeshRenderer::SkinnedMeshRenderer(const SkinnedMeshRenderer& rhs)
	: Renderer(rhs), m_CurrentTime(0)
{
}

void engine::SkinnedMeshRenderer::Bind(const ComPtr<ID3D11DeviceContext>& context)
{
	if (m_Mesh)
	{
		UpdateAnimation(TimeManager::GetInstance().GetDeltaTime());
		m_Mesh->Bind(context);

		for (auto& mat : m_Material)
		{
			mat.second->SetValue(m_BoneMatrix);
		}
	}
}

void engine::SkinnedMeshRenderer::Render(const ComPtr<ID3D11DeviceContext>& context)
{
	// TODO : 아래는 임시코드 임 지워야함.
	//=============================================================
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;           // Anisotropic 필터링 사용
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;         // U 좌표 랩 모드
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;         // V 좌표 랩 모드
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;         // W 좌표 랩 모드
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 16;                            // 최대 이방성 정도 (품질에 따라 조정)
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;      // 비교 함수 설정
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ComPtr<ID3D11SamplerState> sampler;

	D3D11Manager::GetInstance().CreateSampler(samplerDesc, sampler);

	for (auto material : m_Material)
	{
		if (material.second->GetShader())
		{
			material.second->SetSampler("Sampler", sampler);
			material.second->SetFloat4("DirLight_Dir", _float4(1.f, -1.f, 1.f, 0.f));
			material.second->SetColor("DirLight_Diffuse", _float4(1.f, 1.f, 1.f, 1.f));
			material.second->SetColor("DirLight_Ambient", _float4(1.f, 1.f, 1.f, 1.f));
			material.second->SetColor("DirLight_Specular", _float4(1.f, 1.f, 1.f, 1.f));
			//material->SetFloat4("CameraPosition", _float4(0.f, 10.f, -6.f, 1.f));

			material.second->SetColor("Ambient", _float4(0.8f, 0.8f, 0.8f, 0.8f));
			material.second->SetColor("Specular", _float4(1.f, 1.f, 1.f, 1.f));
			_float4X4 worldMat;
			XMStoreFloat4x4(&worldMat, XMMatrixTranspose(GetTransform()->GetWorldMatrix()));

			material.second->SetMatrix("g_WorldMatrix", worldMat);
		}
	}
	//=============================================================

	_float4X4 worldMat;
	XMStoreFloat4x4(&worldMat, XMMatrixTranspose(GetTransform()->GetWorldMatrix()));

	Bind(context);

	auto& subMeshes = m_Mesh->GetSubMeshes();

	for (auto i = 0; i < subMeshes.size(); ++i)
	{
		if (m_Material[subMeshes[i].MaterialIndex]->GetShader())
		{
			m_Material[subMeshes[i].MaterialIndex]->Bind(context.Get());

			context->DrawIndexed(subMeshes[i].IndexCount, subMeshes[i].IndexOffset, 0);
		}
	}
}

engine::Keyframe engine::SkinnedMeshRenderer::SampleBoneTrack(const BoneKeyFrames& track, double currTime)
{
	const auto& frames = track.Frames;

	if (frames.empty())
	{
		return {};
	}

	if (currTime <= frames.front().Time)
	{
		return frames.front();
	}

	if (currTime >= frames.back().Time)
	{
		return frames.back();
	}

	// 이진탐색 등으로 currTime에 해당하는 구간 찾기
	// 여기서는 단순 for문 예시
	for (int i = 0; i < static_cast<int>(frames.size()) - 1; i++)
	{
		const auto& k0 = frames[i];
		const auto& k1 = frames[i + 1];
		if (currTime >= k0.Time && currTime <= k1.Time)
		{
			double t = (currTime - k0.Time) / (k1.Time - k0.Time);
			// 보간
			Keyframe result;
			result.Time = currTime;
			// T, S → Lerp
			result.Translation = k0.Translation + (k1.Translation - k0.Translation) * t;
			result.Scale = k0.Scale + (k1.Scale - k0.Scale) * t;
			// R → Slerp
			Quaternion qr = Quaternion::Slerp(k0.Rotation, k1.Rotation, t);
			result.Rotation = qr;
			return result;
		}
	}
	// 못 찾으면 마지막 키
	return frames.back();
}

void engine::SkinnedMeshRenderer::UpdateAnimation(double deltaTime)
{
	m_CurrentTime += 0.002;

	std::vector<_matrix> localMatrices(m_Skeleton.Bones.size());

	for (auto& clip : m_Animation)
	{
		if (clip.second.Duration < m_CurrentTime)
		{
			m_CurrentTime = 0;
		}

		for (auto& track : clip.second.Tracks)
		{
			Keyframe kf = SampleBoneTrack(track, m_CurrentTime);

			m_Skeleton.Bones[track.BoneIndex].Transform->SetLocalPosition(kf.Translation);
			m_Skeleton.Bones[track.BoneIndex].Transform->SetLocalRotation(kf.Rotation);
			m_Skeleton.Bones[track.BoneIndex].Transform->SetLocalScale(kf.Scale);
		}

		m_BoneMatrix.resize(m_Skeleton.Bones.size());

		for (int i = 0; i < static_cast<int>(m_Skeleton.Bones.size()); ++i)
		{
			_matrix global = m_Skeleton.Bones[i].Transform->GetWorldMatrix();

			_matrix inverse = XMLoadFloat4x4(&m_Skeleton.Bones[i].Offset);

			_float4X4 finalMat;
			XMStoreFloat4x4(&finalMat, XMMatrixTranspose(inverse * global));

			m_BoneMatrix[i] = finalMat;
		}
	}
}


void engine::SkinnedMeshRenderer::Destroy()
{
	m_bDestroyed = true;
}

void engine::SkinnedMeshRenderer::registerComponent(ApplicationMode mode)
{
	Renderer::registerComponent(mode);
}

void engine::SkinnedMeshRenderer::to_json(nlohmann::ordered_json& j)
{
	std::string type = "SkinnedMeshRenderer";
	j = nlohmann::ordered_json{
		{"type", type},
		{"instanceID", GetInstanceID()}
	};
}

void engine::SkinnedMeshRenderer::from_json(const nlohmann::ordered_json& j)
{
	SetInstanceID(j.at("instanceID").get<int>());
}
