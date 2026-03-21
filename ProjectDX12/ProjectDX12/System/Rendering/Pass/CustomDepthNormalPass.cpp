
#include "System/GameObject/GameObject.h"
#include "System/Rendering/Pass/CustomDepthNormalPass.h"
#include "System/Rendering/Texture/RenderTarget.h"

CustomDepthNormalPass::CustomDepthNormalPass()
{
	PassType = RenderingPass::RenderingPassType::CustomDepthNormal;

	PassFormats.resize(TextureType::MAX);
	PassFormats[TextureType::DepthTexture] = DXGI_FORMAT_R16G16_FLOAT;
	PassFormats[TextureType::NormalTexture] = DXGI_FORMAT_R8G8B8A8_UNORM;
}

void CustomDepthNormalPass::Execute()
{
	static const float clearColor[4] = { 0, 0, 0, 0 };

	// ターゲット化
	Depth->SRV2RTV();
	Normal->SRV2RTV();

	// RTVの設定
	Depth->Clear(clearColor);
	Normal->Clear(clearColor);
	DSV->Clear();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
	{
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
	Depth->RTV2SRV();
	Normal->RTV2SRV();
}

void CustomDepthNormalPass::Init(
	std::shared_ptr<DescriptorHeap> rtvHeap, 
	std::shared_ptr<DescriptorHeap> stagingHeap,
	std::shared_ptr<DescriptorHeap> srvHeap, 
	std::shared_ptr<DescriptorHeap> dsvHeap
)
{
	// RTV
	{
		RenderTarget::Description desc = {};
		desc.width		= WINDOW_WIDTH;
		desc.height		= WINDOW_HEIGHT;
		desc.format		= DXGI_FORMAT_R16G16_FLOAT;
		desc.pRTVHeap	= rtvHeap.get();
		desc.pSRVHeap	= srvHeap.get();
		Depth			= std::make_shared<RenderTarget>(desc);
		desc.format		= DXGI_FORMAT_R8G8B8A8_UNORM;
		Normal			= std::make_shared<RenderTarget>(desc);
	}
	// DSV
	{
		DepthStencil::Description desc = {};
		desc.width		= WINDOW_WIDTH;
		desc.height		= WINDOW_HEIGHT;
		desc.pDSVHeap	= dsvHeap.get();
		DSV				= std::make_unique<DepthStencil>(desc);
	}
}

void CustomDepthNormalPass::AddObj(GameObject& obj)
{
	RenderingEngine::RenderingInfo info = { obj };
	RenderObjects.push_back(info);
}

std::shared_ptr<RenderTarget> CustomDepthNormalPass::GetTextureStaging(UINT idx)
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

DescriptorHeap::Handle CustomDepthNormalPass::GetTextureStagingRTV(UINT idx)
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

DescriptorHeap::Handle CustomDepthNormalPass::GetTextureStagingSRV(UINT idx)
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

std::vector<DXGI_FORMAT> CustomDepthNormalPass::GetPassFormat()
{
	return PassFormats;
}
