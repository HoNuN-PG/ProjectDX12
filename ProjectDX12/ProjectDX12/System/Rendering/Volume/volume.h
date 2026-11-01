#ifndef ___VOLUME_H___
#define ___VOLUME_H___

#include <memory>
#include <vector>

#include "Model/MeshBuffer.h"
#include "System/Rendering/ConstantBuffer/ConstantBuffer.h"
#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "System/Rendering/Pipeline/PipelineState.h"
#include "System/Rendering/Pipeline/RootSignature.h"
#include "System/Rendering/Texture/RenderTarget.h"

/**
 * @class Volume
 * @brief ポストプロセス本体
 */
class Volume
{
public:

	Volume() {};
	virtual ~Volume() {};

	virtual void Init() {};
	virtual void Draw() {};

protected:

	/// <summary>
	/// ヒープとポストプロセス用RTの作成（派生クラスのInitから呼ぶ）
	/// </summary>
	void InitResources(UINT heapNum, UINT rtvNum);

private:
	
	friend class PostProcess;
	void SetScreen(MeshBuffer& screen) { pScreen = &screen; }
	
public:

	void SetDestroy() { bDestroy = true; }
	bool IsDestroy() { return bDestroy; }

private:

	bool bDestroy = false;

protected:

	void BindHeap();
	void BindPostProcessRTV();
	void BindRootSignature(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT num);
	void BindPipeline(UINT idx);
	void Rendering();

protected:

	std::unique_ptr<DescriptorHeap>	pHeap;
	std::unique_ptr<DescriptorHeap>	pRTVHeap;
	MeshBuffer* pScreen = nullptr; // 全画面描画用の四角形（RenderingEngineが所有しPostProcessより長生きする）
	std::unique_ptr<RenderTarget> pPostProcessRTV;
	std::unique_ptr<RootSignature> pRootSignatureData;
	std::vector<std::unique_ptr<PipelineState>> pPipelineData;
	std::unique_ptr<ConstantBuffer> Params;

};

#endif
