#ifndef ___ROOT_SIGNATURE_H___
#define ___ROOT_SIGNATURE_H___

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
	struct Description
	{
		Parameter* pParam;
		UINT		paramNum;
	};
public:
	RootSignature(Description desc);
	~RootSignature();
	ID3D12RootSignature* Get() {
		return m_pRootSignature;
	}
	void Bind(D3D12_GPU_DESCRIPTOR_HANDLE* handle, UINT num);
private:
	ID3D12RootSignature* m_pRootSignature;
};

#endif
