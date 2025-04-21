#pragma once
#include "Component.h"

namespace engine
{
    class COREMODULE_API Camera : public Component
    {
    protected:
        //======================================//
        //				constructor				//
        //======================================//

        explicit Camera(const SharedPtr<GameObject>& owner);
        ~Camera() override = default;
        Camera(const Camera& rhs);

    public:
        //======================================//
        //				 property				//
        //======================================//

        void 	SetAspectRatio(const _float aspectRatio) { m_AspectRatio = aspectRatio; }

    	void 	SetFOV(const _float fov) { m_FiledOfView = fov; }
        _float 	GetFOV() const { return m_FiledOfView; }

        void 	SetNearPlane(const _float nearPlane) { m_NearPlane = nearPlane; }
        _float 	GetNearPlane() const { return m_NearPlane; }

        void 	SetFarPlane(const _float farPlane) { m_FarPlane = farPlane; }
        _float 	GetFarPlane() const { return m_FarPlane; }

        //======================================//
        //				  method				//
        //======================================//

    	void    UpdateCamera(_float4X4& viewMat, _float4X4& projMat) const;
    	void    SetMainCamera();

        Vector3 WorldToViewportPoint(const Vector3& worldPos) const;

        void 	Destroy() override;

    protected:
        void 	registerComponent(ApplicationMode mode = CLIENT) override;

    public:
        //======================================//
        //				 serialize				//
        //======================================//

    	void 	to_json(nlohmann::ordered_json& j) override;
        void 	from_json(const nlohmann::ordered_json& j) override;

    private:
        //======================================//
        //				  fields				//
        //======================================//

        _float4X4 	m_ViewMat;
        _float4X4 	m_ProjMat;

        _float 		m_FiledOfView;
        _float 		m_AspectRatio;
        _float 		m_NearPlane;
        _float 		m_FarPlane;

        DECLARE_REGISTER_COMPONENT(Camera)
    };
}
