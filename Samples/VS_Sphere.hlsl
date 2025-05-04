struct VS_IN 
{
	float3 pos	: POSITION0;
	float2 uv	: TEXCOORD0;
};
struct VS_OUT 
{
	float4 pos		: SV_POSITION0;
	float3 normal	: NORMAL0;
	float2 uv		: TEXCOORD0;
};
// モデルの描画で使用する各種行列を受け取る
cbuffer Matrix : register(b0) 
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
};
VS_OUT main(VS_IN vin) 
{
	// ビュー行列変換時に、ビューの移動量を除くことでカメラの回転のみが適用される
	float4x4 localView = view;
	localView._41_42_43 = 0.0f;
	VS_OUT vout;
	vout.pos = float4(vin.pos, 1.0f);
	//vout.pos = mul(vout.pos, world);		// ワールド空間で移動させず、ビュー空間でも移動させなければ、
	vout.pos = mul(vout.pos, localView);	// 必ずモデルの中心にカメラが位置するようになる
	vout.pos = mul(vout.pos, proj);
	vout.uv = vin.uv;
	vout.normal = float3(0.0f, 1.0f, 0.0f); // 受け取り先のピクセルシェーダーが要求しているので設定
	return vout;
}