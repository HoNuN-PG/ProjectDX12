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
	Volume() {};
	virtual ~Volume() {};
	static void Load();
	static void Unload();

	virtual void Init() {};
	virtual void Draw() {};

protected:
	void BindPostProcessRTV();
	void BindPipeline();
	void BindHeap();
	void GetGlobalSRV(std::shared_ptr<RenderTarget> dest, UINT key);
	void Rendering();

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

private:
	static std::unique_ptr<MeshBuffer>									Screen;
protected:
	static std::shared_ptr<DescriptorHeap>								Heap;
	static std::shared_ptr<DescriptorHeap>								RTVHeap;
	static std::unique_ptr<RenderTarget>								PostProcessRTV;
protected:
	std::unique_ptr<RootSignature>										RootSignatureData;
	std::unique_ptr<Pipeline>											PipelineData;
	std::vector<std::unique_ptr<RenderTarget>>							CustomRTVs;
private:
	bool																bDestroy = false;

};

#endif
