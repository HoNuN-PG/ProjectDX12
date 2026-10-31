
#include <d3dcompiler.h>
#include <cstdio>
#include <vector>

#include "System/Rendering/Pipeline/PipelineState.h"

namespace
{
	/// <summary>
	/// 失敗時に内容をデバッグ出力して停止する
	/// </summary>
	bool CheckResult(HRESULT hr, const wchar_t* message, const wchar_t* file)
	{
		if (SUCCEEDED(hr)) { return true; }

		wchar_t text[512];
		swprintf_s(text, L"[PipelineState] %ls (hr=0x%08X) %ls\n", message, static_cast<unsigned int>(hr), file ? file : L"");
		OutputDebugStringW(text);
		assert(!"PipelineState creation failed");
		return false;
	}

	/// <summary>
	/// シェーダーファイルが指定されているか
	/// </summary>
	bool HasFile(const wchar_t* file)
	{
		return file != nullptr && file[0] != L'\0';
	}

	/// <summary>
	/// メッシュシェーダー/通常パイプライン共通の設定
	/// </summary>
	template<typename T>
	void SetCommonDesc(T& pipelineDesc, const PipelineState::Description& desc,
		const D3D12_RASTERIZER_DESC& rasterizer, const D3D12_BLEND_DESC& blend, const D3D12_DEPTH_STENCIL_DESC& stencil)
	{
		// ルートシグネチャ
		pipelineDesc.pRootSignature = desc.pRootSignature;
		// サンプリング
		pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		pipelineDesc.SampleDesc.Count = 1;
		pipelineDesc.SampleDesc.Quality = 0;
		// ラスタライザ
		pipelineDesc.RasterizerState = rasterizer;
		// ブレンド
		pipelineDesc.BlendState = blend;
		// レンダーターゲット
		pipelineDesc.NumRenderTargets = static_cast<UINT>(desc.RenderTargetFormat.size());
		for (UINT i = 0; i < pipelineDesc.NumRenderTargets; ++i)
		{
			pipelineDesc.RTVFormats[i] = desc.RenderTargetFormat[i];
		}
		// 深度バッファ
		pipelineDesc.DepthStencilState = stencil;
		pipelineDesc.DSVFormat = stencil.DepthEnable ? desc.DSVFormat : DXGI_FORMAT_UNKNOWN;
	}
}

PipelineState::PipelineState(const Description& desc)
{
	assert(desc.RenderTargetFormat.size() <= D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);

	// ラスタライザ
	D3D12_RASTERIZER_DESC rasterDesc	= CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterDesc.CullMode					= desc.CullMode;
	rasterDesc.DepthClipEnable			= desc.DepthClip;

	// ブレンド
	// IndependentBlendEnableがfalseなのでRenderTarget[0]の設定が全レンダーターゲットに適用される
	D3D12_BLEND_DESC blendDesc			= CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	blendDesc.AlphaToCoverageEnable		= false;
	blendDesc.IndependentBlendEnable	= false;
	if (desc.Blend == BlendMode::Alpha)
	{
		D3D12_RENDER_TARGET_BLEND_DESC& rt = blendDesc.RenderTarget[0];
		rt.BlendEnable				= TRUE;
		rt.SrcBlend					= D3D12_BLEND_SRC_ALPHA;
		rt.DestBlend				= D3D12_BLEND_INV_SRC_ALPHA;
		rt.BlendOp					= D3D12_BLEND_OP_ADD;
		rt.SrcBlendAlpha			= D3D12_BLEND_ONE;
		rt.DestBlendAlpha			= D3D12_BLEND_ZERO;
		rt.BlendOpAlpha				= D3D12_BLEND_OP_ADD;
		rt.RenderTargetWriteMask	= D3D12_COLOR_WRITE_ENABLE_ALL;
		rt.LogicOpEnable			= false;
	}

	// 深度
	D3D12_DEPTH_STENCIL_DESC dsDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	dsDesc.DepthEnable		= desc.DepthTest;
	dsDesc.DepthWriteMask	= desc.DepthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc		= D3D12_COMPARISON_FUNC_LESS_EQUAL;

	if (HasFile(desc.ASFile) || HasFile(desc.MSFile))
	{
		CreateMeshShaderPipelineState(desc, rasterDesc, blendDesc, dsDesc);
	}
	else
	{
		CreateDefaultPipelineState(desc, rasterDesc, blendDesc, dsDesc);
	}
}

