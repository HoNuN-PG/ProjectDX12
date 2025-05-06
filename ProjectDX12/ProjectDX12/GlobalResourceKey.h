#ifndef ___GLOBAL_RESOURCE_KEY_H___
#define ___GLOBAL_RESOURCE_KEY_H___

enum GlobalConstantBufferResourceKey
{
	ScreenWVP = 0,
	Camera,
	Light,

	MAX_RESOURCE_KEY_TYPE
};

enum GlobalTextureResourceKey
{
	AlbedoTexture = 0,
	NormalTexture,

	MAX_TEXTURE_RESOURCE_KEY_TYPE
};

#endif