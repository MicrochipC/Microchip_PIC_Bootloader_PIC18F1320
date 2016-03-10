#include "DateTime.h"


Clock::Clock()
{
	Start();
}


float Clock::Elapsed()
{
	if(running)
	{
		return ((float)(clock() - time)/(float)CLOCKS_PER_SEC);
	}
	else
	{
		return ((float)time / (float)CLOCKS_PER_SEC);
	}
}

void Clock::Start()
{
	time = clock();
	running = true;
}

void Clock::Stop()
{
	time = clock() - time;	
	running = false;
}