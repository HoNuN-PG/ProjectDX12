
#include "timer.h"

DWORD TimerFPS::ObservationFPS(int avg)
{
	DWORD item = et - st;

	List.push_back(item);
	if (List.size() > avg)
		List.pop_front();
	DWORD ms = 0;
	for (std::list<DWORD>::iterator it = List.begin(); it != List.end(); ++it)
		ms += *it;
	ms /= List.size();
	return ms;
}