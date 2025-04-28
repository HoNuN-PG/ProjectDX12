#ifndef ___PIPELINE_H___
#define ___PIPELINE_H___

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
	struct Description
	{
		ID3D12RootSignature*	pRootSignature;
		const wchar_t*			VSFile;
		const wchar_t*			PSFile;
		InputLayout*			pInputLayout;
		UINT					InputLayoutNum;
		UINT					RenderTargetNum;
	};

public:
	Pipeline(Description desc);
	~Pipeline();
	void Bind() {
		GetCommandList()->SetPipelineState(m_pPipeline);
	}

private:
	ID3D12PipelineState* m_pPipeline;

};

#endif
