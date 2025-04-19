#ifndef ___CONSTANT_WVP_H___
#define ___CONSTANT_WVP_H___

#include <DirectXMath.h>

struct sConstantWVP
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 proj;
};

class cConstantWVP
{
public:
	static void* Calc3DMatrix(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot, DirectX::XMFLOAT3 scale);
	static void* Calc2DMatrix(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 rot, DirectX::XMFLOAT3 scale);

	static DirectX::XMFLOAT4X4 CalcInversVPMatric();
};

#endif