#pragma once
namespace EngineCore
{
	class Time
	{
	public:
		static void Init();
		static void Update();
		static void FrameStart() { s_frameStartTime = std::chrono::steady_clock::now(); }
		static void FrameEnd();

		static float DeltaTime();
		static float TimeSinceStart();

		static void SetTargetFPS(int fps) { s_targetFPS = fps; }
		static int GetTargetFPS() { return s_targetFPS; }

		static float GetNowFPS() { return s_nowFPS; }

	private:
		static std::chrono::steady_clock::time_point s_startTime;
		static std::chrono::steady_clock::time_point s_lastFrameTime;
		static std::chrono::steady_clock::time_point s_frameStartTime;
		static int		s_targetFPS;
		static float	s_deltaTime;

		static int s_frameCount;
		static float s_fpsTimer;
		static float s_nowFPS;
	};
}
