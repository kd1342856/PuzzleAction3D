#pragma once
#include "ThreadSafeQueue.h"
class Entity;
class LoadedEntityQueue
{
public:
	void Push(const std::shared_ptr<Entity>& entity) { m_queue.Push(entity); }

	bool TryPop(std::shared_ptr<Entity>& out) { return m_queue.TryPop(out); }
	
	bool Empty()const { return m_queue.Empty(); }

private:
	ThreadSafeQueue<std::shared_ptr<Entity>> m_queue;
private:
	LoadedEntityQueue() {}
	~LoadedEntityQueue() {}
public:
	static LoadedEntityQueue& Instance()
	{
		static LoadedEntityQueue instance;
		return instance;
	}
};
