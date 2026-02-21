#ifndef ___VIGNETTE_H___
#define ___VIGNETTE_H___

#include <DirectXMath.h>
#include <memory>

#include "System/Rendering/Volume/volume.h"

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

	std::shared_ptr<RenderTarget>	pRTV;

private:

	std::unique_ptr<ConstantBuffer> Params;
	VignetteParams					VignetteParam;

};


#endif
