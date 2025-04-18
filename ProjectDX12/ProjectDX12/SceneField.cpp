#include "SceneField.h"
#include <functional>
#include <stack>
#include <string>
#include <map>
#include <vector>
#include <DirectXMath.h>

const int MaxObjects = 3;
const int MaxConstBufNum = 3;

// #define CAMERA

HRESULT SceneField::Init()
{
	m_pCamera = new cCameraDebug();

	// ～～ 頂点データ作成 ～～
	struct Vertex
	{
		float pos[3];
		float normal[3];
		float uv[2];
	};
	const float maxSize = 20.0f;
	const int GridNum = 500;
	const float planeSpace = maxSize / (GridNum - 1);
	// 頂点生成
	std::vector<Vertex> planeVtx;
	for (int j = 0; j < GridNum; ++j)
	{
		for (int i = 0; i < GridNum; ++i)
		{
			planeVtx.push_back(
				{
					{i * planeSpace - maxSize * 0.5f, 0.0f, j * planeSpace - maxSize * 0.5f},
					{0.0f, 1.0f, 0.0f},
					{ i / (GridNum - 1.0f), j / (GridNum - 1.0f) }
				}
			);
		}
	}
	// インデックス生成
	std::vector<DWORD> planeIdx;
	for (int j = 0; j < GridNum - 1; ++j)
	{
		for (int i = 0; i < GridNum - 1; ++i)
		{
			planeIdx.push_back(GridNum * j + i);
			planeIdx.push_back(GridNum * j + i + 1);
			planeIdx.push_back(GridNum * (j + 1) + i);
			planeIdx.push_back(GridNum * (j + 1) + i);
			planeIdx.push_back(GridNum * j + i + 1);
			planeIdx.push_back(GridNum * (j + 1) + i + 1);
		}
	}
	{	// 頂点バッファの生成
		MeshBuffer::Description desc = {};
		desc.pVtx = planeVtx.data();
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = planeVtx.size();
		desc.pIdx = planeIdx.data();
		desc.idxSize = DXGI_FORMAT_R32_UINT;
		desc.idxCount = planeIdx.size();
		desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_pPlane = new MeshBuffer(desc);
	}

	// 事前の定義
	const int  SPHERE_HORIZONTAL = 17;
	const int  SPHERE_VERTICAL = 9;
	const float SPHERE_SIZE = 1000.0f;
	//  頂点計算
	std::vector<Vertex> sphereVtx;
	for (int j = 0; j < SPHERE_VERTICAL; ++j) {
		float radY = DirectX::XMConvertToRadians(180.0f * j / (SPHERE_VERTICAL - 1));
		float sinY = sinf(radY);
		float cosY = cosf(radY);
		for (int i = 0; i < SPHERE_HORIZONTAL; ++i) {
			float rad = DirectX::XMConvertToRadians(360.0f * i / (SPHERE_HORIZONTAL - 1));
			sphereVtx.push_back({
			 { // 座標
			  sinY * sinf(rad) * SPHERE_SIZE,
			  cosY * SPHERE_SIZE,
			  sinY * cosf(rad) * SPHERE_SIZE
			 },
			 {sinY * sinf(rad), cosY, sinY * cosf(rad) }, // 法線
			 {i / (SPHERE_HORIZONTAL - 1.0f), j / (SPHERE_VERTICAL - 1.0f)} // UV 
				});
		}
	}
	// インデックス
	std::vector<unsigned long> sphereIdx;
	for (int j = 0; j < SPHERE_VERTICAL - 1; ++j) {
		for (int i = 0; i < SPHERE_HORIZONTAL - 1; ++i) {
			sphereIdx.push_back((j + 1) * SPHERE_HORIZONTAL + i);
			sphereIdx.push_back(j * SPHERE_HORIZONTAL + i);
			sphereIdx.push_back(j * SPHERE_HORIZONTAL + i + 1);
			sphereIdx.push_back(j * SPHERE_HORIZONTAL + i + 1);
			sphereIdx.push_back((j + 1) * SPHERE_HORIZONTAL + i + 1);
			sphereIdx.push_back((j + 1) * SPHERE_HORIZONTAL + i);
		}
	}
	{ //  頂点バッファ
		MeshBuffer::Description desc = {};
		desc.pVtx = sphereVtx.data();
		desc.vtxCount = sphereVtx.size();
		desc.vtxSize = sizeof(Vertex);
		desc.pIdx = sphereIdx.data();
		desc.idxCount = sphereIdx.size();
		desc.idxSize = DXGI_FORMAT_R32_UINT;
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_pSphere = new MeshBuffer(desc);
	}

	{	// オブジェクト用ディスクリプターヒープ作成
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = MaxConstBufNum;
		m_pShaderHeap = new DescriptorHeap(desc);
	}
	{	// 変換行列用の定数バッファ作成
		ConstantBuffer::Description desc = {};
		m_pWVPs.resize(2);
		desc.pHeap = m_pShaderHeap;
		desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
		m_pWVPs[0] = new ConstantBuffer(desc);
		m_pWVPs[1] = new ConstantBuffer(desc);
	}
	{	// 時間用の定数バッファ作成
		ConstantBuffer::Description desc = {};
		m_pParams.resize(1);
		desc.pHeap = m_pShaderHeap;
		desc.size = sizeof(DirectX::XMFLOAT4);
		m_pParams[0] = new ConstantBuffer(desc);
	}

	{	// ルートシグネチャ生成
		RootSignature::Parameter param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		};
		RootSignature::Description desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		m_pRootSignature = new RootSignature(desc);
	}
	{	// 水面用ルートシグネチャ生成
		RootSignature::Parameter param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::Description desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		m_pWaterRS = new RootSignature(desc);
	}
	{ // スカイルートシグネチャ
		RootSignature::Parameter param[] = {
		 {D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
		 {D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::Description desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		m_pSkyRS = new RootSignature(desc);
	}

	{	// パイプライン生成
		Pipeline::InputLayout layout[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.RenderTargetNum = 1;
		m_pPipelines.resize(MaxObjects);
		// フィールド
		desc.pRootSignature = m_pRootSignature->Get();
		desc.VSFile = L"assets/shader/VS_Field.cso";
		desc.PSFile = L"assets/shader/PS_Field.cso";
		m_pPipelines[0] = new Pipeline(desc);
		// 水面
		desc.pRootSignature = m_pWaterRS->Get();
		desc.VSFile = L"assets/shader/VS_Water.cso";
		desc.PSFile = L"assets/shader/PS_Water.cso";
		m_pPipelines[1] = new Pipeline(desc);
		// スカイ
		desc.pRootSignature = m_pSkyRS->Get();
		desc.VSFile = L"assets/shader/VS_AtmosphericScattering.cso";
		desc.PSFile = L"assets/shader/PS_AtmosphericScattering.cso";
		m_pPipelines[2] = new Pipeline(desc);
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
void SceneField::Uninit()
{
	delete m_pCamera;

	delete m_pPlane;
	delete m_pRootSignature;
	delete m_pDSVHeap;
	delete m_pDSV;
}
void SceneField::Update()
{
	m_pCamera->Update();
}
void SceneField::Draw()
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

	static float rad = 0.0f;
	rad += 0.005f;
	DirectX::XMFLOAT4X4 fMat[3];
	DirectX::XMMATRIX mat[3];
#ifdef CAMERA
	DirectX::XMFLOAT3 camPos = { cCameraDebug::m_MainPos.x, cCameraDebug::m_MainPos.y, cCameraDebug::m_MainPos.z };
	DirectX::XMFLOAT3 camTarget = { cCameraDebug::m_MainTarget.x, cCameraDebug::m_MainTarget.y, cCameraDebug::m_MainTarget.z };
#else
	DirectX::XMFLOAT3 camPos = { 3, 5.0f, -7 };
	DirectX::XMFLOAT3 camTarget = { -7, 6, 1 };
#endif
	mat[1] = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(camPos.x,camPos.y,camPos.z, 0.0),
		DirectX::XMVectorSet(camTarget.x,camTarget.y,camTarget.z, 0.0),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0)
	);
	mat[2] = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(60.0f), 16.f / 9.f, 0.1f, 500.0f
	);

	for (int i = 0; i < 3; ++i) {
		DirectX::XMStoreFloat4x4(&fMat[i], DirectX::XMMatrixTranspose(mat[i]));
	}

	// スカイスフィアをカメラの位置に
	mat[0] = DirectX::XMMatrixTranslation(camPos.x, camPos.y, camPos.z);
	DirectX::XMStoreFloat4x4(&fMat[0], DirectX::XMMatrixTranspose(mat[0]));
	m_pWVPs[0]->Write(&fMat);

	// 深度バッファOFF
	SetRenderTarget(_countof(hRTV), hRTV);

	// 時間パラメーターの設定
	static float time = 0;
	time += 0.01f;

	// スカイ描画
	DirectX::XMFLOAT4 param = { time,camPos.x,camPos.y,camPos.z };
	m_pParams[0]->Write(&param);
	m_pPipelines[2]->Bind();
	D3D12_GPU_DESCRIPTOR_HANDLE hSky[] = {
		m_pWVPs[0]->GetHandle().hGPU,
		m_pParams[0]->GetHandle().hGPU,
	};
	m_pSkyRS->Bind(hSky, 2);
	m_pSphere->Draw();

	// 深度バッファON
	SetRenderTarget(_countof(hRTV), hRTV, hDSV);

	// 地形
	mat[0] = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	DirectX::XMStoreFloat4x4(&fMat[0], DirectX::XMMatrixTranspose(mat[0]));
	m_pWVPs[1]->Write(&fMat);
	m_pPipelines[0]->Bind();
	D3D12_GPU_DESCRIPTOR_HANDLE handle[] = {
		m_pWVPs[1]->GetHandle().hGPU
	};
	m_pRootSignature->Bind(handle, 1);
	m_pPlane->Draw();

	// 水面
	mat[0] = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	DirectX::XMStoreFloat4x4(&fMat[0], DirectX::XMMatrixTranspose(mat[0]));
	m_pPipelines[1]->Bind();
	D3D12_GPU_DESCRIPTOR_HANDLE hWater[] = {
		m_pWVPs[1]->GetHandle().hGPU,
		m_pParams[0]->GetHandle().hGPU
	};
	m_pWaterRS->Bind(hWater, 2);
	m_pPlane->Draw();
}