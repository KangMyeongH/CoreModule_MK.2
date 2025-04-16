#pragma once
#include "Renderer.h"

namespace engine
{
	class Mesh;

	class COREMODULE_API SkinnedMeshRenderer : public Renderer
    {
        DECLARE_REGISTER_COMPONENT(SkinnedMeshRenderer)
        //======================================//
        //				constructor				//
        //======================================//
	protected:
        explicit SkinnedMeshRenderer(const SharedPtr<GameObject>& owner, const _string& name = "SkinnedMeshRenderer");
        ~SkinnedMeshRenderer() override;
        SkinnedMeshRenderer(const SkinnedMeshRenderer& rhs);

        //======================================//
        //				 property				//
        //======================================//
	public:
        SharedPtr<Mesh> GetMesh() { return m_Mesh; }
        void SetMesh(const SharedPtr<Mesh>& mesh) { m_Mesh = mesh; }

        std::unordered_map<_string, AnimationClip>* GetAnimations() { return &m_Animation; }

        void SetAnimation(const std::unordered_map<_string, AnimationClip>& animationMap) { m_Animation = animationMap; }
        void SetSkeleton(const Skeleton& skeleton) { m_Skeleton = skeleton; }
        _string GetEvent() { return m_AnimState.EventString; }

        //======================================//
        //				  method				//
        //======================================//
	public:
        void Bind(const ComPtr<ID3D11DeviceContext>& context) override;
        void Render(const ComPtr<ID3D11DeviceContext>& context) override;

        Keyframe SampleBoneTrack(const BoneKeyFrames& track, _float currTime);

        void ChangeAnimation(const _string& animName, _float fadeDuration, _bool isLoop);
        void SetNextAnimation(const _string& animName, _float fadeDuration, _bool isLoop);
        void SetAnimationTime(_float time);

        _bool IsAnimFinish() const { return m_AnimState.IsFinish; }

        void UpdateAnimation(_float deltaTime);

        Keyframe BlendKeyframe(const Keyframe& k0, const Keyframe& k1, _float alpha);

        void Play() { m_bPlay = true; }
        void Pause() { m_bPlay = false; }
        void Stop()
        {
	        m_bPlay = false;
            m_AnimState.CurrentClip.clear();
            m_AnimState.CurrentTime = 0.0f;

            m_AnimState.NextClip.clear();
            m_AnimState.NextFadeDuration = 0.0f;
            m_AnimState.NextIsLoop = false;

            m_AnimState.IsCrossFading = false;
            m_AnimState.IsLoop = false;
            m_AnimState.IsFinish = true;

            m_AnimState.OldClip.clear();
            m_AnimState.OldClipTime = 0.0f;

            m_AnimState.FadeTimer = 0.0f;
            m_AnimState.FadeDuration = 0.0f;
        }

        void Destroy() override;

	protected:
        void registerComponent(ApplicationMode mode = CLIENT) override;

        //======================================//
        //				 serialize				//
        //======================================//
	public:
        void to_json(nlohmann::ordered_json& j) override;
        void from_json(const nlohmann::ordered_json& j) override;

        //======================================//
        //				  fields				//
        //======================================//
    private:
        SharedPtr<Mesh> m_Mesh;
        Skeleton m_Skeleton;
        AnimationState m_AnimState;
        std::unordered_map<_string, AnimationClip> m_Animation;
        std::vector<_float4X4> m_BoneMatrix;

        _bool m_bPlay;
    };
}
