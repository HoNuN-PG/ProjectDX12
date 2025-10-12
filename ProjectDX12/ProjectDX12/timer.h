#ifndef ___TIMER_H___
#define ___TIMER_H___

#include <Windows.h>
#include <list>

class TimerFPS
{
public:
	TimerFPS();
	~TimerFPS() {};

private:
	DWORD fix; // 固定FPS
	DWORD fps; // ゲームFPS
	DWORD current;
	DWORD prev;
	std::list<DWORD> fpsList;
public:
	void SetGameFPS(DWORD fps) { fix = fps; };
	float GetGameFPS() { return 1000.0f / fps; }
	float GetObsevationGameFPS(int avg);
	bool CheckGameFPS();

public:
	DWORD st;  // FPS計測開始
	DWORD et;  // FPS計測狩猟
public:
	float GetObservationDbFPS(int avg);
private:
	std::list<DWORD> List;
};

#endif