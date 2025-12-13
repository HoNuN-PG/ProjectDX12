#ifndef ___ROOT_SIGNATURE_H___
#define ___ROOT_SIGNATURE_H___

#include <vector>

// System
#include "DirectX.h"

class RootSignature
{
public:
	struct Parameter
	{
		D3D12_DESCRIPTOR_RANGE_TYPE type;	// ルートパラメータの種類
		UINT						slot;	// 対応レジストリの番号
		UINT						num;	// ディスクリプタのレンジ数
		D3D12_SHADER_VISIBILITY		shader;	// 使用先のシェーダー
	};
	struct Parameters
	{
		std::vector<D3D12_DESCRIPTOR_RANGE_TYPE>	type;	// レンジごとのルートパラメータの種類
		std::vector<UINT>							slot;	// レンジごとの対応レジストリの開始番号
		std::vector<UINT>							num;	// レンジごとのディスクリプタ数
		UINT										range;	// ディスクリプタのレンジ
		D3D12_SHADER_VISIBILITY						shader;	// 使用先のシェーダー
	};

	struct Description
	{
		Parameter*					pParam;
		UINT						paramNum;
		D3D12_TEXTURE_ADDRESS_MODE	sample;
		D3D12_FILTER				filter;

		Description() :
			pParam(nullptr),
			paramNum(0),
			sample(D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
			filter(D3D12_FILTER_MIN_MAG_MIP_LINEAR)
		{}
	};
	struct Descriptions
	{
		Parameters*					pParam;
		UINT						paramNum;
		D3D12_TEXTURE_ADDRESS_MODE	sample;
		D3D12_FILTER				filter;

		Descriptions() :
			pParam(nullptr),
			paramNum(0),
			sample(D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
			filter(D3D12_FILTER_MIN_MAG_MIP_POINT)
		{
		}
	};

public:
	RootSignature(Description desc);
	RootSignature(Descriptions desc);
	~RootSignature();

private:
	/// <summary>
	/// セットアップ
	/// </summary>
	/// <param name="param"></param>
	/// <param name="sample"></param>
	/// <param name="filter"></param>
	/// <param name="num"></param>
	void SetUp(std::vector<D3D12_ROOT_PARAMETER> param, D3D12_TEXTURE_ADDRESS_MODE sample, D3D12_FILTER filter, UINT num);

public:
	/// <summary>
	/// １つのディスクリプタヒープ内のディスクリプタをディスクリプタテーブルと紐づけ
	/// </summary>
	/// <param name="handle"></param>
	/// <param name="num"></param>
	void Bind(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT num);

public:
	ID3D12RootSignature* Get() { return RootSignatureData.Get(); }

private:
	ComPtr<ID3D12RootSignature> RootSignatureData;

};

#endif
