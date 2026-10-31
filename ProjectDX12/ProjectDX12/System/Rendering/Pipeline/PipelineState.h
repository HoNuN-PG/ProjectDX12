#ifndef ___PIPELINE_H___
#define ___PIPELINE_H___

#include "System/DirectX.h"

class  PipelineState
{
public:

	struct InputLayout
	{
		const char* name;
		UINT		index;
		DXGI_FORMAT format;
	};
	static InputLayout IED_POS_TEX[];
	static UINT IED_POS_TEX_COUNT;
	static InputLayout IED_POS_NOR_TEX_COLOR[];
	static UINT IED_POS_NOR_TEX_COLOR_COUNT;

public:

	/// <summary>
	/// ブレンドモード
	/// </summary>
	enum class BlendMode
	{
		None,		// ブレンドなし（上書き）
		Alpha,		// アルファブレンド
	};

	struct Description
	{
		// シェーダー
		const wchar_t*			ASFile;
		const wchar_t*			MSFile;
		const wchar_t*			VSFile;
		const wchar_t*			PSFile;

		// ルートシグネチャ
		ID3D12RootSignature*	pRootSignature;

		// インプットレイアウト
		InputLayout*			pInputLayout;
		UINT					InputLayoutNum;

		// パイプライン設定
		D3D12_CULL_MODE				CullMode = D3D12_CULL_MODE_BACK;
		UINT						RenderTargetNum;
		std::vector<DXGI_FORMAT>	RenderTargetFormat;
		BOOL						DepthTest = TRUE;		// 深度テスト
		BOOL						DepthWrite = TRUE;		// 深度書き込み（DepthTest が TRUE の時のみ有効）
		BOOL						DepthClip = TRUE;		// ニア/ファーでのクリップ（FALSE で深度をクランプ）
		BlendMode					Blend = BlendMode::Alpha;
	};

public:

	PipelineState(Description desc);
	~PipelineState();

public:

	/// <summary>
	/// コマンドリストにパイプラインを設定
	/// </summary>
	void Bind() { assert(PipelineData); GetCommandList()->SetPipelineState(PipelineData.Get()); }

private:

	void CreateAmplificationShaderPipelineState(Description desc, 
		D3D12_RASTERIZER_DESC rasterizer, D3D12_BLEND_DESC blend, D3D12_DEPTH_STENCIL_DESC stencil);
	void CreateMeshShaderPipelineState(Description desc, 
		D3D12_RASTERIZER_DESC rasterizer, D3D12_BLEND_DESC blend, D3D12_DEPTH_STENCIL_DESC stencil);
	void CreateDefaultPipelineState(Description desc, 
		D3D12_RASTERIZER_DESC rasterizer, D3D12_BLEND_DESC blend, D3D12_DEPTH_STENCIL_DESC stencil);

private:

	ComPtr<ID3D12PipelineState> PipelineData;

};

#endif
