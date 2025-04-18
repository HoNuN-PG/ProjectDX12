
static const float fInnerRadius = 1000.0f;
static const float fOuterRadius = 1015.0f;
static const float Kr = 0.0025f;
static const float Km = 0.001f;

static const float PI = 3.141592f;

static const float fSamples = 2.0f;

static const float3 three_primary_colors = float3(0.68f, 0.55f, 0.44f);

static const float fESun = 20.0f;

static const float fScaleDepth = 0.25f;

static const float g = -0.999f;
static const float g2 = g * g;

struct PS_IN
{
	float4 pos			: SV_POSITION0;
	float3 normal		: NORMAL0;
	float2 uv			: TEXCOORD0;
	float4 posWS		: TEXCOORD1;
};

cbuffer Param : register(b0)
{
	float tick;
	float3 camPos;
}

float Scale(float fcos) {
	float x = 1.0f - fcos;
	return fScaleDepth * exp(-0.00287f + x * (0.459f + x * (3.83f + x * (-6.8f + x * 5.25f))));
}

float3 IntersectionPos(float3 dir, float3 a, float radius)
{
	float b = dot(a, dir);
	float c = dot(a, a) - radius * radius;
	float d = max(b * b - c, 0.0f);

	return a + dir * (-b + sqrt(d));
}

float4 main(PS_IN input) : SV_TARGET
{
	static const float3 v3InvWaveLength = 1.0f / pow(three_primary_colors, 4.0f);

	static const float fKrESun = Kr * fESun;
	static const float fKmESun = Km * fESun;

	static const float fKr4PI = Kr * 4.0f * PI;
	static const float fKm4PI = Km * 4.0f * PI;

	static const float fScale = 1.0f / (fOuterRadius - fInnerRadius);
	static const float fScaleOverScaleDepth = fScale / fScaleDepth;

	float3 worldPos = input.posWS.xyz;
	worldPos = IntersectionPos(normalize(worldPos), float3(0.0f, fInnerRadius, 0.0f), fOuterRadius);
	float3 v3CameraPos = float3(0.0f, fInnerRadius, 0.0f);
	// 光源（太陽）を地球からの距離に対応（大体）
	float time = tick * 1.5f;
	float3 L = normalize(float3(cos(time), sin(time), 0.0f));
	float3 v3LightDir = normalize(L *  150000000.0f / (6400.0f / fInnerRadius) - worldPos);

	float3 v3Ray = worldPos - v3CameraPos;
	float fFar = length(v3Ray);
	v3Ray /= fFar;

	float3 v3Start = v3CameraPos;
	float fCameraHeight = length(v3CameraPos);
	float fStartAngle = dot(v3Ray, v3Start) / fCameraHeight;
	float fStartDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));
	float fStartOffset = fStartDepth * Scale(fStartAngle);

	float fSampleLength = fFar / fSamples;
	float fScaledLength = fSampleLength * fScale;
	float3 v3SampleRay = v3Ray * fSampleLength;
	float3 v3SamplePoint = v3Start + v3SampleRay * 0.5f;

	float3 v3FrontColor = float3(0,0,0);
	for (int n = 0; n < int(fSamples); n++) {
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
		float fLightAngle = dot(v3LightDir, v3SamplePoint) / fHeight;
		float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
		float fScatter = (fStartOffset + fDepth * (Scale(fLightAngle) - Scale(fCameraAngle)));
		float3 v3Attenuate = exp(-fScatter * (v3InvWaveLength * fKr4PI + fKm4PI));
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}

	float3 c0 = v3FrontColor * (v3InvWaveLength * fKrESun);
	float3 c1 = v3FrontColor * fKmESun;
	float3 v3Direction = v3CameraPos - worldPos;

	float fcos = dot(v3LightDir, v3Direction) / length(v3Direction);
	float fcos2 = fcos * fcos;

	float rayleighPhase = 0.75f * (1.0f + fcos2);
	float miePhase = 1.5f * ((1.0f - g2) / (2.0f + g2)) * (1.0f + fcos2) / pow(1.0f + g2 - 2.0f * g * fcos, 1.5f);

	float4 col = float4(0,0,0,1);
	 col.rgb = rayleighPhase * c0 + miePhase * c1;
	return col;
}
