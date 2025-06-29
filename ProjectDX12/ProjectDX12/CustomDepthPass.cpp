
#include "CustomDepthPass.h"

#include "GameObject.h"
#include "RenderTarget.h"

CustomDepthPass::CustomDepthPass(
	std::shared_ptr<DescriptorHeap> rtvHeap, 
	std::shared_ptr<DescriptorHeap> srvHeap, 
	std::shared_ptr<DescriptorHeap> dsvHeap)
{
	PassType = RenderingPass::RenderingPassType::CustomDepth;
	// RTV
	{
		RenderTarget::Description desc = {};
		desc.width = WINDOW_WIDTH;
		desc.height = WINDOW_HEIGHT;
		desc.format = DXGI_FORMAT_R16G16_FLOAT;
		desc.pRTVHeap = rtvHeap.get();
		desc.pSRVHeap = srvHeap.get();
		CustomDepthTexture = std::make_shared<RenderTarget>(desc);
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

void CustomDepthPass::Execute()
{
	static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0 };

	// ターゲット化
	CustomDepthTexture->ResourceBarrier(
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// RTVの設定
	CustomDepthTexture->Clear(clearColor);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
		CustomDepthTexture->GetHandleRTV().hCPU,
	};
	SetRenderTarget(_countof(rtvs), rtvs, DSV->GetHandleDSV().hCPU);

	// フォワードレンダリング
	for (int i = 0; i < RenderObjects.size(); ++i)
	{
		RenderObjects[i].obj.RenderingBase();
	}

	// リソース化
	CustomDepthTexture->ResourceBarrier(
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	RenderObjects.clear();
}

void CustomDepthPass::AddObj(GameObject& obj)
{
	RenderingEngine::RenderingInfo info = { obj };
	RenderObjects.push_back(info);
}

std::shared_ptr<RenderTarget> CustomDepthPass::GetTexture(UINT idx)
{
	return CustomDepthTexture;
}

DescriptorHeap::Handle CustomDepthPass::GetTextureRTV(UINT idx)
{
	return CustomDepthTexture->GetHandleRTV();
}

DescriptorHeap::Handle CustomDepthPass::GetTextureSRV(UINT idx)
{
	return CustomDepthTexture->GetHandleSRV();
}
