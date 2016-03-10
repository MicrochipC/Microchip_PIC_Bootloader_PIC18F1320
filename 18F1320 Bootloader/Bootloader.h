
#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

//these must be implemented by the user!!!
#include "Comm.h"
#include "DateTime.h"

#define DATA_OK		0xE7
#define DATA_BAD	0xE8

#define WRITE_COMMAND	0xE3
#define WRITE_OK	0xE4
#define WRITE_BAD	0xE5

#define IDENT       0xEA
#define IDACK       0xEB

#define BL_DONE        0xED


class Bootloader
{
private:
	ComPort *comPort;
	char firmware_filename[255];

	int hexchartoint(char val);
	int SendBuffer(char *buf);

	int major_address;

	int Progress;
	int Programming;
	int cancelled;
	char Status[255];

public:
	Bootloader();
	int GetPercent();
	char *GetStatus();
	int IsProgramming();

	void SetComPort(ComPort *port);
	void SetFilename(char *filename);
	void Update();

};

#endif