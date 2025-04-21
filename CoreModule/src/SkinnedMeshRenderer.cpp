#include "SkinnedMeshRenderer.h"

#include "D3D11Manager.h"
#include "LoadManager.h"
#include "Material.h"
#include "Mesh.h"
#include "TimeManager.h"

DEFINE_REGISTER_COMPONENT(SkinnedMeshRenderer)

engine::SkinnedMeshRenderer::SkinnedMeshRenderer(const SharedPtr<GameObject>& owner, const _string& name)
	: Renderer(owner, name), m_bPlay(true)
{
}

engine::SkinnedMeshRenderer::~SkinnedMeshRenderer() = default;

engine::SkinnedMeshRenderer::SkinnedMeshRenderer(const SkinnedMeshRenderer& rhs)
	: Renderer(rhs), m_bPlay(true)
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

			material.second->SetColor("Ambient", _float4(0.8f, 0.8f, 0.8f, 0.8f));
			material.second->SetColor("Specular", _float4(1.f, 1.f, 1.f, 1.f));

			material.second->SetMatrix("g_WorldMatrix", GetTransform()->GetWorldMatrix());
		}
	}
	//=============================================================

	//_float4X4 worldMat;
	//XMStoreFloat4x4(&worldMat, XMMatrixTranspose(GetTransform()->GetWorldMatrix()));

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

