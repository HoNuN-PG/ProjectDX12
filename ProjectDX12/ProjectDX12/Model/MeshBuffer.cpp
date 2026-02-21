
#include "Material/Materials/M_MS.h"
#include "Model/MeshBuffer.h"

MeshBuffer::MeshBuffer(Description desc)
	: 
	Desc(desc), 
	Vtx(nullptr), 
	Vbv{}, 
	Idx(nullptr), 
	Ibv{}
{
	HRESULT hr;

	// ヒープの設定
	D3D12_HEAP_PROPERTIES heapProp	= {};
	heapProp.Type						= D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty			= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference		= D3D12_MEMORY_POOL_UNKNOWN;
	heapProp.CreationNodeMask			= 1;
	heapProp.VisibleNodeMask			= 1;

	// リソースの設定
	D3D12_RESOURCE_DESC resDesc	= {};
	resDesc.Dimension				= D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width					= desc.vtxSize * desc.vtxCount;
	resDesc.Height					= 1;
	resDesc.DepthOrArraySize		= 1;
	resDesc.MipLevels				= 1;
	resDesc.Format					= DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count		= 1;
	resDesc.Layout					= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags					= D3D12_RESOURCE_FLAG_NONE;

	// 頂点バッファリソースの生成
	hr = GetDevice()->CreateCommittedResource(
		&heapProp, 
		D3D12_HEAP_FLAG_NONE, 
		&resDesc, 
		D3D12_RESOURCE_STATE_GENERIC_READ, 
		nullptr,
		IID_PPV_ARGS(Vtx.GetAddressOf())
	);
	if (FAILED(hr)) { return; }

	// 頂点バッファの初期値設定
	void* pVtxMap;
	hr = Vtx->Map(0, nullptr, (void**)&pVtxMap);
	if (SUCCEEDED(hr)) 
	{
		memcpy_s(pVtxMap, resDesc.Width, desc.pVtx, resDesc.Width);
	}
	Vtx->Unmap(0, nullptr);

	// 頂点バッファビューの設定
	Vbv.BufferLocation	= Vtx->GetGPUVirtualAddress();
	Vbv.SizeInBytes		= resDesc.Width;
	Vbv.StrideInBytes	= desc.vtxSize;
	if (desc.pIdx == nullptr) { return; }

	// リソースの設定
	UINT idxSize = 0;
	switch (desc.idxSize)
	{
	case DXGI_FORMAT_R8_UINT: idxSize = 1; break;
	case DXGI_FORMAT_R16_UINT:idxSize = 2; break;
	case DXGI_FORMAT_R32_UINT:idxSize = 4; break;
	}
	resDesc.Width = idxSize * desc.idxCount;

	// インデックスバッファのリソース生成
	hr = GetDevice()->CreateCommittedResource(
		&heapProp, 
		D3D12_HEAP_FLAG_NONE, 
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(Idx.GetAddressOf())
	);

	// インデックスバッファの初期値設定
	void* pIdxMap	= nullptr;
	hr				= Idx->Map(0, nullptr, &pIdxMap);
	if (SUCCEEDED(hr)) 
	{
		memcpy_s(pIdxMap, resDesc.Width, desc.pIdx, resDesc.Width);
	}
	Idx->Unmap(0, nullptr);

	// インデックスバッファビューの設定
	Ibv.BufferLocation	= Idx->GetGPUVirtualAddress();
	Ibv.Format			= desc.idxSize;
	Ibv.SizeInBytes		= static_cast<UINT>(resDesc.Width);
}

MeshBuffer::~MeshBuffer()
{
}

void MeshBuffer::Draw()
{
	GetCommandList()->IASetPrimitiveTopology(Desc.topology);
	GetCommandList()->IASetVertexBuffers(0, 1, &Vbv);
	if (Idx)
	{
		GetCommandList()->IASetIndexBuffer(&Ibv);
		GetCommandList()->DrawIndexedInstanced(Desc.idxCount, 1, 0, 0, 0);
	}
	else 
	{
		GetCommandList()->DrawInstanced(Desc.vtxCount, 1, 0, 0);
	}
}

