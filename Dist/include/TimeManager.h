#pragma once
#include "core_defines.h"

namespace engine
{
    class COREMODULE_API TimeManager
    {
    private:
        //======================================//
        //				constructor				//
        //======================================//

        TimeManager();
        ~TimeManager();
    public:
        DECLARE_SINGLETON(TimeManager)

        //======================================//
        //				 property				//
        //======================================//

        _float 		GetDeltaTime() const { return m_DeltaTime; }
        _float		GetUnscaledDeltaTime() const { return m_UnscaledDeltaTime; }
        _float		GetTimeScale() const { return m_TimeScale; }
        void		SetTimeScale(const float scale) { m_TimeScale = scale; }
        _float		GetUnscaledTime() const { return m_UnscaledTime; }
        void		SetTargetFrameRate(const float fps) { m_TargetFrameTime = 1.0f / fps; }

        //======================================//
        //				  method				//
        //======================================//

        void	Initialize();
        void	TimeUpdate();
        void	FrameLimit();
        void	SlowMotion(float timeScale, float slowMotionTime);

    private:
        LARGE_INTEGER   m_Frequency;
        LARGE_INTEGER   m_LastFrameTime;
        LARGE_INTEGER   m_CurrentFrameTime;
        _double         m_AccTime;
        _float          m_DeltaTime;
        _float          m_UnscaledDeltaTime;
        _float          m_TimeScale;
        _float			m_UnscaledTime;
        _float			m_TargetFrameTime;
        _float			m_SlowMotionTime;
        _float			m_SlowTime;
        _int			m_FPS;
    };
}
