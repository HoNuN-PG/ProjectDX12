
#include <d3dcompiler.h>
#include <vector>

// System/Rendering/Pipeline
#include "Pipeline.h"

Pipeline::InputLayout Pipeline::IED_POS_TEX[] =
{
	{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
	{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
};
UINT Pipeline::IED_POS_TEX_COUNT = 2;

Pipeline::InputLayout Pipeline::IED_POS_NOR_TEX_COLOR[] =
{
	{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
	{"NORMAL",   0,DXGI_FORMAT_R32G32B32_FLOAT},
	{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
	{"COLOR",    0,DXGI_FORMAT_R32G32B32A32_FLOAT},
};
UINT Pipeline::IED_POS_NOR_TEX_COLOR_COUNT = 4;

Pipeline::Pipeline(Description desc)
{
	// ラスタライザ
	D3D12_RASTERIZER_DESC rasterDesc	= CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterDesc.CullMode					= desc.CullMode;
	rasterDesc.DepthClipEnable			= FALSE;

	// ブレンド
	D3D12_BLEND_DESC blendDesc			= CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	blendDesc.AlphaToCoverageEnable		= false;
	blendDesc.IndependentBlendEnable	= false;
	for (int i = 0; i < desc.RenderTargetNum; ++i)
	{
		blendDesc.RenderTarget[i].BlendEnable				= TRUE;
		blendDesc.RenderTarget[i].SrcBlend					= D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[i].DestBlend					= D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].BlendOp					= D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].SrcBlendAlpha				= D3D12_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha			= D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[i].BlendOpAlpha				= D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].RenderTargetWriteMask		= D3D12_COLOR_WRITE_ENABLE_ALL;
		blendDesc.RenderTarget[i].LogicOpEnable				= false;
	}

	// 深度
	D3D12_DEPTH_STENCIL_DESC dsDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	dsDesc.DepthEnable = desc.WriteDepth;
	if (dsDesc.DepthEnable)
	{
		dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	}

	if (desc.AmpShader)
	{
		CreateAmplificationShaderPipelineState(desc, rasterDesc, blendDesc, dsDesc);
	}
	else if (desc.MeshShader)
	{
		CreateMeshShaderPipelineState(desc, rasterDesc, blendDesc, dsDesc);
	}
	else
	{
		CreateDefaultPipelineState(desc, rasterDesc, blendDesc, dsDesc);
	}
}

Pipeline::~Pipeline()
{
}

void Pipeline::CreateAmplificationShaderPipelineState(Description desc, 
	D3D12_RASTERIZER_DESC rasterizer, D3D12_BLEND_DESC blend, D3D12_DEPTH_STENCIL_DESC stencil)
{
	// シェーダー
	HRESULT hr;
	ID3DBlob* pAS = nullptr, * pMS = nullptr, * pPS = nullptr;
	hr = D3DReadFileToBlob(desc.ASFile, &pAS);
	hr = D3DReadFileToBlob(desc.MSFile, &pMS);
	hr = D3DReadFileToBlob(desc.PSFile, &pPS);
	if (FAILED(hr)) { return; }
	assert(pAS != nullptr);
	assert(pMS != nullptr);
	assert(pPS != nullptr);

	// パイプラインステートの設定
	D3DX12_MESH_SHADER_PIPELINE_STATE_DESC pipelineDesc = {};
	// ルートシグネチャ
	pipelineDesc.pRootSignature = desc.pRootSignature;
	// シェーダ
	pipelineDesc.AS = { pAS->GetBufferPointer(), pAS->GetBufferSize() };
	pipelineDesc.MS = { pMS->GetBufferPointer(), pMS->GetBufferSize() };
	pipelineDesc.PS = { pPS->GetBufferPointer(), pPS->GetBufferSize() };
	// サンプリング
	pipelineDesc.SampleMask = UINT_MAX;
	pipelineDesc.SampleDesc.Count = 1;
	pipelineDesc.SampleDesc.Quality = 0;
	// ラスタライザ
	pipelineDesc.RasterizerState = rasterizer;
	// ブレンド
	pipelineDesc.BlendState = blend;
	// レンダーターゲット
	pipelineDesc.NumRenderTargets = desc.RenderTargetNum;
	for (int i = 0; i < desc.RenderTargetNum; ++i)
	{
		pipelineDesc.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	// 深度バッファ
	pipelineDesc.DepthStencilState = stencil;
	if (stencil.DepthEnable)
	{
		pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	}
	else
	{
		pipelineDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;;
	}

	CD3DX12_PIPELINE_MESH_STATE_STREAM psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(pipelineDesc);

	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
	streamDesc.SizeInBytes = sizeof(psoStream);
	streamDesc.pPipelineStateSubobjectStream = &psoStream;

	// パイプラインの生成
	hr = GetDevice()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(PipelineData.GetAddressOf()));
	if (FAILED(hr)) { return; }
}

void Pipeline::CreateMeshShaderPipelineState(Description desc, 
	D3D12_RASTERIZER_DESC rasterizer, D3D12_BLEND_DESC blend, D3D12_DEPTH_STENCIL_DESC stencil)
{
	// シェーダー
	HRESULT hr;
	ID3DBlob* pMS = nullptr, * pPS = nullptr;
	hr = D3DReadFileToBlob(desc.MSFile, &pMS);
	hr = D3DReadFileToBlob(desc.PSFile, &pPS);
	if (FAILED(hr)) { return; }
	assert(pMS != nullptr);
	assert(pPS != nullptr);

	// パイプラインステートの設定
	D3DX12_MESH_SHADER_PIPELINE_STATE_DESC pipelineDesc = {};
	// ルートシグネチャ
	pipelineDesc.pRootSignature = desc.pRootSignature;
	// シェーダ
	pipelineDesc.MS = { pMS->GetBufferPointer(), pMS->GetBufferSize() };
	pipelineDesc.PS = { pPS->GetBufferPointer(), pPS->GetBufferSize() };
	// サンプリング
	pipelineDesc.SampleMask = UINT_MAX;
	pipelineDesc.SampleDesc.Count = 1;
	pipelineDesc.SampleDesc.Quality = 0;
	// ラスタライザ
	pipelineDesc.RasterizerState = rasterizer;
	// ブレンド
	pipelineDesc.BlendState = blend;
	// レンダーターゲット
	pipelineDesc.NumRenderTargets = desc.RenderTargetNum;
	for (int i = 0; i < desc.RenderTargetNum; ++i)
	{
		pipelineDesc.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	// 深度バッファ
	pipelineDesc.DepthStencilState = stencil;
	if (stencil.DepthEnable)
	{
		pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	}
	else
	{
		pipelineDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;;
	}

	CD3DX12_PIPELINE_MESH_STATE_STREAM psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(pipelineDesc);

	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
	streamDesc.SizeInBytes = sizeof(psoStream);
	streamDesc.pPipelineStateSubobjectStream = &psoStream;

	// パイプラインの生成
	hr = GetDevice()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(PipelineData.GetAddressOf()));
	if (FAILED(hr)) { return; }
}

void Pipeline::CreateDefaultPipelineState(Description desc, 
	D3D12_RASTERIZER_DESC rasterizer, D3D12_BLEND_DESC blend, D3D12_DEPTH_STENCIL_DESC stencil)
{
	// シェーダー
	HRESULT hr;
	ID3DBlob* pVS = nullptr, * pPS = nullptr;
	hr = D3DReadFileToBlob(desc.VSFile, &pVS);
	hr = D3DReadFileToBlob(desc.PSFile, &pPS);
	if (FAILED(hr)) { return; }
	assert(pVS != nullptr);
	assert(pPS != nullptr);

	// インプットレイアウト
	std::vector<D3D12_INPUT_ELEMENT_DESC> element;
	element.resize(desc.InputLayoutNum);
	for (int i = 0; i < desc.InputLayoutNum; ++i)
	{
		element[i].SemanticName = desc.pInputLayout[i].name;
		element[i].SemanticIndex = desc.pInputLayout[i].index;
		element[i].Format = desc.pInputLayout[i].format;
		element[i].InputSlot = 0;
		element[i].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		element[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
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
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	pipelineDesc.SampleDesc.Count = 1;
	pipelineDesc.SampleDesc.Quality = 0;
	// ラスタライザ
	pipelineDesc.RasterizerState = rasterizer;
	// ブレンド
	pipelineDesc.BlendState = blend;
	// プリミティブ
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// レンダーターゲット
	pipelineDesc.NumRenderTargets = desc.RenderTargetNum;
	for (int i = 0; i < desc.RenderTargetNum; ++i)
	{
		pipelineDesc.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	// 深度バッファ
	pipelineDesc.DepthStencilState = stencil;
	if (stencil.DepthEnable)
	{
		pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	}
	else
	{
		pipelineDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;;
	}

	// パイプラインの生成
	hr = GetDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(PipelineData.GetAddressOf()));
	if (FAILED(hr)) { return; }
}
