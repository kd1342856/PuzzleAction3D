#pragma once
template<typename T>
class ThreadSafeQueue
{
public:

	void Push(const T& value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_queue.push(value);
	}

	bool TryPop(T& out)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_queue.empty())return false;
		out = std::move(m_queue.front());
		m_queue.pop();
		return true;
	}

	bool Empty()const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_queue.empty();
	}

private:
	mutable std::queue<T> m_queue;
	mutable std::mutex m_mutex;
};
