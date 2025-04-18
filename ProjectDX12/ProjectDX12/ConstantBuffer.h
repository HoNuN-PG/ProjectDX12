#ifndef ___CONSTANT_BUFFER_H___
#define ___CONSTANT_BUFFER_H___

#include "DescriptorHeap.h"

class ConstantBuffer
{
public:
	struct Description
	{
		DescriptorHeap*	pHeap;	// 定数バッファと紐づけるディスクリプタヒープ
		UINT				size;	// 作成する定数バッファのサイズ
	};
public:
	ConstantBuffer(Description desc);
	~ConstantBuffer();
	void Write(const void* data){
		memcpy_s(m_pPtr, m_size, data, m_size);
	}
	DescriptorHeap::sHandle GetHandle() {
		return m_handle; 
	}
private:
	DescriptorHeap::sHandle		m_handle;	// 該当ディスクリプタのハンドル
	UINT							m_size;		// 定数バッファのサイズ
	ID3D12Resource*					m_pBuf;		// 定数バッファリソース
	D3D12_CONSTANT_BUFFER_VIEW_DESC	m_cbv;		// 定数バッファビュー
	void*							m_pPtr;		// 書き込み先アドレス
};

#endif
