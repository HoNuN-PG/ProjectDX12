
#include "GlobalResourceKey.h"

#include "SceneSandBoxDX12.h"
#include "ConstantWVP.h"

// マテリアル
#include "Material/M_SimpleLit.h"

HRESULT SceneSandBoxDX12::Init()
{
    SceneBase::Initialize();

	// ディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 128;
		Heap = std::make_unique<DescriptorHeap>(desc);
	}
	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = Heap.get();
		desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
		WVPs.push_back(std::make_unique<ConstantBuffer>(desc));
	}
	// 定数バッファ
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = Heap.get();
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		// カメラ
		Params.push_back(std::make_unique<ConstantBuffer>(desc));
		// ライト
		Params.push_back(std::make_unique<ConstantBuffer>(desc));
	}
	// ルートシグネチャ
	{
		RootSignature::ParameterTables param[] = {
			{{D3D12_DESCRIPTOR_RANGE_TYPE_CBV}, {0}, {1}, 1, D3D12_SHADER_VISIBILITY_VERTEX },
			{{D3D12_DESCRIPTOR_RANGE_TYPE_CBV}, {0}, {2}, 1, D3D12_SHADER_VISIBILITY_PIXEL  },
		};
		RootSignature::DescriptionTables desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		RootSignatureData = std::make_unique<RootSignature>(desc);
	}
	// パイプライン
	{
		Pipeline::InputLayout layout[] = {
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"NORMAL",   0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
			{"COLOR",    0,DXGI_FORMAT_R32G32B32A32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.VSFile = L"assets/shader/VS_Object.cso";
		desc.PSFile = L"assets/shader/PS_SimpleLit.cso";
		desc.pRootSignature = RootSignatureData->Get();
		desc.RenderTargetNum = 1;
		PipelineData = std::make_unique<Pipeline>(desc);
	}
	// ディスクリプタヒープ（深度バッファ)
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.num = 1;
		DSVHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// 深度バッファ
	{
		DepthStencil::Description desc = {};
		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		desc.pDSVHeap = DSVHeap.get();
		DSV = std::make_unique<DepthStencil>(desc);
	}

	// マテリアル作成
	Materials.push_back(std::make_unique<M_SimpleLit>());
	Materials.back()->Initialize(Heap.get());

	// スフィア作成
	SphereMesh = std::make_unique<Sphere>();
	SphereMesh->Create();

	// モデル作成
	ModelMesh = std::make_unique<Model>();
	ModelMesh->Create(Materials[0].get(), "assets/model/spot/spot.fbx");

    return E_NOTIMPL;
}

void SceneSandBoxDX12::Uninit()
{
}

void SceneSandBoxDX12::Update()
{
	Camera->Update();
	Light->Update();
}

void SceneSandBoxDX12::Draw()
{
	Camera->Draw();
	Light->Draw();

	SceneBase::WriteGlobalResource();

	ID3D12GraphicsCommandList* pCmdList = GetCommandList();
	// 表示領域の設定
	D3D12_VIEWPORT vp = { 0, 0, 1280.0f, 720.0f, 0.0f, 1.0f };
	D3D12_RECT scissor = { 0, 0, 1280.0f, 720.0f };
	pCmdList->RSSetViewports(1, &vp);
	pCmdList->RSSetScissorRects(1, &scissor);

	// バックバッファに描画
	auto hRTV = GetRTV();
	SetRenderTarget(1, &hRTV, DSV->GetHandleDSV().hCPU);
	DSV->Clear();

	// パイプラインのバインド
	PipelineData->Bind();
	// WVP
	WVPs[0]->Write(ConstantWVP::Calc3DMatrix(
		{ 0,0,0 },
		{ 0,0,0 },
		{ 1,1,1 }));
	// グローバルリソースからリソースをコピー
	GetDevice()->CopyDescriptorsSimple(
		(UINT)2,
		Params[0]->GetHandle().hCPU, GetGlobalResource(GlobalResourceKey::Camera)->GetHandle().hCPU,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	ID3D12DescriptorHeap* heap[] = {
		Heap.get()->Get()
	};
	D3D12_GPU_DESCRIPTOR_HANDLE desc[] = {
		WVPs[0]->GetHandle().hGPU,
		Params[0]->GetHandle().hGPU,
	};
	DescriptorHeap::Bind(heap,_countof(heap));
	RootSignatureData->Bind(desc, _countof(desc));
	// 描画
	SphereMesh->Draw();

	ModelMesh->GetMaterial()->WriteWVP(ConstantWVP::Calc3DMatrix(
		{ 0,1,0 },
		{ 0,0,0 },
		{ 1,1,1 }));
	ModelMesh->GetMaterial()->WriteParams((UINT)2, 0, 
		GetGlobalResource(GlobalResourceKey::Camera)->GetHandle().hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	ModelMesh->Draw();
}
