#pragma once
class MainThreadTask 
{
public:
	static MainThreadTask& Instance() { static MainThreadTask inst; return inst; }

	void Enqueue(std::function<void()> fn);

	// 毎フレーム呼ぶ：溜まったタスクを実行
	void Drain(size_t max = 32);
private:
	std::mutex m_mtx;
	std::queue<std::function<void()>> m_q;
};