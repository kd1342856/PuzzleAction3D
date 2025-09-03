#pragma once
class Entity;
class TPSCamera;
class CameraBase;
namespace SceneWire
{
	void WirePlayer(
		const std::shared_ptr<Entity>& player, 
		const std::shared_ptr<TPSCamera>& tpsCam,
		const std::shared_ptr<CameraBase>& overheadCam);
	void WireMap(
		const std::shared_ptr<Entity>& map,
		const std::vector<std::shared_ptr<Entity>>& extraHitTargets);

	void ApplyModeForPlayer(const std::shared_ptr<Entity>& player, bool isGameMode);

	void SyncPlayerYawFromTPS(const std::shared_ptr<Entity>& player,
		const std::shared_ptr<CameraBase>& tpsCam);
}