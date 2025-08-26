#pragma once
#include "../Component.h"
class RenderComponent :public Component
{
public:
	enum class ModelType
	{
		None,
		Static,
		Dynamic
	};
	RenderComponent() = default;
	virtual ~RenderComponent() = default;

	void SetColorRate(const Math::Color& color) { m_colorRate = color; }
	void SetEmissive(const Math::Vector3& emissive) { m_emissive = emissive; }

	// 描画
	void Draw()override;
	std::shared_ptr<KdModelWork> GetModelWork() { return m_modelWork; }
	std::shared_ptr<KdModelData> GetModelData() { return m_modelData; }
	void SetModelData(const std::string & filePath);
	void SetModelWork(const std::string & filePath);

	// 追加：ロード済みデータを後から注入
	void SetModelData(std::shared_ptr<KdModelData> data)
	{
		m_modelData = std::move(data);
		m_modelType = ModelType::Static;
	}
	void SetModelWork(std::shared_ptr<KdModelData> data) 
	{
		if (!m_modelWork) m_modelWork = std::make_shared<KdModelWork>();
		m_modelWork->SetModelData(data);
		m_modelType = ModelType::Dynamic;
	}

private:
	
	ModelType m_modelType = ModelType::None;
	std::shared_ptr<KdModelData> m_modelData = nullptr;
	std::shared_ptr<KdModelWork> m_modelWork = nullptr;
	Math::Color m_colorRate = Math::Color(1, 1, 1, 1);
	Math::Vector3 m_emissive = Math::Vector3::Zero;
};
