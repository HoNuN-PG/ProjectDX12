#ifndef ___LIGHT_BASE_H___
#define ___LIGHT_BASE_H___

#include "GameObject.h"

#include "imgui/imgui.h"

#define LIGHT_LENGTH (1000)

class LightBase : public GameObject
{
private:
	/**
	 * @struct ライト回転パラメータ
	 */
	struct tLigRad
	{
		float radY, radXZ;		// 回り込み角度
		float lateY, lateXZ;	// 補間角度（現在の角度）
	};

public:
	LightBase();
	~LightBase() {}
	virtual void Init() override {}
	virtual void Uninit() override {}
	virtual void Update() override;
	virtual void Draw() override;

	/**
	 * パラメーター取得
	 */
	DirectX::XMFLOAT3 GetPos() { return m_Position; }
	DirectX::XMFLOAT3 GetTarget() { return m_Target; }
	DirectX::XMFLOAT3 GetDir() { return { m_Target.x - m_Position.x,m_Target.y - m_Position.y, m_Target.z - m_Position.z }; }
	DirectX::XMFLOAT4 GetColor() { return { m_Color.x,m_Color.y,m_Color.z,m_Color.w }; }
	float GetPower() { return m_Power; }
	float GetAmbient() { return m_Ambient; }

	/**
	 * ライトカメラマトリクス取得
	 */
	static DirectX::XMFLOAT4X4 GetLightViewMat() { return m_ViewMatrix; }
	static DirectX::XMFLOAT4X4 GetLightProjectionMat() { return m_ProjectionMatrix; }
	static DirectX::XMFLOAT4X4 GetLightViewProjectionMat();

private:
	/**
	 * ライトパラメータ
	 */
	DirectX::XMFLOAT3 m_Up;				// アップベクトル
	DirectX::XMFLOAT3 m_Position;
	DirectX::XMFLOAT3 m_Target;
	DirectX::XMFLOAT3 m_TargetOffset;	// 注視点オフセット
	tLigRad m_Rad;						// 角度パラメーター
	float m_Dist;						// 距離（注視点との）
	ImVec4 m_Color;						// 色	
	float m_Power;						// 強さ
	float m_Ambient;					// 環境光

private:
	static DirectX::XMFLOAT4X4 m_ViewMatrix;
	static DirectX::XMFLOAT4X4 m_ProjectionMatrix;

};

#endif