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

	struct Description
	{
		ID3D12RootSignature*	pRootSignature;
		const wchar_t*			VSFile;
		const wchar_t*			PSFile;
		InputLayout*			pInputLayout;
		UINT					InputLayoutNum;
		UINT					RenderTargetNum;

		D3D12_CULL_MODE			CullMode;
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