PipelineState::~PipelineState()
{
}

void PipelineState::CreateDefaultPipelineState(const Description& desc,
	const D3D12_RASTERIZER_DESC& rasterizer, const D3D12_BLEND_DESC& blend, const D3D12_DEPTH_STENCIL_DESC& stencil)
{
	// シェーダー
	HRESULT hr;
	ComPtr<ID3DBlob> pVS, pPS;
	hr = D3DReadFileToBlob(desc.VSFile, pVS.GetAddressOf());
	if (!CheckResult(hr, L"シェーダーの読み込みに失敗しました", desc.VSFile)) { return; }
	hr = D3DReadFileToBlob(desc.PSFile, pPS.GetAddressOf());
	if (!CheckResult(hr, L"シェーダーの読み込みに失敗しました", desc.PSFile)) { return; }

	// インプットレイアウト
	std::vector<D3D12_INPUT_ELEMENT_DESC> element(desc.InputLayoutNum);
	for (UINT i = 0; i < desc.InputLayoutNum; ++i)
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
	SetCommonDesc(pipelineDesc, desc, rasterizer, blend, stencil);
	// シェーダ
	pipelineDesc.VS = { pVS->GetBufferPointer(), pVS->GetBufferSize() };
	pipelineDesc.PS = { pPS->GetBufferPointer(), pPS->GetBufferSize() };
	// 頂点レイアウト
	pipelineDesc.InputLayout = { element.data(), desc.InputLayoutNum };
	// プリミティブ
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// パイプラインの生成
	hr = GetDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(PipelineData.GetAddressOf()));
	CheckResult(hr, L"パイプラインステートの作成に失敗しました", desc.VSFile);
}


void PipelineState::CreateMeshShaderPipelineState(const Description& desc,
	const D3D12_RASTERIZER_DESC& rasterizer, const D3D12_BLEND_DESC& blend, const D3D12_DEPTH_STENCIL_DESC& stencil)
{
	// シェーダー（ASは任意）
	HRESULT hr;
	ComPtr<ID3DBlob> pAS, pMS, pPS;
	if (HasFile(desc.ASFile))
	{
		hr = D3DReadFileToBlob(desc.ASFile, pAS.GetAddressOf());
		if (!CheckResult(hr, L"シェーダーの読み込みに失敗しました", desc.ASFile)) { return; }
	}
	hr = D3DReadFileToBlob(desc.MSFile, pMS.GetAddressOf());
	if (!CheckResult(hr, L"シェーダーの読み込みに失敗しました", desc.MSFile)) { return; }
	hr = D3DReadFileToBlob(desc.PSFile, pPS.GetAddressOf());
	if (!CheckResult(hr, L"シェーダーの読み込みに失敗しました", desc.PSFile)) { return; }

	// パイプラインステートの設定
	D3DX12_MESH_SHADER_PIPELINE_STATE_DESC pipelineDesc = {};
	SetCommonDesc(pipelineDesc, desc, rasterizer, blend, stencil);
	// シェーダ
	if (pAS)
	{
		pipelineDesc.AS = { pAS->GetBufferPointer(), pAS->GetBufferSize() };
	}
	pipelineDesc.MS = { pMS->GetBufferPointer(), pMS->GetBufferSize() };
	pipelineDesc.PS = { pPS->GetBufferPointer(), pPS->GetBufferSize() };

	CD3DX12_PIPELINE_MESH_STATE_STREAM psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(pipelineDesc);

	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
	streamDesc.SizeInBytes = sizeof(psoStream);
	streamDesc.pPipelineStateSubobjectStream = &psoStream;

	// パイプラインの生成
	hr = GetDevice()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(PipelineData.GetAddressOf()));
	CheckResult(hr, L"パイプラインステートの作成に失敗しました", desc.MSFile);
}
