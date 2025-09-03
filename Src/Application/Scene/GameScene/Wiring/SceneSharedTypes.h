#pragma once
class Entity;
class CameraBase;
class TPSCamera;
struct EditorSyncArgs
{
	std::vector<std::shared_ptr<Entity>> sceneEntities;
	std::shared_ptr<TPSCamera> tpsCam;
	std::shared_ptr<CameraBase>overheadCam;
};

struct OverheadCamPose
{
	bool has = false;
	Math::Vector3 pos = {};
	Math::Vector3 rot = {};
};