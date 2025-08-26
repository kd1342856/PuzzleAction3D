#pragma once
using json = nlohmann::json;
class Entity;
struct ObjData
{
	std::string name;
	Math::Vector3 pos;
	Math::Vector3 rot;
	Math::Vector3 scale;

	bool isLit = false;
	bool isUnLit = false;
	bool isBright = false;
	bool isShadow = false;

	json ToJson()const
	{
		return
		{
			{"name", name},
			{"pos", { pos.x, pos.y, pos.z }},
			{"rot", { rot.x, rot.y, rot.z }},
			{"scale", { scale.x, scale.y, scale.z }},
			{"isLit", isLit },
			{"isUnLit", isUnLit },
			{"isBrightLit", isBright },
			{"isGenerateDepthMapFromLight", isShadow }
		};
	}

	static ObjData FromJson(const json& j)
	{
		ObjData obj;
		obj.name = j.at("name");
		auto pos = j.at("pos");
		auto rot = j.at("rot");
		auto scale = j.at("scale");
		obj.pos = { pos[0], pos[1], pos[2] };
		obj.rot = { rot[0], rot[1], rot[2] };
		obj.scale = { scale[0], scale[1], scale[2] };

		obj.isLit = j.value("isLit", false);
		obj.isUnLit = j.value("isUnLit", false);
		obj.isBright = j.value("isBrightLit", false);
		obj.isShadow = j.value("isGenerateDepthMapFromLight", false);
		return obj;
	}
};
class ObjectData
{
public:
	ObjectData(){}
	~ObjectData(){}
	std::vector<ObjData> ConvertToDataList(const std::vector<std::shared_ptr<Entity>>& entityList);

	void SaveObj(const std::vector<ObjData>& objects, const std::string& filePath);
	std::vector<ObjData> LoadJson(const std::string& filePath);
	std::vector<std::shared_ptr<Entity>> LoadEntityList(const std::string& filePath);
	
private:


};
