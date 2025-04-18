
#include "Game.h"

#include <DirectXTex.h>
#include <TextureLoad.h>

#include "DirectX.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma comment(lib, "assimp-vc141-mtd.lib")

#include "ConstantWVP.h"
#include "ConstantLighting.h"

#include "imgui/imgui.h"

std::unique_ptr<cCameraDebug>				cGame::m_pCamera;
std::unique_ptr<cLightBase>					cGame::m_pLight;

std::shared_ptr<MeshBuffer>				cGame::m_pScreen;
std::vector<std::shared_ptr<MeshBuffer>>	cGame::m_pModel;
std::shared_ptr<ConstantBuffer>			cGame::m_pObjectCB_WVP[MAX_WVP];
std::shared_ptr<ConstantBuffer>			cGame::m_pObjectCB_WVP_Gbuffer[3];
std::shared_ptr<ConstantBuffer>			cGame::m_pObjectCB_IBL;
std::shared_ptr<RootSignature>				cGame::m_pRootSignature;
std::shared_ptr<RootSignature>				cGame::m_pRootSignatureDeffered;
std::shared_ptr<Pipeline>					cGame::m_pPipelineDefferedWrite;
std::shared_ptr<Pipeline>					cGame::m_pPipelineDeffered;
std::shared_ptr<Pipeline>					cGame::m_pPipeline;
std::shared_ptr<Pipeline>					cGame::m_pSkyPipeline;
std::shared_ptr<Pipeline>					cGame::m_pBlurPipeline;
std::shared_ptr<cTexture>					cGame::m_pTexture;
std::shared_ptr<cRenderTarget>				cGame::m_pRTV[4];
std::shared_ptr<DescriptorHeap>			cGame::m_pHeap;
std::shared_ptr<DescriptorHeap>			cGame::m_pRTVHeap;
std::shared_ptr<DescriptorHeap>			cGame::m_pDSVHeap;
std::shared_ptr<DepthStencil>				cGame::m_pDSV;
std::shared_ptr<MeshBuffer>				cGame::m_pSkySphere;
std::shared_ptr<cTexture>					cGame::m_pHDRI;

