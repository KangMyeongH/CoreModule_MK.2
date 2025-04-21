#include "TimeManager.h"

IMPLEMENT_SINGLETON(engine::TimeManager)

engine::TimeManager::TimeManager()
    : m_Frequency(), m_LastFrameTime(), m_CurrentFrameTime(), m_AccTime(0),
    m_DeltaTime(1.0f), m_UnscaledDeltaTime(0), m_TimeScale(0), m_UnscaledTime(0),
    m_TargetFrameTime(0), m_SlowMotionTime(0), m_SlowTime(0), m_FPS(0)
{

}

engine::TimeManager::~TimeManager() = default;

void engine::TimeManager::Initialize()
{
    QueryPerformanceFrequency(&m_Frequency);
    QueryPerformanceCounter(&m_LastFrameTime);
    m_DeltaTime = 0.0f;
    m_TimeScale = 1.0f;
    m_UnscaledTime = 0.0f;
    m_TargetFrameTime = 1.0f / 155.0f;
}

void engine::TimeManager::TimeUpdate()
{
	QueryPerformanceCounter(&m_CurrentFrameTime);
	LONGLONG elapsedTicks = m_CurrentFrameTime.QuadPart - m_LastFrameTime.QuadPart;
	_double elapsedSeconds = static_cast<double>(elapsedTicks) / static_cast<double>(m_Frequency.QuadPart);

	m_DeltaTime = static_cast<float>(elapsedSeconds) * m_TimeScale;
	m_UnscaledDeltaTime = static_cast<float>(elapsedSeconds);
	m_UnscaledTime += m_UnscaledDeltaTime;

	m_AccTime += m_UnscaledDeltaTime;
	m_SlowTime += m_UnscaledDeltaTime;

	++m_FPS;

	if (m_AccTime >= 1.0f)
	{
		m_CurrentFPS = m_FPS;
		m_FPS = 0;
		m_AccTime = 0;
	}

	if (m_SlowTime >= m_SlowMotionTime)
	{
		m_SlowTime = 0.f;
		m_TimeScale = 1.0f;
		m_SlowMotionTime = -1.f;
	}

	m_LastFrameTime = m_CurrentFrameTime;
}

void engine::TimeManager::FrameLimit()
{
	QueryPerformanceCounter(&m_CurrentFrameTime);
	LONGLONG elapsedTicks = m_CurrentFrameTime.QuadPart - m_LastFrameTime.QuadPart;
	_double elapsedSeconds = static_cast<double>(elapsedTicks) / static_cast<double>(m_Frequency.QuadPart);

	// Wait if frame time is shorter than the target frame time
	while (elapsedSeconds < m_TargetFrameTime)
	{
		QueryPerformanceCounter(&m_CurrentFrameTime);
		elapsedTicks = m_CurrentFrameTime.QuadPart - m_LastFrameTime.QuadPart;
		elapsedSeconds = static_cast<double>(elapsedTicks) / static_cast<double>(m_Frequency.QuadPart);
	}
}

void engine::TimeManager::SlowMotion(const float timeScale, const float slowMotionTime)
{
    m_TimeScale = timeScale;
    m_SlowMotionTime = slowMotionTime;
}

void engine::TimeManager::TitleFPS(HWND hwnd)
{
	std::wstring newTitle = L"FPS : " + std::to_wstring(m_CurrentFPS);
	SetWindowTextW(hwnd, newTitle.c_str());
}
