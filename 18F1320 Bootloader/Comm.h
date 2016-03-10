#ifndef __COMM_H__
#define __COMM_H__

#include "stdafx.h"

//return codes used in Comm.cpp
#define COMM_OK			1  
#define COMM_ERROR		0

#define COM_TIMEOUT         1000             // ms
#define COMMAND_PAUSE       125             // Time (ms) between commands

//---------------------------------------------------------------------------
enum eParity { NONE=0, ODD=1, EVEN=2, MARK=3, SPACE=4 };
enum eStopBits { ONESB=0, ONEANDHALFSB=1, TWOSB=2 };
//---------------------------------------------------------------------------

class ComPort
{
private:
	HANDLE Handle; //defined in <windows.h>
	int DTR, RTS;
	int Baud;
	eParity Par;
	int DataBits;
	eStopBits StopBits;
public:
	ComPort();
	~ComPort();
	int Open(int port, int baud, eParity par, int databits, eStopBits stopbits);
	void Close();
	int Write(unsigned char *data, unsigned int length);
	int Read(unsigned char *data, unsigned int maxsize);
	int DataWaiting();  //returns # bytes waiting

};


#endif