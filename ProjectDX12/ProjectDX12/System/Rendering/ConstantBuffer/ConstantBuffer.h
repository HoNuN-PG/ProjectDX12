#ifndef ___CONSTANT_BUFFER_H___
#define ___CONSTANT_BUFFER_H___

#include "System/Rendering/Pipeline/DescriptorHeap.h"

class ConstantBuffer
{
public:

	struct Description
	{
		DescriptorHeap*	pHeap;	// ディスクリプタヒープ
		UINT			size;	// 作成する定数バッファのサイズ
	};

public:

	ConstantBuffer(Description desc);
	~ConstantBuffer();

public:

	void Write(const void* data) { memcpy_s(ptr, Size, data, Size); }

public:

	DescriptorHeap::Handle GetHandle() { return Handle; }

private:
	DescriptorHeap::Handle			Handle;		// ディスクリプタのハンドル
	UINT							Size;		// 定数バッファのサイズ
	ComPtr<ID3D12Resource>			pResource;	// 定数バッファリソース
	D3D12_CONSTANT_BUFFER_VIEW_DESC	CBV;		// 定数バッファビュー
	void*							ptr;		// 書き込み先アドレス

};

#endif
