#include "TaskManager.h"
using namespace EngineCore;

std::vector<std::thread> TaskManager::s_workers;
std::queue<std::function<void()>> TaskManager::s_tasks;
std::mutex TaskManager::s_mutex;
std::condition_variable TaskManager::s_condition;
bool TaskManager::s_running = false;

void TaskManager::Init()
{
	s_running = true;
	size_t threadCount = std::thread::hardware_concurrency();
	for (size_t i = 0; i < threadCount; ++i)
	{
		s_workers.emplace_back([]()
		{
			while (s_running)
			{
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(s_mutex);
					s_condition.wait(lock, [] {return !s_tasks.empty() || !s_running; });
					if (!s_running && s_tasks.empty())return;
					task = std::move(s_tasks.front());
					s_tasks.pop();
				}
				try
				{
					task();
				}
				catch (const std::exception& e)
				{
					Logger::Error("TaskManager: Task failed with exception: " + std::string(e.what()));
				}
			}
		});
	}
}

void TaskManager::Shutdown(ShutdownMode mode)
{
	//	グレースフルシャットダウン
	if (mode == ShutdownMode::Graceful)
	{
		while (true)
		{
			std::unique_lock<std::mutex> lock(s_mutex);
			if (s_tasks.empty())
			{
				break;
			}
			lock.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}

	{
		std::unique_lock<std::mutex> lock(s_mutex);
		s_running = false;
	}
	s_condition.notify_all();
	for (auto& worker : s_workers)
	{
		if (worker.joinable())worker.join();
	}
	s_workers.clear();
}

void TaskManager::Submit(std::function<void()>task)
{ 
	{
		std::unique_lock<std::mutex>lock(s_mutex);
		s_tasks.push(std::move(task));
	}
	s_condition.notify_one();
}

void TaskManager::Update()
{
	//	For main-thread tasks if needed
}