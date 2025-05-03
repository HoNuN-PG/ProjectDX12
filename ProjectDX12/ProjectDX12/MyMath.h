#ifndef ___MYMATH_H___
#define ___MYMATH_H___

#include <DirectXMath.h>

class MyMath
{
public:
	static DirectX::XMFLOAT3 Subtraction(const DirectX::XMFLOAT3& obj1, const DirectX::XMFLOAT3& obj2)
	{
		DirectX::XMFLOAT3 ans;
		ans.x = obj1.x - obj2.x;
		ans.y = obj1.y - obj2.y;
		ans.z = obj1.z - obj2.z;
		return ans;
	}
	static DirectX::XMFLOAT3 Cross(const DirectX::XMFLOAT3& obj1, const DirectX::XMFLOAT3& obj2)
	{
		DirectX::XMFLOAT3 cross;
		cross.x = (obj1.y * obj2.z) - (obj1.z * obj2.y);
		cross.y = (obj1.z * obj2.x) - (obj1.x * obj2.z);
		cross.z = (obj1.x * obj2.y) - (obj1.y * obj2.x);
		return cross;
	}
};

#endif