#ifndef ___MATERIAL_H___
#define ___MATERIAL_H___

#include "ConstantBuffer.h"
#include "DescriptorHeap.h"
#include "Pipeline.h"
#include "RootSignature.h"
#include "Texture.h"

#include <string>
#include <vector>
#include <memory>
#include <DirectXMath.h>

class Material
{
public:
	enum RenderingPassType
	{
		SHADOW = 0,					// シャドウ
		O_DEPTH_NORMAL_PASS,		// 不透明深度
		MAIN,						// メイン
		T_DEPTH_NORMAL_PASS,		// 透明深度
		OTHER,						// その他

		MAX_RENDERING_PASS
	};
	enum RenderingTiming
	{
		CAM = 0,
		SKYBOX,
		LIGHT,
		DEFERRED,
		FORWARD,
		CANVAS,
		AFTER_POSTPROCESS,
		AFTER_FRAME_BUFFER,

		MAX_TIMING
	};

public:
	Material() {};
	virtual ~Material() {};

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="heap"></param>
	/// <param name="pass">実行パス</param>
	/// <param name="timing">パス内での描画タイミング</param>
	virtual void Initialize(DescriptorHeap* heap,
		RenderingPassType pass = RenderingPassType::MAIN ,RenderingTiming timing = RenderingTiming::FORWARD) = 0;
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
	RenderingTiming GetRenderingTiming() { return Timing; }
	RenderingPassType GetRenderingPassType() { return Pass; }
protected:
	RenderingTiming									Timing;
	RenderingPassType								Pass;
	DescriptorHeap*									Heap;
	std::unique_ptr<RootSignature>					RootSignatureData;
	std::unique_ptr<Pipeline>						PipelineData;
	std::vector<std::unique_ptr<Texture>>			Textures;
	std::unique_ptr<ConstantBuffer>					WVP;
	std::vector<std::unique_ptr<ConstantBuffer>>	Params;

};

#endif