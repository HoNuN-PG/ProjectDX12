#include "SceneProcedural.h"
#include <functional>
#include <stack>
#include <string>
#include <map>
#include <vector>
#include <DirectXMath.h>

const int MaxObjects = 7;
const int MaxConstants = MaxObjects * 2;

HRESULT SceneProcedural::Init()
{
	// ～～ 頂点データ作成 ～～
	struct Vertex {
		DirectX::XMFLOAT3  pos;
		DirectX::XMFLOAT2 uv;
	} vtx[] = {
		{ { -0.5f, 0.5f, 0.0f }, {0.0f, 0.0f} },
		{ {  0.5f, 0.5f, 0.0f }, {1.0f, 0.0f} },
		{ { -0.5f,-0.5f, 0.0f }, {0.0f, 1.0f} },
		{ {  0.5f,-0.5f, 0.0f }, {1.0f, 1.0f} },
	};

	// ～～～　構築後、頂点データに基づいて頂点バッファを生成　～～～
	{	// 頂点バッファの生成
		MeshBuffer::Description desc = {};
		desc.pVtx = vtx;
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = _countof(vtx);
		desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		m_pSprite = new MeshBuffer(desc);
	}

	{	// オブジェクト用ディスクリプターヒープ作成
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = MaxConstants;
		m_pShaderHeap = new DescriptorHeap(desc);
	}
	{	// オブジェクト用の定数バッファ作成
		ConstantBuffer::Description desc = {};
		desc.pHeap = m_pShaderHeap;
		desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
		m_pWVPs.resize(MaxObjects);
		for (int i = 0; i < MaxObjects; ++i) {
			m_pWVPs[i] = new ConstantBuffer(desc);
		}
	}
	{	// オブジェクト用のパラメータ定数バッファ作成
		ConstantBuffer::Description desc = {};
		desc.pHeap = m_pShaderHeap;
		desc.size = sizeof(DirectX::XMFLOAT4);
		m_pParams.resize(MaxObjects);
		for (int i = 0; i < MaxObjects; ++i) {
			m_pParams[i] = new ConstantBuffer(desc);
		}
	}
	{	// ルートシグネチャ生成
		RootSignature::ParameterTable param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::DescriptionTable desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		m_pRootSignature = new RootSignature(desc);
	}
	{	// パイプライン生成
		Pipeline::InputLayout layout[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.pRootSignature = m_pRootSignature->Get();
		desc.RenderTargetNum = 1;
		m_pPipelines.resize(MaxObjects);
		// ノイズ
		desc.VSFile = L"assets/shader/VS_Sprite.cso";
		desc.PSFile = L"assets/shader/PS_Noise.cso";
		m_pPipelines[0] = new Pipeline(desc);
		desc.PSFile = L"assets/shader/PS_Block.cso";
		m_pPipelines[1] = new Pipeline(desc);
		desc.PSFile = L"assets/shader/PS_Voronoi.cso";
		m_pPipelines[2] = new Pipeline(desc);
		desc.PSFile = L"assets/shader/PS_Value.cso";
		m_pPipelines[3] = new Pipeline(desc);
		desc.PSFile = L"assets/shader/PS_Perlin.cso";
		m_pPipelines[4] = new Pipeline(desc);
		desc.PSFile = L"assets/shader/PS_FBM.cso";
		m_pPipelines[5] = new Pipeline(desc);
		desc.PSFile = L"assets/shader/PS_Crack.cso";
		m_pPipelines[6] = new Pipeline(desc);
	}
	{	// DSV用のディスクリプター作成
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.num = 1;
		m_pDSVHeap = new DescriptorHeap(desc);
	}
	{	// 深度バッファ作成
		DepthStencil::Description desc = {};
		desc.width = 1280;
		desc.height = 720;
		desc.pDSVHeap = m_pDSVHeap;
		m_pDSV = new DepthStencil(desc);
	}

	return S_OK;
}
void SceneProcedural::Uninit()
{
	delete m_pSprite;
}
void SceneProcedural::Update()
{
}
void SceneProcedural::Draw()
{
	// 描画
	ID3D12GraphicsCommandList* pCmdList = GetCommandList();
	D3D12_CPU_DESCRIPTOR_HANDLE hRTV[] = { GetRTV() };
	auto hDSV = m_pDSV->GetHandleDSV().hCPU;
	SetRenderTarget(_countof(hRTV), hRTV, hDSV);
	m_pDSV->Clear();

	// 表示領域の設定
	float width = 1280.0f;
	float height = 720.0f;
	D3D12_VIEWPORT vp = { 0, 0, width, height, 0.0f, 1.0f };
	D3D12_RECT scissor = { 0, 0, (LONG)width, (LONG)height };
	pCmdList->RSSetViewports(1, &vp);
	pCmdList->RSSetScissorRects(1, &scissor);

	m_pShaderHeap->Bind();

	DirectX::XMFLOAT4X4 fMat[3];
	DirectX::XMMATRIX mat[3];
	mat[1] = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(0.0f, 0.0f, -7.5f, 0.0),
		DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0)
	);
	mat[2] = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(60.0f), 16.f / 9.f, 0.1f, 100.0f
	);

	for (int i = 0; i < 3; ++i) {
		DirectX::XMStoreFloat4x4(&fMat[i], DirectX::XMMatrixTranspose(mat[i]));
	}
	for (int i = 0; i < MaxObjects; ++i) {
		// 配置場所計算
		mat[0] = DirectX::XMMatrixTranslation((i - MaxObjects / 2) * 2.0f, 0.0f, 0.0f);
		DirectX::XMStoreFloat4x4(&fMat[0], DirectX::XMMatrixTranspose(mat[0]));
		m_pWVPs[i]->Write(&fMat);

		static float t = 0;
		t += 0.1f;
		DirectX::XMFLOAT4 param = { t,0,0,0 };
		m_pParams[i]->Write(&param);

		// 描画
		m_pPipelines[i]->Bind();

		D3D12_GPU_DESCRIPTOR_HANDLE handle[] = {
			m_pWVPs[i]->GetHandle().hGPU,
			m_pParams[i]->GetHandle().hGPU
		};
		m_pRootSignature->Bind(handle, _countof(handle));
		m_pSprite->Draw();
	}
}
