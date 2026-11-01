#ifndef ___COPY_H___
#define ___COPY_H___

#include <memory>
#include <vector>

#include "Model/MeshBuffer.h"
#include "System/Rendering/Pipeline/RootSignature.h"
#include "System/Rendering/Volume/PostProcess.h"

class DescriptorHeap;
class RenderTarget;

class Copy
{
public:

	explicit Copy(MeshBuffer& screen);
	~Copy() {};
	void ExecuteCopy2BBuffer(DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE src);
	void ExecuteCopy(DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE src, RenderTarget* dest);

private:

	void Load();

private:

	MeshBuffer* pScreen; // 全画面描画用の四角形（RenderingEngineが所有しCopyより長生きする）
	std::unique_ptr<RootSignature> pRootSignatureData;
	std::unordered_map<DXGI_FORMAT,std::unique_ptr<PipelineState>> pPipelineData;

};

#endif