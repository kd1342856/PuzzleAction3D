#pragma once
namespace EngineCore
{
	class TaskManager
	{
	public:
		enum class ShutdownMode{ Immediate, Graceful };

		static void Init();
		static void Shutdown(ShutdownMode mode);
		static void Submit(std::function<void()> task);
		static void Update();

	private:
		static std::vector<std::thread> s_workers;
		static std::queue<std::function<void()>> s_tasks;
		static std::mutex s_mutex;
		static std::condition_variable s_condition;
		static bool s_running;
	};
}