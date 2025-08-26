#include "Time.h"
using namespace EngineCore;

std::chrono::steady_clock::time_point Time::s_startTime;
std::chrono::steady_clock::time_point Time::s_lastFrameTime;
std::chrono::steady_clock::time_point Time::s_frameStartTime;
float	Time::s_deltaTime = 0.0f;
int		Time::s_targetFPS = 0;
float	Time::s_nowFPS = 0.0f;
int		Time::s_frameCount = 0;
float	Time::s_fpsTimer = 0.0f;

void Time::Init()
{
	s_startTime = s_lastFrameTime = std::chrono::steady_clock::now();
}

void Time::Update()
{
	auto now = std::chrono::steady_clock::now();
	s_deltaTime = std::chrono::duration<float>(now - s_lastFrameTime).count();
	s_lastFrameTime = now;

	s_fpsTimer += s_deltaTime;
	s_frameCount++;
	if (s_fpsTimer >= 1.0f)
	{
		s_nowFPS = static_cast<float>(s_frameCount) / s_fpsTimer;
		s_frameCount = 0;
		s_fpsTimer = 0.0f;
	}
}

void EngineCore::Time::FrameEnd()
{
	if (s_targetFPS <= 0)return;
	float minFrameTime = 1.0f / static_cast<float>(s_targetFPS);

	auto now = std::chrono::steady_clock::now();
	float frameDuration = std::chrono::duration<float>(now - s_frameStartTime).count();

	if (frameDuration < minFrameTime)
	{
		auto sleepDuration = std::chrono::duration<float>(minFrameTime - frameDuration);
		std::this_thread::sleep_for(sleepDuration);
	}
}

float Time::DeltaTime()
{
	return s_deltaTime;
}

float Time::TimeSinceStart()
{
	return std::chrono::duration<float>(s_lastFrameTime - s_startTime).count();
}