engine::Keyframe engine::SkinnedMeshRenderer::SampleBoneTrack(const BoneKeyFrames& track, const _float currTime)
{
	const auto& frames = track.Frames;

	if (frames.empty())
	{
		return {};
	}

	if (currTime <= static_cast<float>(frames.front().Time))
	{
		return frames.front();
	}

	if (currTime >= static_cast<float>(frames.back().Time))
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

void engine::SkinnedMeshRenderer::ChangeAnimation(const _string& animName, const _float fadeDuration, const _bool isLoop)
{
	auto it = m_Animation.find(animName);

	if (it != m_Animation.end())
	{
		if (!m_AnimState.CurrentClip.empty() && animName != m_AnimState.CurrentClip)
		{
			m_AnimState.IsCrossFading = true;
			m_AnimState.IsLoop = isLoop;

			m_AnimState.OldClip = m_AnimState.CurrentClip;
			m_AnimState.OldClipTime = m_AnimState.CurrentTime;

			m_AnimState.CurrentClip = animName;
			m_AnimState.CurrentTime = 0.0f;

			m_AnimState.FadeDuration = fadeDuration;
			m_AnimState.FadeTimer = 0.0f;

		}

		else
		{
			m_AnimState.IsLoop = isLoop;

			m_AnimState.IsCrossFading = false;
			m_AnimState.CurrentClip = animName;
			m_AnimState.CurrentTime = 0.0f;
		}

		m_AnimState.NextClip.clear();
		m_AnimState.NextFadeDuration = 0.f;
		m_AnimState.NextIsLoop = false;
		m_AnimState.IsFinish = false;

		// 이벤트 활성화 초기화.
		for (auto& event : m_Animation[animName].Events)
		{
			event.IsActive = false;
		}
	}
}

void engine::SkinnedMeshRenderer::SetNextAnimation(const _string& animName, const _float fadeDuration, const _bool isLoop)
{
	m_AnimState.NextClip = animName;
	m_AnimState.NextFadeDuration = fadeDuration;
	m_AnimState.NextIsLoop = isLoop;
}

void engine::SkinnedMeshRenderer::SetAnimationTime(const _float time)
{
	if (!m_AnimState.CurrentClip.empty())
	{
		if (time > m_Animation[m_AnimState.CurrentClip].Duration)
		{
			m_AnimState.CurrentTime = m_Animation[m_AnimState.CurrentClip].Duration;
		}

		else
		{
			m_AnimState.CurrentTime = time;
		}
	}
}

void engine::SkinnedMeshRenderer::UpdateAnimation(_float deltaTime)
{
	if (m_bPlay)
	{
		m_AnimState.CurrentTime += deltaTime;
	}
	
	if (m_Animation.empty() || (m_AnimState.CurrentClip.empty() && m_AnimState.NextClip.empty()))
	{
		m_BoneMatrix.resize(m_Skeleton.Bones.size());

		_matrix rootMat = m_Skeleton.RootBone->GetWorldMatrix();
		_matrix rootInv = XMMatrixInverse(nullptr, rootMat);

		for (int i = 0; i < static_cast<int>(m_Skeleton.Bones.size()); ++i)
		{
			_matrix global = m_Skeleton.Bones[i].Transform->GetWorldMatrix() * rootInv;
			global *= DirectX::XMMatrixScaling(-1.f, 1.f, 1.f);
			_matrix inverse = XMLoadFloat4x4(&m_Skeleton.Bones[i].Offset);

			_float4X4 finalMat;
			XMStoreFloat4x4(&finalMat, XMMatrixTranspose(inverse * global));

			m_BoneMatrix[i] = finalMat;
		}
	}

	else
	{
		if (m_AnimState.CurrentClip.empty())
		{
			ChangeAnimation(m_AnimState.NextClip, m_AnimState.NextFadeDuration, m_AnimState.NextIsLoop);
		}

		if (m_AnimState.IsCrossFading)
		{
			m_AnimState.OldClipTime += deltaTime;
			m_AnimState.FadeTimer += deltaTime;
		}

		if (m_Animation.find(m_AnimState.CurrentClip) != m_Animation.end())
		{
			_float currClipDur = m_Animation[m_AnimState.CurrentClip].Duration;

			bool isEventActive = false;

			// 애니메이션 이벤트 처리
			for (auto& event : m_Animation[m_AnimState.CurrentClip].Events)
			{
				if (event.IsActive)
				{
					continue;
				}

				if (event.Time <= m_AnimState.CurrentTime)
				{
					m_AnimState.EventString = event.EventName;
					event.IsActive = true;
					isEventActive = true;
					break;
				}
			}

			if (!isEventActive)
			{
				m_AnimState.EventString = "";
			}

			// 애니메이션이 끝났을 때 처리
			if (m_AnimState.CurrentTime > currClipDur)
			{
				if (m_AnimState.IsLoop)
				{
					m_AnimState.CurrentTime = std::fmod(m_AnimState.CurrentTime, currClipDur);

					for (auto&  event : m_Animation[m_AnimState.CurrentClip].Events)
					{
						event.IsActive = false;
					}
				}

				else
				{
					m_AnimState.CurrentTime = currClipDur;
					m_AnimState.IsFinish = true;

					if (!m_AnimState.NextClip.empty())
					{
						ChangeAnimation(m_AnimState.NextClip, m_AnimState.NextFadeDuration, m_AnimState.NextIsLoop);
					}
				}
			}
		}

		if (m_AnimState.IsCrossFading && m_Animation.find(m_AnimState.OldClip) != m_Animation.end())
		{
			_float oldClipDur = m_Animation[m_AnimState.OldClip].Duration;
			if (m_AnimState.OldClipTime > oldClipDur)
			{
				m_AnimState.OldClipTime = oldClipDur;
			}
		}

		if (m_AnimState.IsCrossFading && m_AnimState.FadeTimer >= m_AnimState.FadeDuration)
		{
			m_AnimState.IsCrossFading = false;
		}

		m_BoneMatrix.resize(m_Skeleton.Bones.size());

		float alpha = 0.0f;

		if (m_AnimState.IsCrossFading && m_AnimState.FadeDuration > 0.0f)
		{
			alpha = m_AnimState.FadeTimer / m_AnimState.FadeDuration;
			if (alpha > 1.0f)
			{
				alpha = 1.0f;
			}
		}

		//===============//
		for (size_t i = 0; i < m_Skeleton.Bones.size(); ++i)
		{
			Keyframe finalKf;
			if (m_AnimState.IsCrossFading)
			{
				const auto& oldTracks = m_Animation[m_AnimState.OldClip].Tracks;
				const auto& newTracks = m_Animation[m_AnimState.CurrentClip].Tracks;

				Keyframe oldKf;
				Keyframe newKf;

				oldKf = SampleBoneTrack(oldTracks[i], m_AnimState.OldClipTime);
				newKf = SampleBoneTrack(newTracks[i], m_AnimState.CurrentTime);

				finalKf = BlendKeyframe(oldKf, newKf, alpha);
			}

			else
			{
				const auto& currTracks = m_Animation[m_AnimState.CurrentClip].Tracks;
				finalKf = SampleBoneTrack(currTracks[i], m_AnimState.CurrentTime);
			}

			m_Skeleton.Bones[i].Transform->SetLocalPosition(finalKf.Translation);
			m_Skeleton.Bones[i].Transform->SetLocalRotation(finalKf.Rotation);
			m_Skeleton.Bones[i].Transform->SetLocalScale(finalKf.Scale);
		}

		m_BoneMatrix.resize(m_Skeleton.Bones.size());

		_matrix rootMat = m_Skeleton.RootBone->GetWorldMatrix();
		_matrix rootInv = XMMatrixInverse(nullptr, rootMat);


		for (int i = 0; i < static_cast<int>(m_Skeleton.Bones.size()); ++i)
		{
			_matrix global = m_Skeleton.Bones[i].Transform->GetWorldMatrix() * rootInv;
			global *= DirectX::XMMatrixScaling(-1.f, 1.f, 1.f);
			_matrix inverse = XMLoadFloat4x4(&m_Skeleton.Bones[i].Offset);

			_float4X4 finalMat;
			XMStoreFloat4x4(&finalMat, XMMatrixTranspose(inverse * global));

			m_BoneMatrix[i] = finalMat;
		}
	}
}

engine::Keyframe engine::SkinnedMeshRenderer::BlendKeyframe(const Keyframe& k0, const Keyframe& k1, _float alpha)
{
	Keyframe result;
	result.Time = k0.Time * (1.0 - alpha) + k1.Time * alpha;

	result.Translation = k0.Translation + (k1.Translation - k0.Translation) * alpha;
	result.Scale = k0.Scale + (k1.Scale - k0.Scale) * alpha;

	result.Rotation = Quaternion::Slerp(k0.Rotation, k1.Rotation, alpha);

	return result;
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
		{"materials", nlohmann::ordered_json::array()},
		{"animationClips", nlohmann::ordered_json::array()}
	};

	for (const auto& pair : m_Material)
	{
		nlohmann::ordered_json matJson;

		int index = pair.first;
		std::string path = WStringToString(pair.second->GetPath());
		j["materials"].push_back({ {"index", index}, {"path", path} });
	}

	for (const auto& pair : m_Animation)
	{
		nlohmann::ordered_json animJson;

		std::string animName = pair.first;
		std::string path = WStringToString(pair.second.Path);
		j["animationClips"].push_back({ {"index", animName}, {"path", path} });
	}
}

void engine::SkinnedMeshRenderer::from_json(const nlohmann::ordered_json& j)
{
	for (const auto& matJson : j.at("materials"))
	{
		int index = matJson.at("index").get<_int>();
		_wstring path = StringToWString(matJson.at("path").get<_string>());

		SharedPtr<Material> material = Material::Create(shared_from_this());
		LoadManager::GetInstance().LoadMaterialData(material, path);

		SetMaterial(material, index);
	}

	for (const auto& animJson : j.at("animationClips"))
	{
		_string index = animJson.at("index").get<_string>();
		_wstring path = StringToWString(animJson.at("path").get<_string>());

		auto animationClip = LoadManager::GetInstance().ReadAnimationClipDataFromFile(path);

		m_Animation.emplace(index, animationClip);
	}
}
