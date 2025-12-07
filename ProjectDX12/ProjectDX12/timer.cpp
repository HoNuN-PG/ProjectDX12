
// System
#include "timer.h"

TimerFPS::TimerFPS():
	fix(60),
	prev(0)
{
}

bool TimerFPS::CheckGameFPS()
{
	current = timeGetTime();
	DWORD diff = current - prev;
	if (diff >= DWORD(1000) / fix)
	{
		prev = current;
		fps = diff;
		return true;
	}
	return false;
}

float TimerFPS::GetObsevationGameFPS(int avg)
{
	fpsList.push_back(fps);
	if (fpsList.size() > avg)
		fpsList.pop_front();
	DWORD ms = 0;
	for (std::list<DWORD>::iterator it = fpsList.begin(); it != fpsList.end(); ++it)
		ms += *it;
	ms /= fpsList.size();
	return 1000.0f / ms;
}

float TimerFPS::GetObservationDbFPS(int avg)
{
	DWORD item = et - st;

	List.push_back(item);
	if (List.size() > avg)
		List.pop_front();
	DWORD ms = 0;
	for (std::list<DWORD>::iterator it = List.begin(); it != List.end(); ++it)
		ms += *it;
	ms /= List.size();
	return 1000.0f / ms;
}