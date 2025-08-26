#pragma once
#include "Framework/Direct3D/KdTexture.h"

namespace EngineCore 
{

	class Engine {
	public:
		static Engine& Instance() 
		{
			static Engine instance;
			return instance;
		}

		bool Init();         // 初期化
		void Shutdown();     // 終了処理
		void Update();       // 毎フレーム更新
		void PostUpdate();
		void Draw();         // 毎フレーム描画
		void Release();
		bool m_isCameraControlActive = false;
		void SetMouseGrabbed(bool grab);
		bool IsMouseGrabbed() const { return m_mouseGrabbed; }

		void EnsureGameRTSize(int w, int h); // 追加
		void UpdateCameraProjectionForGameRT(); // 追加

		std::shared_ptr<KdTexture> m_gameViewRT;
		bool m_mouseGrabbed = false;

	private:
		Engine(){}
		~Engine() { Release(); }

	};
}
