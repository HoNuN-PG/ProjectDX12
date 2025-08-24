#ifndef ___VIGNETTE_H___
#define ___VIGNETTE_H___

#include "volume.h"

#include <DirectXMath.h>
#include <memory>

class ConstantBuffer;

class Vignette : public Volume
{
public:
	struct VignetteParams
	{
		DirectX::XMFLOAT4 color;
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
	VignetteParams VignetteParam;
	std::unique_ptr<ConstantBuffer> Params;

};


#endif
