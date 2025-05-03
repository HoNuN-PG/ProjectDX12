
#include "GlobalResourceKey.h"

#include "SceneBase.h"

std::unique_ptr<DescriptorHeap>	 SceneBase::GlobalHeap;
std::unordered_map<UINT, std::unique_ptr<ConstantBuffer>> SceneBase::GlobalResource;

void SceneBase::Initialize()
{
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
