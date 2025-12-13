#ifndef ___LIGHT_BASE_H___
#define ___LIGHT_BASE_H___

// ImGUI
#include "imgui/imgui.h"

// System/GameObject
#include "GameObject.h"

#define LIGHT_LENGTH (1000)

class LightBase : public GameObject
{
private:
	/// <summary>
	/// ライト回転パラメータ
	/// </summary>
	struct tLigRad
	{
		float radY, radXZ;		// 角度
		float lateY, lateXZ;	// 補間角度（現在の角度）
	};

public:
	LightBase();
	~LightBase() {}
	virtual void Init() override {}
	virtual void Uninit() override {}
	virtual void Update() override;
	virtual void Draw() override;

	// 各種ライトパラメータ
public:
	DirectX::XMFLOAT3 GetPosition() { return m_Position; }
	DirectX::XMFLOAT3 GetTarget() { return m_Target; }
	DirectX::XMFLOAT3 GetDirection() { return { m_Target.x - m_Position.x,m_Target.y - m_Position.y, m_Target.z - m_Position.z }; }
	DirectX::XMFLOAT4 GetColor() { return { m_Color.x,m_Color.y,m_Color.z,m_Color.w }; }
	float GetPower() { return m_Power; }
	float GetAmbient() { return m_Ambient; }

	// マトリクス
public:
	static DirectX::XMFLOAT4X4 GetLightViewMat() { return m_ViewMatrix; }
	static DirectX::XMFLOAT4X4 GetLightProjectionMat() { return m_ProjectionMatrix; }
	static DirectX::XMFLOAT4X4 GetLightViewProjectionMat();
private:
	static DirectX::XMFLOAT4X4 m_ViewMatrix;
	static DirectX::XMFLOAT4X4 m_ProjectionMatrix;

	// ライトパラメータ
private:
	DirectX::XMFLOAT3 m_Position;
	DirectX::XMFLOAT3 m_Target;
	DirectX::XMFLOAT3 m_TargetOffset;	// 注視点オフセット
	DirectX::XMFLOAT3 m_Up;				// アップベクトル
	tLigRad m_Rad;						// 角度パラメーター
	float m_Dist;						// 距離（注視点との）
	ImVec4 m_Color;						// 色	
	float m_Power;						// 強さ
	float m_Ambient;					// 環境光

};

#endif