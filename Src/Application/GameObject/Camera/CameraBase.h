#pragma once

class CameraBase : public KdGameObject
{
public:
	CameraBase()						{}
	virtual ~CameraBase()	override	{}

	void Init()				override;
	void PreDraw()			override;
	
	void SetTarget(const std::shared_ptr<KdGameObject>& target);

	// 「絶対変更しません！見るだけ！」な書き方
	const std::shared_ptr<KdCamera>& GetCamera() const
	{
		return m_spCamera;
	}

	// 「中身弄るかもね」な書き方
	std::shared_ptr<KdCamera> WorkCamera() const
	{
		return m_spCamera;
	}

	const Math::Matrix GetRotationMatrix()const
	{
		return Math::Matrix::CreateFromYawPitchRoll(
		       DirectX::XMConvertToRadians(m_DegAng.y),
		       DirectX::XMConvertToRadians(m_DegAng.x),
		       DirectX::XMConvertToRadians(m_DegAng.z));
	}

	const Math::Matrix GetRotationYMatrix() const
	{
		return Math::Matrix::CreateRotationY(
			   DirectX::XMConvertToRadians(m_DegAng.y));
	}

	void RegistHitObject(const std::shared_ptr<KdGameObject>& object)
	{
		m_wpHitObjectList.push_back(object);
	}

	void SetActive(bool b) 
	{
		m_active = b; 
	}
	bool IsActive() const 
	{ 
		return m_active;
	}

	Math::Matrix GetViewMatrix() const
	{
		return m_spCamera ? m_spCamera->GetCameraViewMatrix() : Math::Matrix::Identity;
	}
	Math::Matrix GetProjMatrix() const
	{
		return m_spCamera ? m_spCamera->GetProjMatrix() : Math::Matrix::Identity;
	}

	Math::Vector3 GetPosition() const 
	{
		return { m_mLocalPos._41, m_mLocalPos._42, m_mLocalPos._43 };
	}
	void SetPosition(const Math::Vector3& p) 
	{
		m_mLocalPos._41 = p.x; m_mLocalPos._42 = p.y; m_mLocalPos._43 = p.z;
	}

	Math::Vector3 GetEulerDeg() const { return m_DegAng; }
	void SetEulerDeg(const Math::Vector3& deg) { m_DegAng = deg; }

protected:
	// カメラ回転用角度
	Math::Vector3								m_DegAng		= Math::Vector3::Zero;

	void UpdateRotateByMouse();
	void SyncMouseAnchorOnGrabBegin();

	std::shared_ptr<KdCamera>					m_spCamera		= nullptr;
	std::weak_ptr<KdGameObject>					m_wpTarget;
	std::vector<std::weak_ptr<KdGameObject>>	m_wpHitObjectList{};

	Math::Matrix								m_mLocalPos		= Math::Matrix::Identity;
	Math::Matrix								m_mRotation		= Math::Matrix::Identity;

	// カメラ回転用マウス座標の差分
	POINT										m_FixMousePos{};
	
	bool m_active = true;
};