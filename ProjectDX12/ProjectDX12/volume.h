#ifndef ___VOLUME_H___
#define ___VOLUME_H___

#include <memory>
#include <vector>

#include "DescriptorHeap.h"
#include "Pipeline.h"
#include "RootSignature.h"

#include "MeshBuffer.h"

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
	static void CopyGlobalTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE dest, UINT key);
	static void CopyTextureSRV(D3D12_CPU_DESCRIPTOR_HANDLE src, D3D12_CPU_DESCRIPTOR_HANDLE dest);

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
	bool Destroy() {
		if (bDestroy)
		{
			return true;
		}
		return false;
	}

protected:
	void BindHeap();
	void BindPipeline(UINT idx);
	void BindPostProcessRTV();
	void Rendering();

private:
	static std::unique_ptr<MeshBuffer>									Screen;
protected:
	std::shared_ptr<DescriptorHeap>										Heap;
	std::shared_ptr<DescriptorHeap>										RTVHeap;
protected:
	std::unique_ptr<RootSignature>										RootSignatureData;
	std::vector<std::unique_ptr<Pipeline>>								PipelineData;
	std::unique_ptr<RenderTarget>										PostProcessRTV;
private:
	bool																bDestroy = false;

};

#endif
