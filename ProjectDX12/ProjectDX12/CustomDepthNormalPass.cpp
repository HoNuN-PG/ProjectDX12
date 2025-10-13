
#include "GameObject.h"

#include "CustomDepthNormalPass.h"

#include "RenderTarget.h"

CustomDepthNormalPass::CustomDepthNormalPass()
{
	PassType = RenderingPass::RenderingPassType::CustomDepthNormal;
}

void CustomDepthNormalPass::Execute()
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
	DSV->Clear();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
		Depth->GetHandleRTV().hCPU,
		Normal->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs, DSV->GetHandleDSV().hCPU);

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

void CustomDepthNormalPass::Init(
	std::shared_ptr<DescriptorHeap> rtvHeap, 
	std::shared_ptr<DescriptorHeap> srvHeap, 
	std::shared_ptr<DescriptorHeap> dsvHeap)
{
	// RTV
	{
		RenderTarget::Description desc = {};
		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		desc.format = DXGI_FORMAT_R16G16_FLOAT;
		desc.pRTVHeap = rtvHeap.get();
		desc.pSRVHeap = srvHeap.get();
		Depth = std::make_shared<RenderTarget>(desc);
		desc.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Normal = std::make_shared<RenderTarget>(desc);
	}
	// DSV
	{
		DepthStencil::Description desc = {};
		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		desc.pDSVHeap = dsvHeap.get();
		DSV = std::make_unique<DepthStencil>(desc);
	}
}

void CustomDepthNormalPass::AddObj(GameObject& obj)
{
	RenderingEngine::RenderingInfo info = { obj };
	RenderObjects.push_back(info);
}

std::shared_ptr<RenderTarget> CustomDepthNormalPass::GetTexture(UINT idx)
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

DescriptorHeap::Handle CustomDepthNormalPass::GetTextureRTV(UINT idx)
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

DescriptorHeap::Handle CustomDepthNormalPass::GetTextureSRV(UINT idx)
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
