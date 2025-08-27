#pragma once
#include "../../Component.h"

class Entity;
class ColliderComponent;
class RenderComponent;
class CameraBase;

class BuildModeComponent : public Component
{
public:
	void Init() override;
	void Update() override;
	void PostUpdate() override {} // 未使用
	void SetEnabled(bool e) { m_enabled = e; }
	bool IsEnabled() const { return m_enabled; }

	// 設置の当たり対象（地面や既設ブロックなど）
	void RegisterHitEntity(const std::shared_ptr<Entity>& e) { m_hitEntities.push_back(e); }
	void ClearHitEntities() { m_hitEntities.clear(); }

	// 設置用モデル
	void SetBlockModelPath(const std::string& path) { m_blockModelPath = path; }

	// グリッド幅
	void SetGrid(float g) { m_grid = (g > 0.01f) ? g : 1.0f; }

	void SetActiveCamera(const std::shared_ptr<CameraBase>& cam) { m_wpActiveCam = cam; }


private:
	// レイ（画面中心からのレイ）生成：各プロジェクトのカメラに合わせて実装差し替え
	bool MakeCenterRayFromMainCamera(Math::Vector3& outRayPos, Math::Vector3& outRayDir) const;

	// 地面(TypeGround)へヒットした座標を得る
	bool PickGround(Math::Vector3& outPos) const;

	// ゴーストの作成/更新
	void EnsureGhost();
	void UpdateGhostAt(const Math::Vector3& pos);
	void HideGhost();

	// 1ブロック生成
	std::shared_ptr<Entity> CreateBlock(const Math::Vector3& pos);

private:
	bool m_enabled = true;

	// 設置対象とする衝突相手
	std::vector<std::weak_ptr<Entity>> m_hitEntities;

	// 設置プレビュー
	std::shared_ptr<Entity> m_previewEntity;

	// パラメータ
	float m_grid = 1.0f;
	std::string m_blockModelPath = "Asset/Models/Stage/Block/Block.gltf";

	// 設置履歴（削除用）
	std::vector<std::weak_ptr<Entity>> m_placedBlocks;

	std::weak_ptr<CameraBase> m_wpActiveCam;

};