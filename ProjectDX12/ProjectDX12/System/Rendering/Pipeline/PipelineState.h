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
	static constexpr InputLayout IED_POS_TEX[] =
	{
		{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
		{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
	};
	static constexpr UINT IED_POS_TEX_COUNT = _countof(IED_POS_TEX);
	static constexpr InputLayout IED_POS_NOR_TEX_COLOR[] =
	{
		{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
		{"NORMAL",   0,DXGI_FORMAT_R32G32B32_FLOAT},
		{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
		{"COLOR",    0,DXGI_FORMAT_R32G32B32A32_FLOAT},
	};
	static constexpr UINT IED_POS_NOR_TEX_COLOR_COUNT = _countof(IED_POS_NOR_TEX_COLOR);

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
		// シェーダー（AS/MSのどちらかがあればメッシュシェーダーパイプライン）
		const wchar_t*			ASFile;
		const wchar_t*			MSFile;
		const wchar_t*			VSFile;
		const wchar_t*			PSFile;

		// ルートシグネチャ
		ID3D12RootSignature*	pRootSignature;

		// インプットレイアウト
		const InputLayout*		pInputLayout;
		UINT					InputLayoutNum;

		// パイプライン設定
		D3D12_CULL_MODE				CullMode = D3D12_CULL_MODE_BACK;
		std::vector<DXGI_FORMAT>	RenderTargetFormat;		// 要素数がレンダーターゲット数
		BOOL						DepthTest = TRUE;		// 深度テスト
		BOOL						DepthWrite = TRUE;		// 深度書き込み（DepthTest が TRUE の時のみ有効）
		BOOL						DepthClip = TRUE;		// ニア/ファーでのクリップ（FALSE で深度をクランプ）
		BlendMode					Blend = BlendMode::Alpha;
		DXGI_FORMAT					DSVFormat = DXGI_FORMAT_D32_FLOAT;	// DepthTest が TRUE の時のみ使用
	};

public:

	PipelineState(const Description& desc);
	~PipelineState();

public:

	/// <summary>
	/// コマンドリストにパイプラインを設定
	/// </summary>
	void Bind() { assert(PipelineData); GetCommandList()->SetPipelineState(PipelineData.Get()); }

private:

	void CreateDefaultPipelineState(const Description& desc,
	const D3D12_RASTERIZER_DESC& rasterizer, const D3D12_BLEND_DESC& blend, const D3D12_DEPTH_STENCIL_DESC& stencil);
	void CreateMeshShaderPipelineState(const Description& desc,
		const D3D12_RASTERIZER_DESC& rasterizer, const D3D12_BLEND_DESC& blend, const D3D12_DEPTH_STENCIL_DESC& stencil);

private:

	ComPtr<ID3D12PipelineState> PipelineData;

};

#endif
