#ifndef ___MYMATH_H___
#define ___MYMATH_H___

#include <DirectXMath.h>

class DXFL
{
public:
	static DirectX::XMFLOAT3 Scale(const DirectX::XMFLOAT3& obj, const float& scale)
	{
		DirectX::XMFLOAT3 ans;
		ans.x = obj.x * scale;
		ans.y = obj.y * scale;
		ans.z = obj.z * scale;
		return ans;
	}
	static DirectX::XMFLOAT3 Add(const DirectX::XMFLOAT3& obj1, const DirectX::XMFLOAT3& obj2)
	{
		DirectX::XMFLOAT3 ans;
		ans.x = obj1.x + obj2.x;
		ans.y = obj1.y + obj2.y;
		ans.z = obj1.z + obj2.z;
		return ans;
	}
	static DirectX::XMFLOAT3 Subtraction(const DirectX::XMFLOAT3& obj1, const DirectX::XMFLOAT3& obj2)
	{
		DirectX::XMFLOAT3 ans;
		ans.x = obj1.x - obj2.x;
		ans.y = obj1.y - obj2.y;
		ans.z = obj1.z - obj2.z;
		return ans;
	}
	static float Magnitude(const DirectX::XMFLOAT3& obj)
	{
		return sqrtf(powf(obj.x, 2.0f) + powf(obj.y, 2.0f) + powf(obj.z, 2.0f));
	}
	static DirectX::XMFLOAT3 Normalize(const DirectX::XMFLOAT3& obj)
	{
		float fMagnitude = Magnitude(obj);

		// É[ÉçèúéZÇÃèúäO
		if (fMagnitude == 0.0f){ return DirectX::XMFLOAT3(); }

		DirectX::XMFLOAT3 temp;
		temp.x = obj.x / fMagnitude;
		temp.y = obj.y / fMagnitude;
		temp.z = obj.z / fMagnitude;
		return temp;
	}
	static DirectX::XMFLOAT3 Cross(const DirectX::XMFLOAT3& obj1, const DirectX::XMFLOAT3& obj2)
	{
		DirectX::XMFLOAT3 cross;
		cross.x = (obj1.y * obj2.z) - (obj1.z * obj2.y);
		cross.y = (obj1.z * obj2.x) - (obj1.x * obj2.z);
		cross.z = (obj1.x * obj2.y) - (obj1.y * obj2.x);
		return Normalize(cross);
	}
};

#endif