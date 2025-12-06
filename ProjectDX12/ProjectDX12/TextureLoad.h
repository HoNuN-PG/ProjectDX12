#ifndef __TEXTURE_LOAD_H__
#define __TEXTURE_LOAD_H__

#include <DirectXTex.h>

inline HRESULT LoadTexture(const char* fileName, DirectX::TexMetadata* pInfo, DirectX::ScratchImage* pImage)
{
	HRESULT hr;

	wchar_t wPath[MAX_PATH];
	size_t wLen = 0;
	MultiByteToWideChar(0, 0, fileName, -1, wPath, MAX_PATH);

	if (strstr(fileName, ".hdr")) 
	{
		hr = DirectX::LoadFromHDRFile(wPath, pInfo, *pImage);
	}
	else
	{
		hr = DirectX::LoadFromWICFile(wPath, DirectX::WIC_FLAGS_NONE, pInfo, *pImage);
	}

	return hr;
}

#endif // __TEXTURE_LOAD_H__
