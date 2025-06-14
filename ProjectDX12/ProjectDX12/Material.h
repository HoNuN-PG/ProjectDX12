#ifndef ___MATERIAL_H___
#define ___MATERIAL_H___

#include "ConstantBuffer.h"
#include "DescriptorHeap.h"
#include "Pipeline.h"
#include "RootSignature.h"
#include "Texture.h"

#include "RenderingPass.h"

#include <string>
#include <vector>
#include <memory>
#include <DirectXMath.h>

class Material
{
public:
	enum RenderingTiming
	{
		Shadow = 0,					// シャドウ
		OpaqueDepthNormal,			// 不透明深度法線
		Deffered,					// ディファード
		Forward,					// フォワード
		TranslucentDepthNormal,		// 透明深度法線
		Canvas,						// キャンバス
		Other,						// その他

		MAX_RENDERING_PASS_TYPE
	};

public:
	Material() {};
	virtual ~Material() {};

public:
	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize(DescriptorHeap* heap,
		RenderingTiming timing = RenderingTiming::Forward) = 0;
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

public:
	RenderingTiming GetRenderTiming() { return Timing; };
protected:
	RenderingTiming									Timing;
	DescriptorHeap*									Heap;
	std::unique_ptr<RootSignature>					RootSignatureData;
	std::unique_ptr<Pipeline>						PipelineData;
	std::vector<std::unique_ptr<Texture>>			Textures;
	std::unique_ptr<ConstantBuffer>					WVP;
	std::vector<std::unique_ptr<ConstantBuffer>>	Params;

};

#endif