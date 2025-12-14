#ifndef ___VOLUME_H___
#define ___VOLUME_H___

#include <memory>
#include <vector>

// Model
#include "MeshBuffer.h"

// System/Rendering/Pipeline
#include "DescriptorHeap.h"
#include "Pipeline.h"
#include "RootSignature.h"
// System/Rendering/Texture
#include "RenderTarget.h"

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
	std::shared_ptr<DescriptorHeap>										Heap;
	std::shared_ptr<DescriptorHeap>										RTVHeap;

private:
	static std::unique_ptr<MeshBuffer>									Screen;
protected:
	std::unique_ptr<RootSignature>										RootSignatureData;
	std::vector<std::unique_ptr<Pipeline>>								PipelineData;
	std::unique_ptr<RenderTarget>										PostProcessRTV;

};

#endif
