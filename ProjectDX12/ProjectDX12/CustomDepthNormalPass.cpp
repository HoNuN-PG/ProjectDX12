
#include "CustomDepthNormalPass.h"

#include "GameObject.h"
#include "RenderTarget.h"

CustomDepthNormalPass::CustomDepthNormalPass()
{
	PassType = RenderingPass::RenderingPassType::CustomDepthNormal;
}

void CustomDepthNormalPass::Execute()
{
	static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0 };

	// ターゲット化
	CustomDepthTexture->ResourceBarrier(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CustomNormalTexture->ResourceBarrier(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// RTVの設定
	CustomDepthTexture->Clear(clearColor);
	CustomNormalTexture->Clear(clearColor);
	DSV->Clear();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
		CustomDepthTexture->GetHandleRTV().hCPU,
		CustomNormalTexture->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs, DSV->GetHandleDSV().hCPU);

	for (int i = 0; i < RenderObjects.size(); ++i)
	{
		RenderObjects[i].obj.RenderingBase();
	}

	// リソース化
	CustomDepthTexture->ResourceBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CustomNormalTexture->ResourceBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	RenderObjects.clear();
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
		CustomDepthTexture = std::make_shared<RenderTarget>(desc);
		desc.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		CustomNormalTexture = std::make_shared<RenderTarget>(desc);
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
	case(TextureType::CustomDepth):
		return CustomDepthTexture;
	case(TextureType::CustomNormal):
		return CustomNormalTexture;
	default:
		break;
	}
	return nullptr;
}

DescriptorHeap::Handle CustomDepthNormalPass::GetTextureRTV(UINT idx)
{
	switch (idx)
	{
	case(TextureType::CustomDepth):
		return CustomDepthTexture->GetHandleRTV();
	case(TextureType::CustomNormal):
		return CustomNormalTexture->GetHandleRTV();
	default:
		break;
	}
	return DescriptorHeap::Handle();
}

DescriptorHeap::Handle CustomDepthNormalPass::GetTextureSRV(UINT idx)
{
	switch (idx)
	{
	case(TextureType::CustomDepth):
		return CustomDepthTexture->GetHandleSRV();
	case(TextureType::CustomNormal):
		return CustomNormalTexture->GetHandleSRV();
	default:
		break;
	}
	return DescriptorHeap::Handle();
}
