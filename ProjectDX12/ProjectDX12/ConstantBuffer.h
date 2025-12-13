#ifndef ___CONSTANT_BUFFER_H___
#define ___CONSTANT_BUFFER_H___

// System/Rendering/Pipeline
#include "DescriptorHeap.h"

class ConstantBuffer
{
public:
	struct Description
	{
		DescriptorHeap*	pHeap;	// 定数バッファと紐づけるディスクリプタヒープ
		UINT			size;	// 作成する定数バッファのサイズ
	};

public:
	ConstantBuffer(Description desc);
	~ConstantBuffer();

public:
	void Write(const void* data) { memcpy_s(Ptr, Size, data, Size); }

public:
	DescriptorHeap::Handle GetHandle() { return Handle; }

private:
	DescriptorHeap::Handle			Handle;		// 該当ディスクリプタのハンドル
	UINT							Size;		// 定数バッファのサイズ
	ID3D12Resource*					Resource;	// 定数バッファリソース
	D3D12_CONSTANT_BUFFER_VIEW_DESC	CBV;		// 定数バッファビュー
	void*							Ptr;		// 書き込み先アドレス

};

#endif
