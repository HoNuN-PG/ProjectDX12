#ifndef ___GLOBAL_RESOURCE_KEY_H___
#define ___GLOBAL_RESOURCE_KEY_H___

enum GlobalConstantBufferResourceKey
{
	Camera = 0,
	Light,
	ShadowMaps1,
	ShadowMaps2,
	ShadowMaps3,
	ShadowReciever,

	MAX_RESOURCE_KEY_TYPE
};

enum GlobalTextureResourceKey
{
	MainTexture = 0,
	DefferedAlbedoTexture,
	DefferedNormalTexture,

	MAX_TEXTURE_RESOURCE_KEY_TYPE
};

#endif