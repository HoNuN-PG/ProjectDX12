#ifndef ___CONSTANT_LIGHTING_H___
#define ___CONSTANT_LIGHTING_H___

#include <DirectXMath.h>

struct sCamera
{
	DirectX::XMFLOAT4 pos;
};

struct sLight
{
	DirectX::XMFLOAT4 lightParam;
	DirectX::XMFLOAT4 lightColor;
};

#endif