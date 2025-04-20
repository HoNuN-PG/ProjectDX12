
float rand(float2 vec)
{
	return frac(sin(dot(vec, float2(12.9898f, 78.233f))) * 43758.5453f);
}

float2 rand2(float2 vec)
{
	return frac(sin(float2(dot(vec, float2(127.1f, 311.7f)),dot(vec, float2(269.5f, 183.3f)))) * 43758.5453f);
}

float PerlinNoise(float2 vec, int block)
{
	int2 i_uv = floor(vec * block);		// ブロック番号
	float2 f_uv = frac(vec * block);	// ブロックごとの値

	// バリューノイズの計算に使用する4箇所の位置を定義
	int2 offset[] = {
		{0,0}, {1,0},
		{0,1}, {1,1},
	};

	// 4箇所のベクトルの値を取得
	float2 corner[] = {
		rand2(i_uv + offset[0]) * 2.0f - 1.0f,
		rand2(i_uv + offset[1]) * 2.0f - 1.0f,
		rand2(i_uv + offset[2]) * 2.0f - 1.0f,
		rand2(i_uv + offset[3]) * 2.0f - 1.0f,
	};

	// 4隅の値を取得
	float lt, rt, lb, rb;
	lt = dot(corner[0], f_uv - offset[0]);
	rt = dot(corner[1], f_uv - offset[1]);
	lb = dot(corner[2], f_uv - offset[2]);
	rb = dot(corner[3], f_uv - offset[3]);

	// 補間に使用する値を修正
	f_uv = smoothstep(0.05f, 0.95f, f_uv);

	// ノイズの値を補間
	float top = lerp(lt, rt, f_uv.x);
	float bottom = lerp(lb, rb, f_uv.x);
	float noise = lerp(top, bottom, f_uv.y);
	noise = (noise + 1) * 0.5f;

	return noise;
}

float TurbulenceNoise(float2 vec, int block)
{
	float noise = PerlinNoise(vec, block);
	return 1 - abs(noise * 2.0f - 1.0f);
}

/**
 * @param(lacunarity)ノイズの縮小率
 * @param(gain)合成の減衰率
 * @param(amplitude)初期合成率
 * @param(roll)繰り返し回数
 */
float PerlinFBM(float lacunarity, float gain, float amplitude, int roll,
	float2 vec, int block)
{
    float n = 0.0f;

    int i = 0;
    for (i = 0; i < roll; ++i)
    {
        n += PerlinNoise(vec, block) * amplitude;
        block *= lacunarity;
        amplitude *= gain;
    }

    return n;
}

/** 
 * @param(lacunarity)ノイズの縮小率
 * @param(gain)合成の減衰率
 * @param(amplitude)初期合成率
 */
float TurbulenceFBM(float lacunarity,float gain,float amplitude,int roll,
	float2 vec,int block)
{	
	float n = 0.0f;
	
	int i = 0;
	for (i = 0; i < roll; ++i)
	{
		n += TurbulenceNoise(vec, block) * amplitude;
		block *= lacunarity;
		amplitude *= gain;
	}

	return n;
}

float2 RandVoronoi(float2 uv, float offset)
{
	float2x2 m = float2x2(15.27f, 47.63f, 99.41f, 89.98f);
	uv = frac(sin(mul(uv, m)) * 46839.32f);
	return float2(sin(uv.y * offset) * 0.5f + 0.5f, cos(uv.x * offset) * 0.5 + 0.5);
}

void GetVoronoi(float2 uv, float CellDensity, float AngleOffset,
	out float DistFromCenter, out float DistFromEdge, out float2 CenterToOffset, out float cells)
{
	// セル番号
	int2 cell = floor(uv * CellDensity);
	// セルにおける自身の位置
	float2 posInCell = frac(uv * CellDensity);

	DistFromCenter = 8.0f;
	float2 closestOffset;

	int y;
	int x;
	for (y = -1; y <= 1; ++y)
	{
		for (x = -1; x <= 1; ++x)
		{
			// 確認するセル
			int2 cellToCheck = int2(x, y);
			// セルごとのランダム位置
			float2 rand = RandVoronoi(cell + cellToCheck, AngleOffset);
			// セルにおける自身にから見た各セルのランダム位置
			float2 cellOffset = float2(cellToCheck) + rand - posInCell;

			// 各オフセットまでの距離
			float distToPoint = dot(cellOffset, cellOffset);

			// 最短のオフセットの位置を保存
			if (distToPoint < DistFromCenter)
			{
				DistFromCenter = distToPoint;
				CenterToOffset = cellOffset;
				cells = rand.x;
			}
		}
	}

	// 各オフセットまでの距離の乗算を戻す
	DistFromCenter = sqrt(DistFromCenter);

	DistFromEdge = 8.0f;

	for (y = -1; y <= 1; ++y)
	{
		for (x = -1; x <= 1; ++x)
		{
			int2 cellToCheck = int2(x, y);
			float2 rand = RandVoronoi(cell + cellToCheck, AngleOffset);
			float2 cellOffset = float2(cellToCheck) + rand - posInCell;

			float distToEdge = dot(0.5f * (CenterToOffset + cellOffset), normalize(cellOffset - CenterToOffset));

			DistFromEdge = min(DistFromEdge, distToEdge);
		}
	}
}