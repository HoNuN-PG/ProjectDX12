#ifndef ___RENDERING_COMPONENT_H___
#define ___RENDERING_COMPONENT_H___

#include "Component.h"
#include "MeshBuffer.h"
#include "Material.h"

class RenderingComponent : public Component
{
public:
	enum RenderingTiming
	{
		CAM = 0,
		SKYBOX,
		LIGHT,
		DEFERRED,
		FORWARD,
		CANVAS,
		AFTER_POSTPROCESS,
		BACK_BUFFER,

		MAX_TIMING
	};
	RenderingTiming GetRenderingTiming()
	{ return Timing; }
private:
	RenderingTiming Timing = RenderingTiming::FORWARD;

public:
	using Component::Component;

	virtual void Init() override {};
	virtual void Uninit() override {}
	virtual void Update() override {}
	virtual void Draw() override {}
	virtual void Rendering() = 0;

public:
	Material* GetMaterial()
	{
		return MaterialData;
	}
protected:
	std::unique_ptr<MeshBuffer> MeshData;
	Material* MaterialData;

};

#endif