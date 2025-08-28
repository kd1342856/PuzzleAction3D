#pragma once
class Entity;
namespace EntityFactory
{
	std::shared_ptr<Entity> CreatePlayer();
	std::shared_ptr<Entity> CreateBlock();

	std::shared_ptr<Entity> CreateModelEntity(
		const std::string& name,
		const std::string& modelPath,
		const Math::Vector3& pos = { 0,0,0 },
		const Math::Vector3& rot = { 0,0,0 },
		const Math::Vector3& scl = { 1,1,1 },
		bool dynamic = false						// trueなら SetModelWork()、falseなら SetModelData()
	);
}