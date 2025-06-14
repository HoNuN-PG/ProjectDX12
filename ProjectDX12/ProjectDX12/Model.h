#ifndef ___MODEL_H___
#define ___MODEL_H___

#include "RenderingComponent.h"

#include <memory>
#include <DirectXMath.h>

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
	void Create(std::vector<std::shared_ptr<Material>> materials, const char* path);

};

#endif