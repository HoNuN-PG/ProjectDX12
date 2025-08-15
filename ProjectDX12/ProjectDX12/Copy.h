#ifndef ___COPY_H___
#define ___COPY_H___

#include <memory>
#include <vector>

#include "MeshBuffer.h"
#include "Pipeline.h"
#include "RootSignature.h"

class DescriptorHeap;
class RenderTarget;

class Copy
{
public:
	struct Vertex
	{
		float pos[3];
		float uv[2];
	};

public:
	Copy() {};
	~Copy() {};
	static void Load();
	static void ExecuteCopy(DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE src, D3D12_CPU_DESCRIPTOR_HANDLE dest);
	static void ExecuteCopy(DescriptorHeap* heap, D3D12_GPU_DESCRIPTOR_HANDLE src, std::shared_ptr<RenderTarget> dest);

private:
	static std::unique_ptr<MeshBuffer>									Screen;
	static std::unique_ptr<RootSignature>								RootSignatureData;
	static std::unique_ptr<Pipeline>									PipelineData;

};

#endif