MeshletBuffer::MeshletBuffer(Description desc)
	:
	Vtx(nullptr),
	pMeshlets(nullptr),
	pUniqueVertexIndices(nullptr),
	pPrimitiveIndices(nullptr)
{
	HRESULT hr;

	// ヒープの設定
	D3D12_HEAP_PROPERTIES heapProp	= {};
	heapProp.Type					= D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;
	heapProp.CreationNodeMask		= 1;
	heapProp.VisibleNodeMask		= 1;

	// リソースの設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension			= D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width				= desc.vtxSize * desc.vtxCount;
	resDesc.Height				= 1;
	resDesc.DepthOrArraySize	= 1;
	resDesc.MipLevels			= 1;
	resDesc.Format				= DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count	= 1;
	resDesc.Layout				= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags				= D3D12_RESOURCE_FLAG_NONE;

	//========================================================
	// 頂点バッファの生成
	hr = GetDevice()->CreateCommittedResource(
		&heapProp, 
		D3D12_HEAP_FLAG_NONE, 
		&resDesc, 
		D3D12_RESOURCE_STATE_GENERIC_READ, 
		nullptr,
		IID_PPV_ARGS(Vtx.GetAddressOf())
	);
	if (FAILED(hr)) { return; }

	// 頂点バッファの初期値設定
	void* pVtxMap;
	hr = Vtx->Map(0, nullptr, (void**)&pVtxMap);
	if (SUCCEEDED(hr))
	{
		memcpy_s(pVtxMap, resDesc.Width, desc.pVtx, resDesc.Width);
	}
	Vtx->Unmap(0, nullptr);

	// シェーダーリソースビューの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format						= DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension				= D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping		= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement			= 0;
	srvDesc.Buffer.NumElements			= desc.vtxCount;
	srvDesc.Buffer.StructureByteStride	= desc.vtxSize;
	hVtx = desc.pHeap->Allocate();
	GetDevice()->CreateShaderResourceView(Vtx.Get(), &srvDesc, hVtx.hCPU);

	//========================================================
	// Meshletの生成
	hr = DirectX::ComputeMeshlets(
		desc.indices.data(),
		desc.indices.size() / 3,
		desc.positions.data(),
		desc.positions.size(),
		nullptr,
		meshlets,
		uniqueVertexIndices,
		primitiveIndices
	);
	if (FAILED(hr)){ return; }

	//========================================================
	// meshlets
	// バッファの生成
	resDesc.Width = sizeof(DirectX::Meshlet) * meshlets.size();
	hr = GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(pMeshlets.GetAddressOf())
	);
	if (FAILED(hr)) { return; }

	// バッファの初期値設定
	void* pMeshletMap;
	hr = pMeshlets->Map(0, nullptr, (void**)&pMeshletMap);
	if (SUCCEEDED(hr))
	{
		memcpy_s(pMeshletMap, resDesc.Width, meshlets.data(), resDesc.Width);
	}
	pMeshlets->Unmap(0, nullptr);

	// シェーダーリソースビューの作成
	srvDesc.Buffer.NumElements = meshlets.size();
	srvDesc.Buffer.StructureByteStride = sizeof(DirectX::Meshlet);
	hMeshlets = desc.pHeap->Allocate();
	GetDevice()->CreateShaderResourceView(pMeshlets.Get(), &srvDesc, hMeshlets.hCPU);

	//========================================================
	// uniqueVertexIndices
	// バッファの生成
	resDesc.Width = sizeof(uint8_t) * uniqueVertexIndices.size();
	hr = GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(pUniqueVertexIndices.GetAddressOf())
	);
	if (FAILED(hr)) { return; }

	// バッファの初期値設定
	void* pUniqueVIBMap;
	hr = pUniqueVertexIndices->Map(0, nullptr, (void**)&pUniqueVIBMap);
	if (SUCCEEDED(hr))
	{
		memcpy_s(pUniqueVIBMap, resDesc.Width, uniqueVertexIndices.data(), resDesc.Width);
	}
	pUniqueVertexIndices->Unmap(0, nullptr);

	// シェーダーリソースビューの作成
	srvDesc.Buffer.NumElements = uniqueVertexIndices.size();
	srvDesc.Buffer.StructureByteStride = sizeof(uint8_t);
	hUniqueVertexIndices = desc.pHeap->Allocate();
	GetDevice()->CreateShaderResourceView(pUniqueVertexIndices.Get(), &srvDesc, hUniqueVertexIndices.hCPU);

	//========================================================
	// primitiveIndices
	// バッファの生成
	resDesc.Width = sizeof(DirectX::MeshletTriangle) * primitiveIndices.size();
	hr = GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(pPrimitiveIndices.GetAddressOf())
	);
	if (FAILED(hr)) { return; }

	// バッファの初期値設定
	void* pPrimitiveIndicesMap;
	hr = pPrimitiveIndices->Map(0, nullptr, (void**)&pPrimitiveIndicesMap);
	if (SUCCEEDED(hr))
	{
		memcpy_s(pPrimitiveIndicesMap, resDesc.Width, primitiveIndices.data(), resDesc.Width);
	}
	pPrimitiveIndices->Unmap(0, nullptr);

	// シェーダーリソースビューの作成
	srvDesc.Buffer.NumElements = primitiveIndices.size();
	srvDesc.Buffer.StructureByteStride = sizeof(DirectX::MeshletTriangle);
	hPrimitiveIndices = desc.pHeap->Allocate();
	GetDevice()->CreateShaderResourceView(pPrimitiveIndices.Get(), &srvDesc, hPrimitiveIndices.hCPU);

	//========================================================
	// CullDataの生成
	cullDatas.resize(meshlets.size());
	DirectX::ComputeCullData(
		desc.positions.data(), desc.positions.size(),
		meshlets.data(), meshlets.size(),
		reinterpret_cast<uint32_t*>(uniqueVertexIndices.data()), uniqueVertexIndices.size(),
		primitiveIndices.data(), primitiveIndices.size(),
		cullDatas.data()
	);

	// バッファの生成
	resDesc.Width = sizeof(DirectX::CullData) * cullDatas.size();
	hr = GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(pCullDatas.GetAddressOf())
	);
	if (FAILED(hr)) { return; }

	// バッファの初期値設定
	void* pCullDatasMap;
	hr = pCullDatas->Map(0, nullptr, (void**)&pCullDatasMap);
	if (SUCCEEDED(hr))
	{
		memcpy_s(pCullDatasMap, resDesc.Width, cullDatas.data(), resDesc.Width);
	}
	pCullDatas->Unmap(0, nullptr);

	// シェーダーリソースビューの作成
	srvDesc.Buffer.NumElements = cullDatas.size();
	srvDesc.Buffer.StructureByteStride = sizeof(DirectX::CullData);
	hCullDatas = desc.pHeap->Allocate();
	GetDevice()->CreateShaderResourceView(pCullDatas.Get(), &srvDesc, hCullDatas.hCPU);
}

