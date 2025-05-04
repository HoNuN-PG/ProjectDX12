struct PS_IN {
	float4 pos		: SV_POSITION0;
	float3 normal	: NORMAL0;
	float2 uv		: TEXCOORD0;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

// ガウス分布関数
// sigmaの値に応じてぼかし具合が変化
float gauss(float x, float sigma) {
	float PI = 3.141592f;
	return 1.0f / (sqrt(2.0f * PI) * sigma) * exp(-(x * x) / (2.0f * sigma * sigma));
}

float4 main(PS_IN pin) : SV_TARGET{
	// 定数定義
	float2 texSize = 1.0f / float2(1280.0f, 720.0f);	// シェーダーに渡されるテクスチャのサイズ
	float sigma = 20.0f;								// ぼかし強度
	int Kernel = 7;										// ぼかしをかける範囲(奇数
	int hKernel = Kernel / 2;							// 半分のサイズ
	// 変数宣言
	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float sum = 0;
	float weight;
	[unroll(7)]
	for (int j = -hKernel; j <= hKernel; ++j) {
		[unroll(7)]
		for (int i = -hKernel; i <= hKernel; ++i) {
			float2 uv = pin.uv;
			uv.x += i * texSize.x;
			uv.y += j * texSize.y;
			weight = gauss(i + j, sigma);
			color += tex.Sample(samp, uv) * weight;
			sum += weight;
		}
	}
	color *= 1.0f / sum;
	return color;
}