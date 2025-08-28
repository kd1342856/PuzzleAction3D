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

	// グリッド幅
	void SetGrid(float g) { m_grid = (g > 0.01f) ? g : 1.0f; }
	void SetBlockModelPath(const std::string& path) { m_blockModelPath = path; }


	void SetActiveCamera(const std::shared_ptr<CameraBase>& cam) { m_wpActiveCam = cam; }


private:
	// レイ（画面中心からのレイ）生成：各プロジェクトのカメラに合わせて実装差し替え
	bool MakeCenterRayFromMainCamera(Math::Vector3& outRayPos, Math::Vector3& outRayDir) const;

	// ゴーストの作成/更新
	void EnsureGhost();
	void UpdateGhostAt(const Math::Vector3& pos);
	void HideGhost();

	// 1ブロック生成
	std::shared_ptr<Entity> CreateBlock(const Math::Vector3& pos);

	bool PickPlacePoint(Math::Vector3& outPos);
	bool RaycastGroundOrBlocks(const Math::Vector3& r0, const Math::Vector3& dir, Math::Vector3& outPos);

private:
	bool m_enabled = true;
	float m_layerY = 0.0f;

	// 設置対象とする衝突相手
	std::vector<std::weak_ptr<Entity>> m_hitEntities;

	// 設置プレビュー
	std::shared_ptr<Entity> m_previewEntity;

	// パラメータ
	float m_grid = 1.0f;
	std::string m_blockModelPath = "Asset/Models/Stage/Block/Block.gltf";

	// 設置履歴（削除用）
	std::vector<std::weak_ptr<Entity>> m_placedBlocks;

	//	カメラ
	std::weak_ptr<CameraBase> m_wpActiveCam;

	bool PickOnGround(Math::Vector3& outHitPos);
	void PlaceBlockAt(const Math::Vector3& posSnapped);

	bool m_prevLMB = false;
};