#ifndef ___VOLUME_H___
#define ___VOLUME_H___

#include <memory>
#include <vector>

#include "DescriptorHeap.h"
#include "MeshBuffer.h"
#include "Pipeline.h"
#include "RenderTarget.h"
#include "RootSignature.h"

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

private:
	static std::unique_ptr<MeshBuffer>									Screen;
	std::unique_ptr<RootSignature>										RootSignatureData;
	std::unique_ptr<Pipeline>											PipelineData;
	std::vector<std::unique_ptr<RenderTarget>>							RTVs;
	std::unique_ptr<DescriptorHeap> RTVHeap;
	std::unique_ptr<DescriptorHeap> Heap;
};

#endif
