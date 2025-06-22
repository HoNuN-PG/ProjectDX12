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
	Material();
	virtual ~Material() {};

public:
	/// <summary>
	/// 初期化
	/// </summary>
	static void Initialize(
		std::shared_ptr<Material> material,
		DescriptorHeap* heap,
		RenderingTiming timing = RenderingTiming::Forward);
protected:
	virtual void Initialize(
		DescriptorHeap* heap,
		RenderingTiming timing = RenderingTiming::Forward) = 0;

protected:
	void Create
	(
		DescriptorHeap* heap,
		RootSignature::DescriptionTable,
		Pipeline::Description pipeline
	);
	void BindBase(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT handleNum);

public:
	void SetOwner(std::weak_ptr<class GameObject> owner) { Owner = owner; }
	void AddMaterialInstance();
	/// <summary>
	/// マテリアル設定
	/// 設定後MaterialInstanceIdxが更新される
	/// </summary>
	virtual void Bind() = 0;
	void AddTexture(const char* path);
	/// <summary>
	/// MaterialInstanceIdxにWVPを書き込み
	/// WVPの書き込み直後にBindすることでMaterialInstanceIdxに書き込まれたWVPで設定を行う
	/// </summary>
	/// <param name="data"></param>
	void WriteWVP(void* data);
	void WriteParams(void* data, UINT idx);
	void WriteParams(UINT range, UINT startIdx, D3D12_CPU_DESCRIPTOR_HANDLE startHandle, D3D12_DESCRIPTOR_HEAP_TYPE type);

	/// <summary>
	/// 描画終了時に呼び出し
	/// </summary>
	void EndRendering();

public:
	RenderingTiming GetRenderTiming() { return Timing; };
protected:
	RenderingTiming									Timing;
	DescriptorHeap*									Heap;
	std::unique_ptr<RootSignature>					RootSignatureData;
	std::unique_ptr<Pipeline>						PipelineData;
	std::vector<std::unique_ptr<Texture>>			Textures;
	unsigned int									MaterialInstanceCount;
	unsigned int									MaterialInstanceIdx;
	std::vector<std::unique_ptr<ConstantBuffer>>	WVP;
	std::vector<std::unique_ptr<ConstantBuffer>>	Params;
protected:
	std::weak_ptr<class GameObject>					Owner;

};

#endif