MeshletBuffer::~MeshletBuffer()
{
}

void MeshletBuffer::Draw(int AmpShaderResourceStartSlot, int MeshShaderResourceStartSlot)
{
	UINT num = meshlets.size();
	if (AmpShaderResourceStartSlot != -1)
	{
		GetCommandList()->SetGraphicsRootShaderResourceView((UINT)AmpShaderResourceStartSlot, pCullDatas->GetGPUVirtualAddress());
		num = (num + LANE_COUNT - 1) / LANE_COUNT;
	}
	GetCommandList()->SetGraphicsRootShaderResourceView((UINT)MeshShaderResourceStartSlot, Vtx->GetGPUVirtualAddress());
	GetCommandList()->SetGraphicsRootShaderResourceView((UINT)MeshShaderResourceStartSlot + 1, pMeshlets->GetGPUVirtualAddress());
	GetCommandList()->SetGraphicsRootShaderResourceView((UINT)MeshShaderResourceStartSlot + 2, pUniqueVertexIndices->GetGPUVirtualAddress());
	GetCommandList()->SetGraphicsRootShaderResourceView((UINT)MeshShaderResourceStartSlot + 3, pPrimitiveIndices->GetGPUVirtualAddress());
	GetCommandList()->DispatchMesh(num, 1, 1);
}
