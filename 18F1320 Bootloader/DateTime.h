#ifndef __DATETIME_H__
#define __DATETIME_H__


#include <time.h>


class Clock
{
private:
	clock_t	time;
	bool running;

public:
	Clock();
	float Elapsed();
	void Start();
	void Stop();
};

#endif