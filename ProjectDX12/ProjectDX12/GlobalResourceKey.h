#ifndef ___GLOBAL_RESOURCE_KEY_H___
#define ___GLOBAL_RESOURCE_KEY_H___

enum GlobalConstantBufferResourceKey
{
	Camera = 0,
	Light,
	ShadowMaps,
	ShadowReceive,

	MAX_RESOURCE_KEY_TYPE
};

enum GlobalTextureResourceKey
{
	MainTexture = 0,
	DepthTexture,
	NormalTexture,
	DefferedAlbedoTexture,
	DefferedNormalTexture,

	MAX_TEXTURE_RESOURCE_KEY_TYPE
};

#endif