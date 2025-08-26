#pragma once
struct GameStageSpec 
{
	// Map
	std::string mapModelPath;     
	std::string objDataPath;


	// Player
	std::string playerModelPath;   // 例: "Asset/Models/Player/Player.gltf"（固定なら空でもOK）
	bool        playerDynamic = false; // 動的モデルなら true
	
	std::string spawnJsonPath;  
	std::string playerStatePath;
};

class GameScene;

namespace StageLoader
{
	// 非同期で Map / Player を生成して LoadedEntityQueue に積む
	// スポーンは MainThread で GameScene に適用
	void LoadStageAsync(const GameStageSpec& spec, GameScene* scene);
}