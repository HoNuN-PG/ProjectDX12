
#include "GlobalResourceKey.h"

#include "SceneBase.h"

std::unique_ptr<DescriptorHeap>	 SceneBase::GlobalHeap;
std::unordered_map<UINT, std::unique_ptr<ConstantBuffer>> SceneBase::GlobalResource;

HRESULT SceneBase::InitBase()
{
	Initialize();
	Init();
	return E_NOTIMPL;
}

void SceneBase::UninitBase()
{
	for (auto& objectList : GameObjects)
	{
		for (GameObject* object : objectList)
		{
			object->UninitBase();
			delete object;
		}
		objectList.clear();
	}
	Uninit();
}

void SceneBase::UpdateBase()
{
	for (int i = 0; i < MAX_LAYER; i++)
	{
		for (GameObject* object : GameObjects[i])
		{
			object->UpdateBase();
		}
	}
	for (auto& objectList : GameObjects)
	{
		objectList.remove_if(
			[](GameObject* object) {return object->Destroy(); });
	}
	Update();
}

void SceneBase::DrawBase()
{
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

	WriteGlobalResource();

	DirectX::XMFLOAT4X4 matrix;
	DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixIdentity());
	for (int i = 0; i < MAX_LAYER; i++)
	{
		for (GameObject* object : GameObjects[i])
		{
			object->DrawBase(matrix);
		}
	}
	Draw();
}

void SceneBase::Initialize()
{
	// ディスクリプタヒープ（深度バッファ)
	{
		DescriptorHeap::Description desc	= {};
		desc.heapType						= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.num							= 1;
		DSVHeap								= std::make_unique<DescriptorHeap>(desc);
	}
	// 深度バッファ
	{
		DepthStencil::Description desc	= {};
		desc.width						= WINDOW_WIDTH;
		desc.height						= WINDOW_HEIGHT;
		desc.pDSVHeap					= DSVHeap.get();
		DSV								= std::make_unique<DepthStencil>(desc);
	}

	Camera = std::make_unique<CameraDebug>();
	Light = std::make_unique<LightBase>();

	// グローバルディスクリプタヒープ
	{
		DescriptorHeap::Description desc = {};
		desc.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.num = 64;
		GlobalHeap = std::make_unique<DescriptorHeap>(desc);
	}
	// グローバルリソース
	{
		ConstantBuffer::Description desc = {};
		desc.pHeap = GlobalHeap.get();
		desc.size = sizeof(DirectX::XMFLOAT4X4);
		GlobalResource[GlobalResourceKey::Camera] = std::make_unique<ConstantBuffer>(desc);
		GlobalResource[GlobalResourceKey::Light] = std::make_unique<ConstantBuffer>(desc);
	}
}

void SceneBase::WriteGlobalResource()
{
	// カメラ
	DirectX::XMFLOAT4X4 camera;
	camera._11 = CameraDebug::m_MainPos.x;
	camera._12 = CameraDebug::m_MainPos.y;
	camera._13 = CameraDebug::m_MainPos.z;
	camera._14 = 0.0f;
	GlobalResource[GlobalResourceKey::Camera]->Write(&camera);
	// ライト
	DirectX::XMFLOAT4X4 light;
	light._11 = Light->GetDir().x;
	light._12 = Light->GetDir().y;
	light._13 = Light->GetDir().z;
	light._14 = Light->GetPower();
	light._21 = Light->GetColor().x;
	light._22 = Light->GetColor().y;
	light._23 = Light->GetColor().z;
	light._24 = Light->GetAmbient();
	GlobalResource[GlobalResourceKey::Light]->Write(&light);
}

ConstantBuffer* SceneBase::GetGlobalResource(UINT key)
{
	if (!GlobalResource.contains(key))
		return nullptr;
	return GlobalResource[key].get();
}
