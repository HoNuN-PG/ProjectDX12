#include "SceneLSystem3D.h"
#include <functional>
#include <stack>
#include <string>
#include <map>
#include <DirectXMath.h>

class LSystem3D
{
	// LSystemNEOクラスの実装(プリント参照)
public:
	void Execute(int iteration, const char* initValue, void* arg = nullptr);
	void AddRule(char key, const char* str);						// 変換ルールの追加
	void AddBehavior(char key, std::function<void(void*)> func);	// 実行処理

private:
	std::map<char, std::string> m_rule;						// 変換ルール
	std::map<char, std::function<void(void*)>> m_behavior;	// 文字の動作
};

HRESULT SceneLSystem3D::Init()
{
	m_pCamera = new CameraDebug();
	m_pLight = new LightBase();

	// LSystemを利用した頂点データの作成

	// 頂点座標
	struct Vertex {
		float pos[3];
		float normal[3];
		float color[4];
	};
	std::vector<Vertex> vtx;	// ポリゴンの頂点情報	
	std::vector<int> idx;		// インデックス

	// ポリゴン基本位置及び基本インデックス
	float thick = 0.1f;
	std::vector<Vertex> vtx_base_branch = {
		// -Z
		{{-thick,  1.0f,-thick},{0,0,-1}},
		{{ thick,  1.0f,-thick},{0,0,-1}},
		{{-thick,  0.0f,-thick},{0,0,-1}},
		{{ thick,  0.0f,-thick},{0,0,-1}},
		// +X
		{{ thick,  1.0f,-thick},{ 1,0,0}},
		{{ thick,  1.0f, thick},{ 1,0,0}},
		{{ thick,  0.0f,-thick},{ 1,0,0}},
		{{ thick,  0.0f, thick},{ 1,0,0}},
		// +Z
		{{ thick,  1.0f, thick},{0,0, 1}},
		{{-thick,  1.0f, thick},{0,0, 1}},
		{{ thick,  0.0f, thick},{0,0, 1}},
		{{-thick,  0.0f, thick},{0,0, 1}},
		// -X
		{{-thick,  1.0f, thick},{-1,0,0}},
		{{-thick,  1.0f,-thick},{-1,0,0}},
		{{-thick,  0.0f, thick},{-1,0,0}},
		{{-thick,  0.0f,-thick},{-1,0,0}},
		// +Y
		{{-thick,  1.0f, thick},{0, 1,0}},
		{{ thick,  1.0f, thick},{0, 1,0}},
		{{-thick,  1.0f,-thick},{0, 1,0}},
		{{ thick,  1.0f,-thick},{0, 1,0}},
		// -Y
		{{-thick,  0.0f,-thick},{0,-1,0}},
		{{ thick,  0.0f,-thick},{0,-1,0}},
		{{-thick,  0.0f, thick},{0,-1,0}},
		{{ thick,  0.0f, thick},{0,-1,0}},
	};
	thick = 0.5f;
	std::vector<Vertex> vtx_base_leaf = {
		// -Z
		{{-thick / 2,  1.0f,-thick},{0,0,-1}},
		{{ thick / 2,  1.0f,-thick},{0,0,-1}},
		{{-thick / 2,  0.0f,-thick},{0,0,-1}},
		{{ thick / 2,  0.0f,-thick},{0,0,-1}},
		// +X
		{{ thick / 2,  1.0f,-thick},{ 1,0,0}},
		{{ thick / 2,  1.0f, thick},{ 1,0,0}},
		{{ thick / 2,  0.0f,-thick},{ 1,0,0}},
		{{ thick / 2,  0.0f, thick},{ 1,0,0}},
		// +Z
		{{ thick / 2,  1.0f, thick},{0,0, 1}},
		{{-thick / 2,  1.0f, thick},{0,0, 1}},
		{{ thick / 2,  0.0f, thick},{0,0, 1}},
		{{-thick / 2,  0.0f, thick},{0,0, 1}},
		// -X
		{{-thick / 2,  1.0f, thick},{-1,0,0}},
		{{-thick / 2,  1.0f,-thick},{-1,0,0}},
		{{-thick / 2,  0.0f, thick},{-1,0,0}},
		{{-thick / 2,  0.0f,-thick},{-1,0,0}},
		// +Y
		{{-thick / 2,  1.0f, thick},{0, 1,0}},
		{{ thick / 2,  1.0f, thick},{0, 1,0}},
		{{-thick / 2,  1.0f,-thick},{0, 1,0}},
		{{ thick / 2,  1.0f,-thick},{0, 1,0}},
		// -Y
		{{-thick / 2,  0.0f,-thick},{0,-1,0}},
		{{ thick / 2,  0.0f,-thick},{0,-1,0}},
		{{-thick / 2,  0.0f, thick},{0,-1,0}},
		{{ thick / 2,  0.0f, thick},{0,-1,0}},
	};
	std::vector<int> idx_base = {
		 0, 1, 2,  1, 3, 2,
		 4, 5, 6,  5, 7, 6,
		 8, 9,10,  9,11,10,
		12,13,14, 13,15,14,
		16,17,18, 17,19,18,
		20,21,22, 21,23,22,
	};

	// 頂点バッファの構築関数
	auto makeBuffer = [this] (std::vector<Vertex> v,std::vector<int> i)
	{
		MeshBuffer::Description desc = {};
		desc.pVtx = v.data();
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = v.size();
		desc.pIdx = i.data();
		desc.idxSize = DXGI_FORMAT_R32_UINT;
		desc.idxCount = i.size();
		desc.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_pTree.push_back(new MeshBuffer(desc));
	};

	// スタック情報
	struct Param {
		DirectX::XMFLOAT3 pos; // 現在の位置
		DirectX::XMFLOAT3 vec; // 現在の進行方向
	};
	std::stack<Param> stack;
	stack.push({ DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(0,1,0) });

	// ポリゴン構築関数
	auto makeBranch = [this, &vtx, &idx, vtx_base_branch, idx_base, &makeBuffer](Param param) {
		// 頂点構築
		float length = sqrtf(powf(param.vec.x, 2) + powf(param.vec.y, 2) + powf(param.vec.z, 2)); // ベクトルの長さ
		for (int i = 0; i < vtx_base_branch.size(); i++)
		{
			vtx.push_back({ 
				{vtx_base_branch[i].pos[0], vtx_base_branch[i].pos[1] * length, vtx_base_branch[i].pos[2]},
				{vtx_base_branch[i].normal[0], vtx_base_branch[i].normal[1], vtx_base_branch[i].normal[2]},
				{132 / 255.f, 54 / 255.f, 21 / 255.f, 1},
			});
		}
		// インデックス構築
		for (int i = 0; i < idx_base.size(); i++)
		{
			idx.push_back(idx_base[i]);
		}

		// バッファ構築
		makeBuffer(vtx,idx);

		// ワールド変換行列構築

		// X軸回転
		float radX = -atan2(param.vec.z, param.vec.y);
		// Y軸回転
		float radY = -atan2(param.vec.z, param.vec.x);
		// Z軸回転
		float radZ = -atan2(param.vec.x, param.vec.y);

		DirectX::XMFLOAT3 vec = param.vec;
		DirectX::XMFLOAT3 pos = param.pos;
		
		// 入力ベクトルと基準ベクトルを正規化
		DirectX::XMVECTOR inputVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&vec));
		DirectX::XMVECTOR referenceVec = DirectX::XMVectorSet(0, 1, 0, 0); // 基準ベクトル {0, 1, 0}

		DirectX::XMMATRIX rots;
		if (DirectX::XMVector3Equal(inputVec, referenceVec)) 
		{
			// 一致している場合
			rots = DirectX::XMMatrixIdentity();
		}
		else if (DirectX::XMVector3Equal(inputVec, DirectX::XMVectorNegate(referenceVec))) 
		{
			// 反平行の場合、X軸を基準に回転
			DirectX::XMVECTOR arbitraryAxis = DirectX::XMVectorSet(1, 0, 0, 0);
			rots = DirectX::XMMatrixRotationAxis(arbitraryAxis, DirectX::XM_PI);
		}
		else 
		{
			// 通常の回転計算
			DirectX::XMVECTOR axis = DirectX::XMVector3Cross(referenceVec, inputVec);
			float angle = acosf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(referenceVec, inputVec)));
			rots = DirectX::XMMatrixRotationAxis(axis, angle);
		}

		DirectX::XMMATRIX scale, rot, trans;
		scale = DirectX::XMMatrixScaling(1, 1, 1);
		rot = rots; //DirectX::XMMatrixRotationRollPitchYaw(radX, radY, radZ);
		trans = DirectX::XMMatrixTranslation(param.pos.x, param.pos.y, param.pos.z);
		DirectX::XMMATRIX W = scale * rot * trans;
		m_W.push_back(W);

		// クリア
		vtx.clear();
		idx.clear();
	};

	// ポリゴン構築関数
	auto makeLeaf = [this, &vtx, &idx, vtx_base_leaf, idx_base, &makeBuffer](Param param) {
		// 頂点構築
		float length = sqrtf(powf(param.vec.x, 2) + powf(param.vec.y, 2) + powf(param.vec.z, 2)); // ベクトルの長さ
		for (int i = 0; i < vtx_base_leaf.size(); i++)
		{
			vtx.push_back({
				{vtx_base_leaf[i].pos[0], vtx_base_leaf[i].pos[1], vtx_base_leaf[i].pos[2]},
				{vtx_base_leaf[i].normal[0], vtx_base_leaf[i].normal[1], vtx_base_leaf[i].normal[2]},
				{0.2f,1.0f,0.2f,1.0f},
				});
		}
		// インデックス構築
		for (int i = 0; i < idx_base.size(); i++)
		{
			idx.push_back(idx_base[i]);
		}

		// バッファ構築
		makeBuffer(vtx, idx);

		// ワールド変換行列構築

		// X軸回転
		float radX = -atan2(param.vec.z, param.vec.y);
		// Y軸回転
		float radY = -atan2(param.vec.z, param.vec.x);
		// Z軸回転
		float radZ = -atan2(param.vec.x, param.vec.y);

		DirectX::XMFLOAT3 vec = param.vec;
		DirectX::XMFLOAT3 pos = param.pos;

		// 入力ベクトルと基準ベクトルを正規化
		DirectX::XMVECTOR inputVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&vec));
		DirectX::XMVECTOR referenceVec = DirectX::XMVectorSet(0, 1, 0, 0); // 基準ベクトル {0, 1, 0}

		DirectX::XMMATRIX rots;
		if (DirectX::XMVector3Equal(inputVec, referenceVec))
		{
			// 一致している場合
			rots = DirectX::XMMatrixIdentity();
		}
		else if (DirectX::XMVector3Equal(inputVec, DirectX::XMVectorNegate(referenceVec)))
		{
			// 反平行の場合、X軸を基準に回転
			DirectX::XMVECTOR arbitraryAxis = DirectX::XMVectorSet(1, 0, 0, 0);
			rots = DirectX::XMMatrixRotationAxis(arbitraryAxis, DirectX::XM_PI);
		}
		else
		{
			// 通常の回転計算
			DirectX::XMVECTOR axis = DirectX::XMVector3Cross(referenceVec, inputVec);
			float angle = acosf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(referenceVec, inputVec)));
			rots = DirectX::XMMatrixRotationAxis(axis, angle);
		}

		DirectX::XMMATRIX scale, rot, trans;
		scale = DirectX::XMMatrixScaling(1, 1, 1);
		rot = rots; //DirectX::XMMatrixRotationRollPitchYaw(radX, radY, radZ);
		trans = DirectX::XMMatrixTranslation(param.pos.x, param.pos.y, param.pos.z);
		DirectX::XMMATRIX W = scale * rot * trans;
		m_W.push_back(W);

		// クリア
		vtx.clear();
		idx.clear();
	};

	// LSystemNEO を利用して図形を直線表示する
	LSystem3D lsystem;

	// ルール追加
	lsystem.AddRule('A', "S[B]-|[B]+|[B]");
	lsystem.AddRule('B', "&FFFA");

	// 移動を行いながら枝を構築していく
	auto moveFunc = [&makeBranch](void* arg) {
		std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
		Param& top = pStack->top();
		makeBranch(top);
		top.pos.x += top.vec.x; top.pos.y += top.vec.y; top.pos.z += top.vec.z;
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
	lsystem.AddBehavior('+', [&vtx](void* arg) { // Z軸で右回転
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
			{ top.vec.x,top.vec.y,top.vec.z,0 },
			q
		);
		DirectX::XMFLOAT3 v;
		DirectX::XMStoreFloat3(&v, vec);

		top.vec = { v.x, v.y, v.z };
	});
	lsystem.AddBehavior('-', [&vtx](void* arg) { // Z軸で左回転
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
		int max_angle = 75;
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
	auto pitchDown = [](Param& top, float max)
	{
		// Z軸回転で下へ回転する量
		int max_angle = max;
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
	};
	lsystem.AddBehavior('&', [&vtx, &pitchDown](void* arg) {
		std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
		Param& top = pStack->top();
		// Z軸回転で下へ回転する量
		pitchDown(top,20);
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

	// 停止位置に葉を構築
	auto stopFunc = [&makeLeaf, &pitchDown](void* arg) {
		std::stack<Param>* pStack = reinterpret_cast<std::stack<Param>*>(arg);
		Param& top = pStack->top();
		pitchDown(top,65);
		makeLeaf(top);
	};
	lsystem.AddBehavior('A', stopFunc);

	// 構築済みのルール、処理に基づいてLSystemNEO内で頂点データを生成
	lsystem.Execute(8, "FA", &stack);

	{	// オブジェクト用ディスクリプターヒープ作成
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = m_pTree.size() * 2;
		m_pShaderHeap = new DescriptorHeap(desc);
	}
	{	// オブジェクト用の定数バッファ作成
		for (int i = 0; i < m_pTree.size(); i++)
		{
			ConstantBuffer::Description desc = {};
			desc.pHeap = m_pShaderHeap;
			desc.size = sizeof(DirectX::XMFLOAT4X4) * 3;
			m_pWVP.push_back(new ConstantBuffer(desc));
		}
		for (int i = 0; i < m_pTree.size(); i++)
		{
			ConstantBuffer::Description desc = {};
			desc.pHeap = m_pShaderHeap;
			desc.size = sizeof(DirectX::XMFLOAT4X4);
			m_pLightBuffer.push_back(new ConstantBuffer(desc));
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
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT}
		};
		Pipeline::Description desc = {};
		desc.pInputLayout = layout;
		desc.InputLayoutNum = _countof(layout);
		desc.VSFile = L"assets/shader/VS_LSystem.cso";
		desc.PSFile = L"assets/shader/PS_LSystem.cso";
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
void SceneLSystem3D::Uninit()
{
	delete m_pCamera;
	delete m_pLight;
	for(int i = 0;i < m_pTree.size();i++)
	{
		delete m_pTree[i];
	}
	m_pTree.clear();
	m_W.clear();
	m_pWVP.clear();
	m_pLightBuffer.clear();
}
void SceneLSystem3D::Update()
{
	m_pCamera->Update();
	m_pLight->Update();
}
void SceneLSystem3D::Draw()
{
	m_pCamera->Draw();
	m_pLight->Draw();

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

	for (int i = 0; i < m_pTree.size(); i++)
	{
		mat[0] = m_W[i];
		mat[1] = DirectX::XMMatrixLookAtLH(
			DirectX::XMVectorSet(CameraDebug::m_MainPos.x, CameraDebug::m_MainPos.y, CameraDebug::m_MainPos.z, 0.0),
			DirectX::XMVectorSet(CameraDebug::m_MainTarget.x, CameraDebug::m_MainTarget.y, CameraDebug::m_MainTarget.z, 0.0),
			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0)
		);
		mat[2] = DirectX::XMMatrixPerspectiveFovLH(
			DirectX::XMConvertToRadians(60.0f), 16.f / 9.f, 0.1f, 100.0f
		);

		for (int j = 0; j < 3; ++j) {
			DirectX::XMStoreFloat4x4(&fMat[j], DirectX::XMMatrixTranspose(mat[j]));
		}

		DirectX::XMFLOAT4X4 light;
		light._11 = m_pLight->GetDir().x;
		light._12 = m_pLight->GetDir().y;
		light._13 = m_pLight->GetDir().z;
		light._14 = m_pLight->GetPower();
		light._21 = m_pLight->GetColor().x;
		light._22 = m_pLight->GetColor().y;
		light._23 = m_pLight->GetColor().z;
		light._24 = m_pLight->GetAmbient();

		m_pWVP[i]->Write(&fMat);
		m_pLightBuffer[i]->Write(&light);
		D3D12_GPU_DESCRIPTOR_HANDLE handle[] = {
			m_pWVP[i]->GetHandle().hGPU,
			m_pLightBuffer[i]->GetHandle().hGPU,
		};
		m_pRootSignature->Bind(handle, _countof(handle));
		m_pTree[i]->Draw();
	}
}

void LSystem3D::Execute(int iteration, const char * initValue, void * arg)
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

void LSystem3D::AddRule(char key, const char * str)
{
	m_rule.insert(std::pair<char, std::string>(key, str));
}

void LSystem3D::AddBehavior(char key, std::function<void(void*)> func)
{
	m_behavior.insert(std::pair<char, std::function<void(void*)>>(key, func));
}