HRESULT cGame::InitGame()
{
	// カメラ
	m_pCamera = std::make_unique<cCameraDebug>();
	m_pLight = std::make_unique<cLightBase>();

	// スクリーン頂点
	Vertex screenVtx[] =
	{
		{{-0.5f, 0.5f,0} ,{0,0},{1,1,1,1}} ,
		{{ 0.5f, 0.5f,0} ,{1,0},{1,1,1,1}} ,
		{{-0.5f,-0.5f,0} ,{0,1},{1,1,1,1}} ,
		{{ 0.5f,-0.5f,0} ,{1,1},{1,1,1,1}} ,
	};

	MeshBuffer::Description desc = {};
	// スクリーン
	desc.pVtx = screenVtx;
	desc.vtxSize = sizeof(Vertex);
	desc.vtxCount = _countof(screenVtx);
	desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	m_pScreen = std::make_shared<MeshBuffer>(desc);

	// 天球モデルの頂点情報
	struct Vertex {
		float pos[3];
		float uv[2];
	};
	// 天球作成時の定数
	const int SkyDetail = 3;
	const int SkyVtxXNum = SkyDetail * 4 + 1;			// 横方向の頂点数
	const int SkyVtxYNum = SkyDetail * 2 + 1;			// 縦方向の頂点数
	const int SkyVtxNum = SkyVtxXNum * SkyVtxYNum;		// 全体の頂点数
	const int SkyIdxXNum = SkyVtxXNum - 1;				// 横方向の面数
	const int SkyIdxYNum = SkyVtxYNum - 1;				// 縦方向の面数
	const int SkyIdxNum = SkyIdxXNum * SkyIdxYNum * 6;	// 全体のインデックス数(面 = 3+3)
	// 頂点情報作成
	Vertex sphereVtx[SkyVtxNum];
	for (int j = 0; j < SkyVtxYNum; ++j) {
		float v = (float)j / SkyIdxYNum;
		float radY = DirectX::XM_PI * v;
		float sY = sinf(radY);
		float cY = cosf(radY);
		int idx = SkyVtxXNum * j;
		for (int i = 0; i < SkyVtxXNum; ++i) {
			float u = (float)i / SkyIdxXNum;
			float radXZ = DirectX::XM_2PI * u;
			float x = sinf(radXZ) * sY;
			float z = cosf(radXZ) * sY;
			sphereVtx[idx + i] = { {x, cY, z}, {u, v} };
		}
	}
	// インデックス作成
	unsigned short sphereIdx[SkyIdxNum];
	for (int j = 0; j < SkyIdxYNum; ++j) {
		int vtxIdxY = j * SkyVtxXNum;
		int idxY = j * SkyIdxXNum;
		for (int i = 0; i < SkyIdxXNum; ++i) {
			int vtxIdx = vtxIdxY + i;
			int idx = (idxY + i) * 6;
			sphereIdx[idx + 0] = vtxIdx + 1;
			sphereIdx[idx + 1] = vtxIdx;
			sphereIdx[idx + 2] = vtxIdx + SkyVtxXNum;
			sphereIdx[idx + 3] = sphereIdx[idx + 2];
			sphereIdx[idx + 4] = sphereIdx[idx + 2] + 1;
			sphereIdx[idx + 5] = sphereIdx[idx + 0];
		}
	}
	{
		// メッシュ作成
		MeshBuffer::Description desc = {};
		desc.pVtx = sphereVtx;
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = SkyVtxNum;
		desc.pIdx = sphereIdx;
		desc.idxSize = DXGI_FORMAT_R16_UINT;
		desc.idxCount = SkyIdxNum;
		desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_pSkySphere = std::make_shared<MeshBuffer>(desc);
	}

	// モデル読込
	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_JoinIdenticalVertices;
	flag |= aiProcess_PreTransformVertices;
	flag |= aiProcess_FlipUVs; // UV修正
	const aiScene* pScene = importer.ReadFile("assets/model/spot/spot.fbx", flag);
	if (!pScene) {
		MessageBox(nullptr, importer.GetErrorString(), "Assimp Error", MB_OK);
		return E_FAIL;
	}

	// 読み込んだデータから頂点バッファ生成
	desc.vtxSize	= sizeof(ModelVertex);
	desc.idxSize	= DXGI_FORMAT_R16_UINT;
	desc.topology	= D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	for (int i = 0; i < pScene->mNumMeshes; ++i) {
		// 頂点データの初期値作成
		std::vector<ModelVertex> modelVtx;
		for (int j = 0; j < pScene->mMeshes[i]->mNumVertices; ++j) {
			// データ吸出し
			aiVector3D vtxPos = pScene->mMeshes[i]->mVertices[j];
			aiVector3D vtxNormal = pScene->mMeshes[i]->HasNormals() ?
				pScene->mMeshes[i]->mNormals[j] : aiVector3D(0.0f, 0.0f, 0.0f);
			aiVector3D vtxUv = pScene->mMeshes[i]->HasTextureCoords(0) ?
				pScene->mMeshes[i]->mTextureCoords[0][j] : aiVector3D(0.0f, 0.0f, 0.0f);
			// 読み出したデータを設定
			modelVtx.push_back({
			DirectX::XMFLOAT3(vtxPos.x, vtxPos.y, vtxPos.z),
			DirectX::XMFLOAT3(vtxNormal.x, vtxNormal.y, vtxNormal.z),
			DirectX::XMFLOAT2(vtxUv.x, vtxUv.y),
				});
		}
		// インデックスデータの初期値作成
		std::vector<WORD> modelIdx;
		for (int j = 0; j < pScene->mMeshes[i]->mNumFaces; ++j) {
			aiFace& face = pScene->mMeshes[i]->mFaces[j];
			modelIdx.push_back(face.mIndices[0]);
			modelIdx.push_back(face.mIndices[1]);
			modelIdx.push_back(face.mIndices[2]);
		}
		// データ生成
		desc.pVtx		= modelVtx.data();
		desc.vtxCount	= modelVtx.size();
		desc.pIdx		= modelIdx.data();
		desc.idxCount	= modelIdx.size();
		m_pModel.push_back(std::make_shared<MeshBuffer>(desc));
	}
	{
		// ディスクリプターヒープ
		DescriptorHeap::Description desc	= {};
		desc.heapType						= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num							= 15; // 必要なディスクリプタ数に応じて変更
		m_pHeap								= std::make_shared<DescriptorHeap>(desc);
	}
	{ 
		// 定数バッファ
		ConstantBuffer::Description desc	= {};
		desc.pHeap							= m_pHeap.get();
		desc.size							= sizeof(DirectX::XMFLOAT4X4) * 3;
		for (int i = 0; i < MAX_WVP; i++)
		{
			m_pObjectCB_WVP[i] = std::make_shared<ConstantBuffer>(desc);
		}
	}
	{
		// 定数バッファ
		ConstantBuffer::Description desc = {};
		desc.pHeap = m_pHeap.get();
		desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
		for (int i = 0; i < 3; i++)
		{
			m_pObjectCB_WVP_Gbuffer[i] = std::make_shared<ConstantBuffer>(desc);
		}
	}
	{
		// 定数バッファ
		// IBL
		ConstantBuffer::Description desc = {};
		desc.pHeap = m_pHeap.get();
		desc.size = sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4) * 3;
		m_pObjectCB_IBL = std::make_shared<ConstantBuffer>(desc);
	}
	{ 
		// ルートシグネチャ
		RootSignature::Parameter param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::Description desc	= {};
		desc.pParam							= param;
		desc.paramNum						= _countof(param);
		m_pRootSignature					= std::make_shared<RootSignature>(desc);
	}
	{
		// ディファードルートシグネチャ
		RootSignature::Parameter param[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX},
			{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL},
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1, D3D12_SHADER_VISIBILITY_PIXEL},
		};
		RootSignature::Description desc = {};
		desc.pParam = param;
		desc.paramNum = _countof(param);
		m_pRootSignatureDeffered = std::make_shared<RootSignature>(desc);
	}
	{
		// ディファードパイプライン
		Pipeline::InputLayout layout[] = {
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"NORMAL", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
		};
		Pipeline::Description desc	= {};
		desc.pInputLayout				= layout;
		desc.InputLayoutNum				= _countof(layout);
		desc.VSFile						= L"assets/shader/VS_DefferedWrite.cso";
		desc.PSFile						= L"assets/shader/PS_DefferedWrite.cso";
		desc.pRootSignature				= m_pRootSignature->Get();
		desc.RenderTargetNum			= 3;
		m_pPipelineDefferedWrite		= std::make_shared<Pipeline>(desc);
	}
	{
		// ディファードパイプライン
		Pipeline::InputLayout layout[] = {
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
			{"COLOR",    0,DXGI_FORMAT_R32G32B32A32_FLOAT},
		};
		Pipeline::Description desc	= {};
		desc.pInputLayout				= layout;
		desc.InputLayoutNum				= _countof(layout);
		desc.VSFile						= L"assets/shader/Vertex.cso";
		desc.PSFile						= L"assets/shader/PS_Deffered.cso";
		desc.pRootSignature				= m_pRootSignatureDeffered->Get();
		desc.RenderTargetNum			= 1;
		m_pPipelineDeffered				= std::make_shared<Pipeline>(desc);
	}
	{ 
		// パイプライン
		Pipeline::InputLayout layout[] = {
			{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT},
			{"TEXCOORD", 0,DXGI_FORMAT_R32G32_FLOAT},
			{"COLOR",    0,DXGI_FORMAT_R32G32B32A32_FLOAT},
		};
		Pipeline::Description desc	= {};
		desc.pInputLayout				= layout;
		desc.InputLayoutNum				= _countof(layout);
		desc.VSFile						= L"assets/shader/Vertex.cso";
		desc.PSFile						= L"assets/shader/Pixel.cso";
		desc.pRootSignature				= m_pRootSignature->Get();
		desc.RenderTargetNum			= 1;
		m_pPipeline						= std::make_shared<Pipeline>(desc);
	}
	{
		// スカイスフィアパイプライン
		Pipeline::InputLayout layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.VSFile = L"assets/shader/VS_Sphere.cso"; 
		desc.PSFile = L"assets/shader/ModelPixel.cso";
		desc.pRootSignature = m_pRootSignature->Get();
		desc.RenderTargetNum = 1;
		m_pSkyPipeline = std::make_shared<Pipeline>(desc);
	}
	{
		// ブラーパイプライン
		Pipeline::InputLayout layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT},
		};
		Pipeline::Description desc = {};
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.VSFile = L"assets/shader/VS_Sphere.cso";
		desc.PSFile = L"assets/shader/PS_Blur.cso";
		desc.pRootSignature = m_pRootSignature->Get();
		desc.RenderTargetNum = 1;
		m_pBlurPipeline = std::make_shared<Pipeline>(desc);
	}
	{ 
		// テクスチャ
		cTexture::sDescription desc = {};
		desc.fileName				= "assets/model/spot/spot_texture.png";
		desc.pHeap					= m_pHeap.get();
		m_pTexture					= std::make_shared<cTexture>(desc);
	}
	{
		// 環境マップ
		cTexture::sDescription desc = {};
		desc.fileName				= "assets/texture/environmentMap.hdr";
		desc.pHeap					= m_pHeap.get();
		m_pHDRI						= std::make_shared<cTexture>(desc);
	}
	{ 
		// ディスクリプターヒープ(レンダーターゲット)
		DescriptorHeap::Description desc	= {};
		desc.heapType						= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.num							= 4; // レンダーターゲットの数に応じて設定
		m_pRTVHeap							= std::make_shared<DescriptorHeap>(desc);
	}
	{ 
		// レンダーターゲット
		for (int i = 0; i < 3; i++)
		{
			cRenderTarget::sDescription desc = {};
			desc.width = 1280;
			desc.height = 720;
			desc.pRTVHeap = m_pRTVHeap.get();
			desc.pSRVHeap = m_pHeap.get();
			m_pRTV[i] = std::make_shared<cRenderTarget>(desc);
		}
		cRenderTarget::sDescription desc = {};
		desc.width = 1280 * 0.25f;
		desc.height = 720 * 0.25f;
		desc.pRTVHeap = m_pRTVHeap.get();
		desc.pSRVHeap = m_pHeap.get();
		m_pRTV[3] = std::make_shared<cRenderTarget>(desc);
	}
	{
		// ディスクリプターヒープ（深度バッファ)
		DescriptorHeap::Description desc	= {};
		desc.heapType						= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.num							= 1;
		m_pDSVHeap							= std::make_shared<DescriptorHeap>(desc);
	}
	{
		// 深度バッファ
		DepthStencil::Description desc	= {};
		desc.width							= WINDOW_WIDTH;
		desc.height							= WINDOW_HEIGHT;
		desc.pDSVHeap						= m_pDSVHeap.get();
		m_pDSV								= std::make_shared<DepthStencil>(desc);
	}
	return HRESULT();
}

