#pragma once
#include "../Player/PlayerData.h"
#include "../ObjData.h"
struct SceneData {
	PlayerData player;
	std::vector<ObjData> objects;

	nlohmann::json ToJson() const {
		nlohmann::json j;
		j["player"] = player.ToJson();
		j["objects"] = nlohmann::json::array();
		for (const auto& o : objects) j["objects"].push_back(o.ToJson());
		return j;
	}
	static SceneData FromJson(const nlohmann::json& j) {
		SceneData s;
		if (j.contains("player"))  s.player = PlayerData::FromJson(j["player"]);
		if (j.contains("objects")) {
			for (const auto& e : j["objects"]) s.objects.push_back(ObjData::FromJson(e));
		}
		return s;
	}
};