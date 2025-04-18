#ifndef ___CONSTANT_LIGHTING_H___
#define ___CONSTANT_LIGHTING_H___

#include <DirectXMath.h>

struct sLight
{
	DirectX::XMFLOAT4 dir;
};

struct sCamera
{
	DirectX::XMFLOAT4 pos;
};

#endif