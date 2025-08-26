#include "MainThreadTask.h"

void MainThreadTask::Enqueue(std::function<void()> fn)
{
	std::lock_guard<std::mutex> lk(m_mtx);
	m_q.push(std::move(fn));
}

void MainThreadTask::Drain(size_t max)
{
	for (size_t i = 0; i < max; ++i)
	{
		std::function<void()> fn;
		{
			std::lock_guard<std::mutex> lk(m_mtx);
			if (m_q.empty()) break;
			fn = std::move(m_q.front()); m_q.pop();
		}
		fn();
	}
}
