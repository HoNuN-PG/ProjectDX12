
// System/GameObject
#include "GameObject.h"
// System/Rendering/Pass
#include "DepthNormalPass.h"
// System/Rendering
#include "GlobalResourceKey.h"

OpaqueDepthNormalPass::OpaqueDepthNormalPass()
{
	PassType = RenderingPass::RenderingPassType::OpaqueDepthNormal;
	DSV = Engine->GetDSV();
}

void OpaqueDepthNormalPass::Execute()
{
	static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0 };

	// ターゲット化
	Depth->ResourceBarrier(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	Normal->ResourceBarrier(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// RTVの設定
	Depth->Clear(clearColor);
	Normal->Clear(clearColor);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
		Depth->GetHandleRTV().hCPU,
		Normal->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs, DSV->GetHandleDSV().hCPU);

	// DepthNormal
	for (int i = 0; i < RenderObjects.size(); ++i)
	{
		RenderObjects[i].obj.RenderingBase();
	}
	RenderObjects.clear();

	// リソース化
	Depth->ResourceBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	Normal->ResourceBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void OpaqueDepthNormalPass::Init(
	std::shared_ptr<DescriptorHeap> rtvHeap, 
	std::shared_ptr<DescriptorHeap> srvHeap, 
	std::shared_ptr<DescriptorHeap> dsvHeap)
{
	// RTV
	{
		RenderTarget::Description desc = {};
		desc.format = DXGI_FORMAT_R16G16_FLOAT;
		desc.pRTVHeap = rtvHeap.get();
		desc.pSRVHeap = srvHeap.get();

		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		desc.format = DXGI_FORMAT_R32G32_FLOAT;
		Depth = std::make_shared<RenderTarget>(desc);
		desc.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Normal = std::make_shared<RenderTarget>(desc);
	}
}

void OpaqueDepthNormalPass::AddObj(GameObject& obj)
{
	RenderingEngine::RenderingInfo info = { obj };
	RenderObjects.push_back(info);
}

std::shared_ptr<RenderTarget> OpaqueDepthNormalPass::GetTexture(UINT idx)
{
	switch (idx)
	{
	case(TextureType::DepthTexture):
		return Depth;
	case(TextureType::NormalTexture):
		return Normal;
	default:
		break;
	}
	return nullptr;
}

DescriptorHeap::Handle OpaqueDepthNormalPass::GetTextureRTV(UINT idx)
{
	switch (idx)
	{
	case(TextureType::DepthTexture):
		return Depth->GetHandleRTV();
	case(TextureType::NormalTexture):
		return Normal->GetHandleRTV();
	default:
		break;
	}
	return DescriptorHeap::Handle();
}

DescriptorHeap::Handle OpaqueDepthNormalPass::GetTextureSRV(UINT idx)
{
	switch (idx)
	{
	case(TextureType::DepthTexture):
		return Depth->GetHandleSRV();
	case(TextureType::NormalTexture):
		return Normal->GetHandleSRV();
	default:
		break;
	}
	return DescriptorHeap::Handle();
}