void cGame::UninitGame()
{

}

void cGame::UpdateGame()
{
	m_pCamera->Update();
	m_pLight->Update();
}

void cGame::DrawGame()
{
	m_pCamera->Draw();
	m_pLight->Draw();

	ID3D12GraphicsCommandList* pCmdList = GetCommandList();
	// 表示領域の設定
	D3D12_VIEWPORT vp	= { 0, 0, 1280.0f, 720.0f, 0.0f, 1.0f };
	D3D12_RECT scissor	= { 0, 0, 1280.0f, 720.0f };
	pCmdList->RSSetViewports(1, &vp);
	pCmdList->RSSetScissorRects(1, &scissor);

	const float clearColor[4] = { 1.0f,0.0f,1.0f,0 };

	// ディファード用レンダーターゲットの切り替え
	for (int i = 0; i < 3; i++)
	{
		m_pRTV[i]->ResourceBarrier(
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[3] = {
		m_pRTV[0]->GetHandleRTV().hCPU,
		m_pRTV[1]->GetHandleRTV().hCPU,
		m_pRTV[2]->GetHandleRTV().hCPU,
	};
	SetRenderTarget(3,rtvs, m_pDSV->GetHandleDSV().hCPU);
	for (int i = 0; i < 3; i++)
	{
		m_pRTV[i]->Clear(clearColor);
	}
	m_pDSV->Clear();

	{
		// モデルを描画
		m_pPipelineDefferedWrite->Bind();

		// モデルデバッグ
		static float rot[3] = { 0,0,0 };
#ifdef _DEBUG
		ImGui::Begin("ModelDebug");
		{
			ImGui::SliderFloat3("Angle", rot, 0.0f, 360.0f);
		}
		ImGui::End();
#endif

		m_pObjectCB_WVP[MODEL]->Write(cConstantWVP::Calc3DMatrix(
			{ 0,0,0 },
			{ DirectX::XMConvertToRadians(rot[0]),DirectX::XMConvertToRadians(rot[1]),DirectX::XMConvertToRadians(rot[2]) },
			{ 1,1,1 }));
		D3D12_GPU_DESCRIPTOR_HANDLE handle[] = {
			m_pObjectCB_WVP[MODEL]->GetHandle().hGPU,
			m_pTexture->GetHandle().hGPU
		};
		m_pHeap->Bind();
		m_pRootSignature->Bind(handle, _countof(handle));
		auto it = m_pModel.begin();
		while (it != m_pModel.end())
		{
			(*it)->Draw();
			++it;
		}
	}

	// ブラー用レンダーターゲットの切り替え
	for (int i = 0; i < 3; i++)
	{
		m_pRTV[i]->ResourceBarrier(
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	m_pRTV[3]->ResourceBarrier(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	rtvs[0] = m_pRTV[3]->GetHandleRTV().hCPU;
	pCmdList->OMSetRenderTargets(1, rtvs, false, nullptr);
	m_pRTV[3]->Clear(clearColor);

	{
		// ビューポート変更
		vp.Width = 1280 * 0.25f;
		vp.Height = 720 * 0.25f;
		pCmdList->RSSetViewports(1, &vp);
		// 天球描画
		m_pBlurPipeline->Bind();
		// シェーダーに渡す定数バッファ&テクスチャを指定
		m_pObjectCB_WVP[BLUR]->Write(cConstantWVP::Calc3DMatrix(
			{ 0,0,0 },
			{ 0,0,0 },
			{ 1,1,1 }));
		D3D12_GPU_DESCRIPTOR_HANDLE hSky[] = {
			m_pObjectCB_WVP[BLUR]->GetHandle().hGPU,
			m_pHDRI->GetHandle().hGPU
		};
		m_pHeap->Bind();
		m_pRootSignature->Bind(hSky, _countof(hSky));
		// 描画
		m_pSkySphere->Draw();
		// ビューポート変更
		vp.Width = 1280;
		vp.Height = 720;
		pCmdList->RSSetViewports(1, &vp);
	}

	// 通常レンダーターゲットの切り替え
	m_pRTV[3]->ResourceBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	auto hRTV = GetRTV();
	pCmdList->OMSetRenderTargets(1, &hRTV, false, nullptr);

	{
		// スカイスフィア描画
		m_pSkyPipeline->Bind();
		// シェーダーに渡す定数バッファ&テクスチャを指定
		m_pObjectCB_WVP[SKY_SPHERE]->Write(cConstantWVP::Calc3DMatrix(
			{ 0,0,0 },
			{ 0,0,0 },
			{ 1,1,1 }));
		D3D12_GPU_DESCRIPTOR_HANDLE hSky[] = {
		m_pObjectCB_WVP[SKY_SPHERE]->GetHandle().hGPU,
		m_pHDRI->GetHandle().hGPU
		};
		m_pHeap->Bind();
		m_pRootSignature->Bind(hSky, _countof(hSky));
		// 描画
		m_pSkySphere->Draw();
	}

	// Gbuffer表示デバッグ
#ifdef _DEBUG
	static bool isGbuffer = false;
	ImGui::Begin("Gbuffer");
	{
		ImGui::Checkbox("ViewGbuffer", &isGbuffer);
	}
	ImGui::End();
#endif

	if (isGbuffer)
	{
		// レンダーターゲット（GBuffer）の内容を描画
		m_pPipeline->Bind();
		for (int i = 0; i < 3; i++)
		{
			m_pObjectCB_WVP_Gbuffer[i]->Write(cConstantWVP::Calc2DMatrix(
				{ 320.0f + 640.0f * (int)(i % 2), -180.0f - 360.0f * (int)(i / 2), 0 },
				{ 0,0,0 },
				{ WINDOW_WIDTH / 2.5f, WINDOW_HEIGHT / 2.5f, 0 }));
			D3D12_GPU_DESCRIPTOR_HANDLE hScreen[] = {
				m_pObjectCB_WVP_Gbuffer[i]->GetHandle().hGPU,
				i == 0 ? m_pRTV[3]->GetHandleSRV().hGPU : m_pRTV[i]->GetHandleSRV().hGPU,
			};
			m_pHeap->Bind();
			m_pRootSignature->Bind(hScreen, _countof(hScreen));
			m_pScreen->Draw();
		}
	}

	{
		// ディファードライティング
		m_pPipelineDeffered->Bind();
		if (isGbuffer)
		{
			m_pObjectCB_WVP[DEFFERED]->Write(cConstantWVP::Calc2DMatrix(
				{ 320.0f + 640.0f * (int)(3 % 2), -180.0f - 360.0f * (int)(3 / 2), 0 },
				{ 0,0,0 },
				{ WINDOW_WIDTH / 2.5f, WINDOW_HEIGHT / 2.5f, 0 }));
		}
		else
		{
			m_pObjectCB_WVP[DEFFERED]->Write(cConstantWVP::Calc2DMatrix(
				{ WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2, 0 },
				{ 0,0,0 },
				{ WINDOW_WIDTH	  ,  WINDOW_HEIGHT	  , 0 }));
		}

		// IBL情報
		DirectX::XMFLOAT4X4 param[2];
		// VP逆行列
		param[0] = cConstantWVP::CalcInversVP();
		// ライトの設定
		param[1]._11 = m_pLight->GetDir().x;
		param[1]._12 = m_pLight->GetDir().y;
		param[1]._13 = m_pLight->GetDir().z;
		param[1]._14 = m_pLight->GetPower();
		param[1]._21 = m_pLight->GetColor().x;
		param[1]._22 = m_pLight->GetColor().y;
		param[1]._23 = m_pLight->GetColor().z;
		param[1]._24 = m_pLight->GetAmbient();
		// カメラの設定
		param[1]._31 = cCameraDebug::m_MainPos.x;
		param[1]._32 = cCameraDebug::m_MainPos.y;
		param[1]._33 = cCameraDebug::m_MainPos.z;
		param[1]._34 = 0.0f;
		m_pObjectCB_IBL->Write(&param);
		D3D12_GPU_DESCRIPTOR_HANDLE hScreen[] = {
			m_pObjectCB_WVP[DEFFERED]->GetHandle().hGPU,
			m_pObjectCB_IBL->GetHandle().hGPU,
			m_pRTV[0]->GetHandleSRV().hGPU,
			m_pRTV[1]->GetHandleSRV().hGPU,
			m_pRTV[2]->GetHandleSRV().hGPU,
			m_pRTV[3]->GetHandleSRV().hGPU,	// ぼかしスカイスフィア
		};
		m_pHeap->Bind();
		m_pRootSignatureDeffered->Bind(hScreen, _countof(hScreen));
		m_pScreen->Draw();
	}
}
