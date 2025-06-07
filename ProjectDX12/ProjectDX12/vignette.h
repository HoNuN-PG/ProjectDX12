#ifndef ___VIGNETTE_H___
#define ___VIGNETTE_H___

#include <DirectXMath.h>
#include <memory>

#include "volume.h"

class ConstantBuffer;

class Vignette : public Volume
{
public:
	struct VignetteParams
	{
		float start;
		float range;
		DirectX::XMFLOAT2 pad1;
	};
public:
	Vignette() {};
	~Vignette() {}
	void Init() override;
	void Draw() override;
private:
	std::shared_ptr<RenderTarget> RTV;
	VignetteParams Param;
	std::unique_ptr<ConstantBuffer> ParamBuf;

};


#endif
