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

        void SetAnimation(const std::unordered_map<_string, AnimationClip>& animationMap)
        {
	        m_Animation = animationMap;
        }
        void SetSkeleton(const Skeleton& skeleton) { m_Skeleton = skeleton; }

        //======================================//
        //				  method				//
        //======================================//
	public:
        void Bind(const ComPtr<ID3D11DeviceContext>& context) override;
        void Render(const ComPtr<ID3D11DeviceContext>& context) override;

        Keyframe SampleBoneTrack(const BoneKeyFrames& track, double currTime);

        void UpdateAnimation(double deltaTime);

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
        std::unordered_map<_string, AnimationClip> m_Animation;
        std::vector<_float4X4> m_BoneMatrix;
        _string m_CurrentAnimation;
        double m_CurrentTime;
    };
}
