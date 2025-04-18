#include "Game.h"
#include "Input.h"
#include <algorithm>
#include "SceneFractal.h"
#include "SceneLSystem.h"
#include "SceneLSystemNEO.h"
#include "SceneProcedural.h"
#include "SceneField.h"
#include "Scene.h"

enum Scenes {
	GAME = 0,
	FRACTAL,
	LSYSTEM,
	LSYSTEMNEO,
	PROCEDURAL,
	FIELD,
	MAX
};
int g_scene = FIELD;

SceneFractal g_fractal;
SceneLSystem g_lsystem;
SceneLSystemNEO g_lsystemNEO;
SceneProcedural g_procedural;
SceneField g_field;

HRESULT InitScene()
{
	HRESULT hr = E_FAIL;
	switch (g_scene) {
	default:break;
	case GAME: hr = cGame::InitGame(); break;
	case FRACTAL: hr = g_fractal.Init(); break;
	case LSYSTEM: hr = g_lsystem.Init(); break;
	case LSYSTEMNEO: hr = g_lsystemNEO.Init(); break;
	case PROCEDURAL: hr = g_procedural.Init(); break;
	case FIELD: hr = g_field.Init(); break;
	}
	return hr;
}
void UninitScene()
{
	switch (g_scene) {
	default:break;
	case GAME: cGame::UninitGame(); break;
	case FRACTAL: g_fractal.Uninit(); break;
	case LSYSTEM: g_lsystem.Uninit(); break;
	case LSYSTEMNEO: g_lsystemNEO.Uninit(); break;
	case PROCEDURAL: g_procedural.Uninit(); break;
	case FIELD: g_field.Uninit(); break;
	}
}
void UpdateScene()
{
	// キー入力
	// シーンインデックスの扱い方を変更
	int preScene = g_scene;
	if (Input::GetKeyTrigger(VK_LEFT))
		g_scene = max(GAME, g_scene--);
	if (Input::GetKeyTrigger(VK_RIGHT))
		g_scene = min(g_scene++, MAX - 1);
	if (g_scene != preScene) {
		int newScene = g_scene;
		g_scene = preScene;
		UninitScene();
		g_scene = newScene;
		InitScene();
	}

	switch (g_scene) {
	default:break;
	case GAME: cGame::UpdateGame(); break;
	case LSYSTEM: g_lsystem.Update(); break;
	case LSYSTEMNEO: g_lsystemNEO.Update(); break;
	case PROCEDURAL: g_procedural.Update(); break;
	case FIELD: g_field.Update(); break;
	}
}
void DrawScene()
{
	// 描画
	switch (g_scene) {
	default:break;
	case GAME: cGame::DrawGame(); break;
	case FRACTAL: g_fractal.Draw(); break;
	case LSYSTEM: g_lsystem.Draw(); break;
	case LSYSTEMNEO: g_lsystemNEO.Draw(); break;
	case PROCEDURAL: g_procedural.Draw(); break;
	case FIELD: g_field.Draw(); break;
	}
}