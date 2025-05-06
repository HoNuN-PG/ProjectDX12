#ifndef ___CONSTANT_LIGHTING_H___
#define ___CONSTANT_LIGHTING_H___

#include <DirectXMath.h>

struct sCamera
{
	DirectX::XMFLOAT4 camParam;
};

struct sLight
{
	DirectX::XMFLOAT4 ligParam;
	DirectX::XMFLOAT4 ligColor;
};

#endif