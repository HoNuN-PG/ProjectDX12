#ifndef ___VOLUME_H___
#define ___VOLUME_H___

#include <memory>
#include <vector>

#include "Model/MeshBuffer.h"
#include "System/Rendering/Pipeline/DescriptorHeap.h"
#include "System/Rendering/Pipeline/PipelineState.h"
#include "System/Rendering/Pipeline/RootSignature.h"
#include "System/Rendering/Texture/RenderTarget.h"

class Volume
{
public:

	struct Vertex
	{
		float pos[3];
		float uv[2];
	};

public:

	static void Load();
	static void Unload();

public:

	Volume() {};
	virtual ~Volume() {};

	virtual void Init() {};
	virtual void Draw() {};

protected:

	virtual void Init(UINT heapNum,UINT rtvNum);

public:

	void SetDestroy() { bDestroy = true; }
	bool IsDestroy() { return bDestroy; }
	bool Destroy() 
	{
		if (bDestroy)
		{
			return true;
		}
		return false;
	}
private:
	bool bDestroy = false;

protected:

	void BindPostProcessRTV();
	void BindHeap();
	void BindRootSignature(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT num);
	void BindPipeline(UINT idx);
	void Rendering();

protected:

	std::shared_ptr<DescriptorHeap>	pHeap;
	std::shared_ptr<DescriptorHeap>	pRTVHeap;

private:
	static std::unique_ptr<MeshBuffer> pScreen;
protected:
	std::unique_ptr<RenderTarget> pPostProcessRTV;
	std::unique_ptr<RootSignature> pRootSignatureData;
	std::vector<std::unique_ptr<PipelineState>> pPipelineData;

};

#endif
