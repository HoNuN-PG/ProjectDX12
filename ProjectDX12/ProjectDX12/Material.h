#ifndef ___MATERIAL_H___
#define ___MATERIAL_H___

#include "DescriptorHeap.h"
#include "RootSignature.h"
#include "Pipeline.h"
#include "ConstantBuffer.h"
#include "Texture.h"

#include <memory>
#include <vector>
#include <string>

class Material
{
public:
	Material() {};
	virtual ~Material() {};

public:
	virtual void Initialize(DescriptorHeap* heap) = 0;
	virtual void Draw() = 0;
	void AddTexture(const char* path);
	void WriteWVP(void* data);
	void WriteParams(void* data, UINT idx);
	void WriteParams(UINT range, UINT startIdx, D3D12_CPU_DESCRIPTOR_HANDLE startHandle, D3D12_DESCRIPTOR_HEAP_TYPE type);

protected:
	void Create
	(
		DescriptorHeap* heap,
		RootSignature::ParameterTable* param,
		UINT paranNum,
		Pipeline::InputLayout* layout,
		UINT layoutNum,
		const wchar_t* vsPath,
		const wchar_t* psPath,
		UINT rtNum
	);
	void Create
	(
		DescriptorHeap* heap,
		RootSignature::ParameterTables* param,
		UINT paranNum,
		Pipeline::InputLayout* layout,
		UINT layoutNum,
		const wchar_t* vsPath,
		const wchar_t* psPath,
		UINT rtNum
	);
	void DrawBase(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT handleNum);

protected:
	DescriptorHeap* Heap;
	std::unique_ptr<RootSignature>					RootSignatureData;
	std::unique_ptr<Pipeline>						PipelineData;
	std::vector<std::unique_ptr<Texture>>			Textures;
	std::unique_ptr<ConstantBuffer>					WVP;
	std::vector<std::unique_ptr<ConstantBuffer>>	Params;

};

#endif