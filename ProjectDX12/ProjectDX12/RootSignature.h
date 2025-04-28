#ifndef ___ROOT_SIGNATURE_H___
#define ___ROOT_SIGNATURE_H___

#include "DirectX.h"
#include <vector>

class RootSignature
{
public:
	struct ParameterTable
	{
		D3D12_DESCRIPTOR_RANGE_TYPE type;	// ルートパラメータの種類
		UINT						slot;	// 対応レジストリの番号
		UINT						num;	// ディスクリプタのレンジ数
		D3D12_SHADER_VISIBILITY		shader;	// 使用先のシェーダー
	};
	/// <summary>
	/// ルートパラメータの種類,
	/// 対応レジストリの開始番号, 
	/// ディスクリプタ数,
	/// ディスクリプタのレンジ,
	/// 使用先のシェーダー,
	/// </summary>
	struct ParameterTables
	{
		D3D12_DESCRIPTOR_RANGE_TYPE type;	// ルートパラメータの種類
		UINT						slot;	// 対応レジストリの開始番号
		UINT						num;	// ディスクリプタ数
		UINT						range;	// ディスクリプタのレンジ
		D3D12_SHADER_VISIBILITY		shader;	// 使用先のシェーダー
	};
	struct DescriptionTable
	{
		ParameterTable*	pParam;
		UINT			paramNum;
	};
	struct DescriptionTables
	{
		ParameterTables*	pParam;
		UINT				paramNum;
	};

public:
	RootSignature(DescriptionTable desc);
	RootSignature(DescriptionTables desc);
	~RootSignature();
private:
	void SetUp(std::vector<D3D12_ROOT_PARAMETER> param,UINT num);

public:
	ID3D12RootSignature* Get() {
		return m_pRootSignature;
	}
	/// <summary>
	/// １つのディスクリプタヒープ内のディスクリプタをディスクリプタテーブルと紐づけ
	/// </summary>
	/// <param name="handle"></param>
	/// <param name="num"></param>
	void Bind(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT num);

private:
	ID3D12RootSignature* m_pRootSignature;

};

#endif
