#pragma once
using json = nlohmann::json;

struct PlayerData 
{
	std::string modelPath;
	bool isDynamic = true;
	Math::Vector3 pos{ 0,0,0 };
	Math::Vector3 rot{ 0,0,0 };
	Math::Vector3 scale{ 1,1,1 };

	json ToJson() const 
	{
		return 
		{
			{"modelPath", modelPath},
			{"isDynamic", isDynamic},
			{"pos", {pos.x,pos.y,pos.z}},
			{"rot", {rot.x,rot.y,rot.z}},
			{"scale", {scale.x,scale.y,scale.z}}
		};
	}
	static PlayerData FromJson(const json& j)
	{
		PlayerData p;
		p.modelPath = j.value("modelPath", "");
		p.isDynamic = j.value("isDynamic", true);
		auto pos = j.value("pos", std::vector<float>{0, 0, 0});
		auto rot = j.value("rot", std::vector<float>{0, 0, 0});
		auto scl = j.value("scale", std::vector<float>{1, 1, 1});
		if (pos.size() == 3) p.pos = { pos[0], pos[1], pos[2] };
		if (rot.size() == 3) p.rot = { rot[0], rot[1], rot[2] };
		if (scl.size() == 3) p.scale = { scl[0], scl[1], scl[2] };
		return p;
	}
};