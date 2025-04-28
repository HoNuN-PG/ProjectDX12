#include "SceneLSystem.h"
#include <functional>
#include <stack>
#include <string>
#include <map>
#include <vector>
#include <DirectXMath.h>

class LSystem
{
	// LSystemクラスの実装(プリント参照)
public:
	void Execute(int iteration, const char* initValue, void* arg = nullptr);
	void AddRule(char key, const char* str);						// 変換ルールの追加
	void AddBehavior(char key, std::function<void(void*)> func);	// 実行処理

private:
	std::map<char, std::string> m_rule;						// 変換ルール
	std::map<char, std::function<void(void*)>> m_behavior;	// 文字の動作
};

HRESULT SceneLSystem::Init()
{
	m_pCamera = new CameraDebug();

	// LSystemを利用した頂点データの作成

	// 頂点座標
	struct Vertex {
		float pos[3];
	};
	std::vector<Vertex> vtx;
	std::vector<int> idx;
	int num = 0;

	// スタック情報
	struct Param {
		DirectX::XMFLOAT3 pos; // 現在の位置
		DirectX::XMFLOAT3 vec; // 現在の進行方向
	};
	std::stack<Param> stack;
	stack.push({ DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(0,1,0) });

	// ポリゴン基本位置及び基本インデックス
	float thick = 0.1f;
	std::vector<Vertex> vtx_base = {
		// -Z
		{-thick,  1.0f,-thick},
		{ thick,  1.0f,-thick},
		{-thick,  0.0f,-thick},
		{ thick,  0.0f,-thick},
		// +X
		{ thick,  1.0f,-thick},
		{ thick,  1.0f, thick},
		{ thick,  0.0f,-thick},
		{ thick,  0.0f, thick},
		// +Z
		{ thick,  1.0f, thick},
		{-thick,  1.0f, thick},
		{ thick,  0.0f, thick},
		{-thick,  0.0f, thick},
		// -X
		{-thick,  1.0f, thick},
		{-thick,  1.0f,-thick},
		{-thick,  0.0f, thick},
		{-thick,  0.0f,-thick},
		// +Y
		{-thick,  1.0f, thick} ,
		{ thick,  1.0f, thick} ,
		{-thick,  1.0f,-thick} ,
		{ thick,  1.0f,-thick} ,
		// -Y
		{-thick,  0.0f,-thick} ,
		{ thick,  0.0f,-thick} ,
		{-thick,  0.0f, thick} ,
		{ thick,  0.0f, thick} ,
	};
	std::vector<int> idx_base = {
		 0, 1, 2,  1, 3, 2,
		 4, 5, 6,  5, 7, 6,
		 8, 9,10,  9,11,10,
		12,13,14, 13,15,14,
		16,17,18, 17,19,18,
		20,21,22, 21,23,22,
	};

	// ポリゴン構築関数
	auto polygonFunc = [&vtx, &idx, &num, vtx_base, idx_base](Param param) {
		// 頂点構築
		for (int i = 0; i < vtx_base.size(); i++)
		{
			if (vtx_base[i].pos[1] == 1)
			{
				vtx.push_back({
					param.pos.x + param.vec.x + vtx_base[i].pos[0],
					param.pos.y + param.vec.y,
					param.pos.z + param.vec.z + vtx_base[i].pos[2]
					});
			}
			else
			{
				vtx.push_back({
					param.pos.x + vtx_base[i].pos[0],
					param.pos.y + vtx_base[i].pos[1],
					param.pos.z + vtx_base[i].pos[2] 
					});
			}
		}
		// インデックス構築
		for (int i = 0; i < idx_base.size(); i++)
		{
			idx.push_back(idx_base[i] + (num * 24));
		}
		// ポリゴンの数を設定
		num++;
	};

	// LSystem を利用して図形を直線表示する
	LSystem lsystem;
	// ルール追加
	lsystem.AddRule('A', "S[B]-|[B]+|[B]");
	lsystem.AddRule('B', "&FFFA");
	// 移動処理
	auto moveFunc = [&vtx](void* arg) {
		std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
		Param& top = pStack->top();
		vtx.push_back({ top.pos.x, top.pos.y, top.pos.z });
		top.pos.x += top.vec.x; top.pos.y += top.vec.y; top.pos.z += top.vec.z;
		vtx.push_back({ top.pos.x, top.pos.y, top.pos.z });
	};
	lsystem.AddBehavior('F', moveFunc);
	lsystem.AddBehavior('X', moveFunc);
	// スタック処理
	lsystem.AddBehavior('[', [&vtx](void* arg) {
		std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
		pStack->push(pStack->top());
	});
	lsystem.AddBehavior(']', [&vtx](void* arg) {
		std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
		pStack->pop();
	});
	// 回転処理
	lsystem.AddBehavior('+', [&vtx](void* arg) { // Z軸で左回転
		std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
		Param& top = pStack->top();
		const int max_angle = 45;
		DirectX::XMVECTOR q = {
			0 * sinf(-DirectX::XMConvertToRadians(rand() % max_angle) / 2),
			0 * sinf(-DirectX::XMConvertToRadians(rand() % max_angle) / 2),
			1 * sinf(-DirectX::XMConvertToRadians(rand() % max_angle) / 2),
				cosf(-DirectX::XMConvertToRadians(rand() % max_angle) / 2)
		};
		DirectX::XMVECTOR vec = DirectX::XMVector3Rotate(
			{top.vec.x,top.vec.y,top.vec.z,0},
			q
		);
		DirectX::XMFLOAT3 v;
		DirectX::XMStoreFloat3(&v,vec);

		top.vec = { v.x, v.y, v.z };
	});
	lsystem.AddBehavior('-', [&vtx](void* arg) { // Z軸で右回転
		std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
		Param& top = pStack->top();
		const int max_angle = 45;
		DirectX::XMVECTOR q = {
			0 * sinf(DirectX::XMConvertToRadians(rand() % max_angle) / 2),
			0 * sinf(DirectX::XMConvertToRadians(rand() % max_angle) / 2),
			1 * sinf(DirectX::XMConvertToRadians(rand() % max_angle) / 2),
				cosf(DirectX::XMConvertToRadians(rand() % max_angle) / 2)
		};
		DirectX::XMVECTOR vec = DirectX::XMVector3Rotate(
			{ top.vec.x,top.vec.y,top.vec.z,0 },
			q
		);
		DirectX::XMFLOAT3 v;
		DirectX::XMStoreFloat3(&v, vec);

		top.vec = { v.x, v.y, v.z };
	});
	lsystem.AddBehavior('|', [&vtx](void* arg) { // Y軸回転
		std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
		Param& top = pStack->top();
		int max_angle = 45;
		max_angle *= (rand() % 2) ? -1 : 1;
		DirectX::XMVECTOR q = {
			0 * sinf(DirectX::XMConvertToRadians(rand() % max_angle) / 2),
			1 * sinf(DirectX::XMConvertToRadians(rand() % max_angle) / 2),
			0 * sinf(DirectX::XMConvertToRadians(rand() % max_angle) / 2),
				cosf(DirectX::XMConvertToRadians(rand() % max_angle) / 2)
		};
		DirectX::XMVECTOR vec = DirectX::XMVector3Rotate(
			{ top.vec.x,top.vec.y,top.vec.z,0 },
			q
		);
		DirectX::XMFLOAT3 v;
		DirectX::XMStoreFloat3(&v, vec);

		top.vec = { v.x, v.y, v.z };
	});
	// ピッチダウン
	lsystem.AddBehavior('&', [&vtx](void* arg) {
		std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
		Param& top = pStack->top();
		// Z軸回転で下へ回転する量
		int max_angle = 20;
		// クォータニオン構築
		DirectX::XMVECTOR q;
		if (top.vec.x > 0)
		{
			q = {
				0 * sinf(-DirectX::XMConvertToRadians(rand() % max_angle) / 2),
				0 * sinf(-DirectX::XMConvertToRadians(rand() % max_angle) / 2),
				1 * sinf(-DirectX::XMConvertToRadians(rand() % max_angle) / 2),
					cosf(-DirectX::XMConvertToRadians(rand() % max_angle) / 2)
			};
		}
		else
		{
			q = {
				0 * sinf(DirectX::XMConvertToRadians(rand() % max_angle) / 2),
				0 * sinf(DirectX::XMConvertToRadians(rand() % max_angle) / 2),
				1 * sinf(DirectX::XMConvertToRadians(rand() % max_angle) / 2),
					cosf(DirectX::XMConvertToRadians(rand() % max_angle) / 2)
			};
		}
		// クォータニオンを使用してベクトルを回転
		DirectX::XMVECTOR vec = DirectX::XMVector3Rotate(
			{ top.vec.x,top.vec.y,top.vec.z,0 },
			q
		);
		DirectX::XMFLOAT3 v;
		DirectX::XMStoreFloat3(&v, vec);
		top.vec = { v.x, v.y, v.z };
	});
	// スケーリング処理
	lsystem.AddBehavior('S', [&vtx](void* arg) {
		std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
		Param& top = pStack->top();
		top.vec = { 
			top.vec.x * ((100 + (rand() % 101 - 50)) * 0.01f),
			top.vec.y * ((100 + (rand() % 101 - 50)) * 0.01f),
			top.vec.z * ((100 + (rand() % 101 - 50)) * 0.01f) };
	});

	// 構築済みのルール、処理に基づいてLSystem内で頂点データを生成
	lsystem.Execute(10, "FA", &stack);

	// LSystemによるデータ構築後、頂点データに基づいて頂点バッファを生成
	{	// 頂点バッファの生成
		MeshBuffer::Description desc = {};
		desc.pVtx = vtx.data();
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = vtx.size();
		desc.topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		m_pTree = new MeshBuffer(desc);
	}
	{	// オブジェクト用ディスクリプターヒープ作成
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 1;
		m_pShaderHeap = new DescriptorHeap(desc);
	}
	{	// オブジェクト用の定数バッファ作成
		ConstantBuffer::Description desc = {};
		desc.pHeap = m_pShaderHeap;
		desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
		m_pWVP = new ConstantBuffer(desc);
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
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT}
		};
		Pipeline::Description desc = {};
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.VSFile = L"assets/shader/VS_LSystem.cso";
		desc.PSFile = L"assets/shader/PS_LSystemLine.cso";
		desc.pRootSignature = m_pRootSignature->Get();
		desc.RenderTargetNum = 1;
		m_pPipeline = new Pipeline(desc);
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
void SceneLSystem::Uninit()
{
	delete m_pCamera;
	delete m_pTree;
}
void SceneLSystem::Update()
{
	m_pCamera->Update();
}
void SceneLSystem::Draw()
{
	m_pCamera->Draw();

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

	m_pPipeline->Bind();
	m_pShaderHeap->Bind();

	DirectX::XMFLOAT4X4 fMat[3];
	DirectX::XMMATRIX mat[3];

	mat[0] = DirectX::XMMatrixIdentity();
	mat[1] = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(CameraDebug::m_MainPos.x, CameraDebug::m_MainPos.y, CameraDebug::m_MainPos.z, 0.0),
		DirectX::XMVectorSet(CameraDebug::m_MainTarget.x, CameraDebug::m_MainTarget.y, CameraDebug::m_MainTarget.z, 0.0),
		DirectX::XMVectorSet(0.0f,  1.0f,   0.0f, 0.0)
	);
	mat[2] = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(60.0f), 16.f / 9.f, 0.1f, 100.0f
	);

	for (int i = 0; i < 3; ++i) {
		DirectX::XMStoreFloat4x4(&fMat[i], DirectX::XMMatrixTranspose(mat[i]));
	}

	m_pWVP->Write(&fMat);
	D3D12_GPU_DESCRIPTOR_HANDLE handle[] = {
		m_pWVP->GetHandle().hGPU
	};
	m_pRootSignature->Bind(handle, 1);
	m_pTree->Draw();
}

void LSystem::Execute(int iteration, const char * initValue, void * arg)
{
	std::string::iterator it;
	std::string base;				// 変換前文字列
	std::string str = initValue;	// 変換後文字列
	// イテレーション回数分、変換処理を実行
	for (int i = 0; i < iteration; ++i) {
		base = str; // 変換済みの文字列を再度変換
		str = "";	// 変換後の文字を格納できるよう初期化
		it = base.begin();
		while (it != base.end()) {
			auto rule = m_rule.find(*it);	// 変換ルールがあるか探索
			if (rule != m_rule.end())
				str += rule->second;		// 変換後の文字を追加
			else
				str += *it;					// 変換ルールがないので、現在の文字をそのまま追加
			++it;
		}
	}
	// 変換後の文字に対応する処理を実行
	it = str.begin();
	while (it != str.end()) {
		auto behavior = m_behavior.find(*it);
		if (behavior != m_behavior.end())
			behavior->second(arg);
		++it;
	}
}

void LSystem::AddRule(char key, const char * str)
{
	m_rule.insert(std::pair<char, std::string>(key, str));
}

void LSystem::AddBehavior(char key, std::function<void(void*)> func)
{
	m_behavior.insert(std::pair<char, std::function<void(void*)>>(key, func));
}
