#ifndef ___TIMER_H___
#define ___TIMER_H___

#include <Windows.h>
#include <list>

class TimerFPS
{
public:
	TimerFPS() {};
	~TimerFPS() {};
public:
	DWORD ObservationFPS(int avg);
public:
	DWORD st;
	DWORD et;
private:
	std::list<DWORD> List;
};

#endif