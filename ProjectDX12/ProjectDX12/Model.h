#ifndef ___MODEL_H___
#define ___MODEL_H___

#include <DirectXMath.h>
#include <memory>

#include "RenderingComponent.h"

class Model : public RenderingComponent
{
public:
	struct ModelVertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT4 color;
	};

public:
	using RenderingComponent::RenderingComponent;

	virtual void Init() override {};
	virtual void Uninit() override {}
	virtual void Update() override {}
	virtual void Draw() override;
	virtual void Rendering() override;

public:
	virtual ~Model(){}
	void Create(Material* material, const char* path);

};

#endif