#pragma once
#include <typeindex>
class Component;
enum class DrawPass;
class Entity : public KdGameObject
{
public:
	enum class VisibilityFlags :uint8_t
	{
		None   = 0,
		Lit	   = 1 << 0,
		UnLit  = 1 << 1,
		Bright = 1 << 2,
		Shadow = 1 << 3,
	};

	void Init()override;
	void Update()override;
	void DrawLit()override;
	void DrawUnLit()override;
	void DrawBright()override;
	void GenerateDepthMapFromLight()override;

	virtual void Draw();


	void SetVisible(bool visible) { m_visible = visible; }

	void SetVisibility(VisibilityFlags flag, bool enabled);
	bool IsVisible(VisibilityFlags flag)const { return (m_visibility & static_cast<uint8_t>(flag)) != 0; }

	void SetName(const std::string& name) { m_name = name; }
	const std::string& GetName() { return m_name; }

	template<typename T>
	void AddComponent(std::shared_ptr<Component> comp);

	template<typename T>
	bool HasComponent() const;

	template<typename T>
		T& GetComponent();

private:
	std::string m_name = "Unnamed";
	std::unordered_map<std::type_index, std::shared_ptr<Component>> m_components;
	bool m_visible = true;
	uint8_t m_visibility = static_cast<uint8_t>(VisibilityFlags::Lit);
};
#include "Entity.inl"
