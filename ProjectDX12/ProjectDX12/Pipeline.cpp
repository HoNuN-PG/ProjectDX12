
#include "Pipeline.h"

#include <d3dcompiler.h>
#include <vector>

Pipeline::Pipeline(Description desc)
{
	// ラスタライザステート
	D3D12_RASTERIZER_DESC rasterDesc	= {};
	rasterDesc.MultisampleEnable		= false;
	rasterDesc.CullMode					= D3D12_CULL_MODE_NONE;
	rasterDesc.FillMode					= D3D12_FILL_MODE_SOLID;
	rasterDesc.DepthClipEnable			= false;

	// ブレンド
	D3D12_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable					= false;
	blendDesc.IndependentBlendEnable				= false;
	blendDesc.RenderTarget[0].BlendEnable			= TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].LogicOpEnable = false;

	// 深度
	D3D12_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable				= TRUE;
	dsDesc.DepthWriteMask			= D3D12_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc				= D3D12_COMPARISON_FUNC_LESS_EQUAL;
	dsDesc.StencilEnable			= FALSE;

	// シェーダー
	HRESULT hr;
	ID3DBlob* pVS, * pPS;
	hr = D3DReadFileToBlob(desc.VSFile, &pVS);
	if (FAILED(hr)) { return; }
	hr = D3DReadFileToBlob(desc.PSFile, &pPS);
	if (FAILED(hr)) { return; }

	// インプットレイアウト
	std::vector<D3D12_INPUT_ELEMENT_DESC> element;
	element.resize(desc.InputLayoutNum);
	for (int i = 0; i < desc.InputLayoutNum; ++i)
	{
		element[i].SemanticName			= desc.pInputLayout[i].name;
		element[i].SemanticIndex		= desc.pInputLayout[i].index;
		element[i].Format				= desc.pInputLayout[i].format;
		element[i].InputSlot			= 0;
		element[i].AlignedByteOffset	= D3D12_APPEND_ALIGNED_ELEMENT;
		element[i].InputSlotClass		= D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		element[i].InstanceDataStepRate = 0;
	}

	// パイプラインステートの設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
	// ルートシグネチャ
	pipelineDesc.pRootSignature = desc.pRootSignature;
	// シェーダ
	pipelineDesc.VS = { pVS->GetBufferPointer(), pVS->GetBufferSize() };
	pipelineDesc.PS = { pPS->GetBufferPointer(), pPS->GetBufferSize() };
	// 頂点レイアウト
	pipelineDesc.InputLayout = { element.data(), desc.InputLayoutNum };
	// サンプリング
	pipelineDesc.SampleMask			= D3D12_DEFAULT_SAMPLE_MASK;
	pipelineDesc.SampleDesc.Count	= 1;
	pipelineDesc.SampleDesc.Quality = 0;
	// ラスタライザ
	pipelineDesc.RasterizerState = rasterDesc;
	// ブレンド
	pipelineDesc.BlendState = blendDesc;
	// プリミティブ
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// レンダーターゲット
	pipelineDesc.NumRenderTargets		= desc.RenderTargetNum;
	for(int i = 0;i < desc.RenderTargetNum;++i)
		pipelineDesc.RTVFormats[i]		= DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineDesc.DSVFormat				= DXGI_FORMAT_UNKNOWN;
	// 深度バッファ
	pipelineDesc.DepthStencilState	= dsDesc;
	pipelineDesc.DSVFormat			= DXGI_FORMAT_D32_FLOAT;

	// パイプラインの生成
	hr = GetDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&PipelineData));
	if (FAILED(hr)) { return; }
}

Pipeline::~Pipeline()
{
	PipelineData->Release();
}
