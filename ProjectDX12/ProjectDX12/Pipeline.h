#ifndef ___PIPELINE_H___
#define ___PIPELINE_H___

// System
#include "DirectX.h"

class  Pipeline
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
	struct Description
	{
		// シェーダー
		BOOL					AmpShader = FALSE;
		BOOL					MeshShader = FALSE;
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
		D3D12_CULL_MODE			CullMode = D3D12_CULL_MODE_BACK;
		UINT					RenderTargetNum;
		BOOL					WriteDepth = TRUE;
	};

public:
	Pipeline(Description desc);
	~Pipeline();

public:
	/// <summary>
	/// コマンドリストにパイプラインを設定
	/// </summary>
	void Bind() { GetCommandList()->SetPipelineState(PipelineData.Get()); }

private:
	ComPtr<ID3D12PipelineState> PipelineData;

};

#endif
