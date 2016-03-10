
#include "Comm.h"
#include "cstringt.h"

ComPort::ComPort()
{
	Handle = INVALID_HANDLE_VALUE;
}

ComPort::~ComPort()
{
	Close();
}

void ComPort::Close()
{
	if(Handle != INVALID_HANDLE_VALUE)
		CloseHandle(Handle);
	Handle = INVALID_HANDLE_VALUE;
}

int ComPort::Open(int port, int baud, eParity par, int databits, eStopBits stopbits)
{
	CString comname;
	comname.Format("COM%u", port);
	//Create the port
	//COM1 COM2 LPT1 etc. are valid filenames that represent ports
	Handle = CreateFile((char *)(LPCSTR)comname, //COM1, COM2, COM3, etc.
		GENERIC_READ | GENERIC_WRITE, //read and write access
		0, //share mode
		NULL, //security attributes
		OPEN_EXISTING, //creation disposition
		0, //flags and attributes
		NULL); //handle for the templatefile
	if(Handle == INVALID_HANDLE_VALUE)  //open failed
	{
		//Show the error message here
		return COMM_ERROR;
	}

	Baud = baud;
	Par = par;
	DataBits = databits;
	StopBits = stopbits;

	RTS = RTS_CONTROL_DISABLE;
	DTR = DTR_CONTROL_DISABLE;

	DCB dcb; //use this to tell the port how to behave
	GetCommState(Handle, &dcb); //fill up the structure so we don't have to tell it everything
	dcb.BaudRate = baud;
	dcb.Parity = par;
	dcb.ByteSize = databits;
	dcb.StopBits = stopbits;
	dcb.fDtrControl = DTR;
	dcb.fRtsControl = RTS;
	//tell the port how to behave
	SetCommState(Handle, &dcb);
	//now tell it when to time-out
	COMMTIMEOUTS commto;
	commto.ReadIntervalTimeout = MAXDWORD;
	commto.ReadTotalTimeoutMultiplier = 0;
	commto.ReadTotalTimeoutConstant = 0;
	SetCommTimeouts(Handle, &commto);

	return COMM_OK;  
}

int ComPort::Write(unsigned char *data, unsigned int length)
{
	if(Handle == INVALID_HANDLE_VALUE) 
		return COMM_ERROR;

	unsigned long numwritten;
	if( WriteFile(Handle, data, length, &numwritten, NULL))
		return numwritten;  //non-zero indicates success, so I send #bytes written
	else if( GetLastError() == ERROR_IO_PENDING )
	{
		//keep trying
	}

	return COMM_ERROR;
}

int ComPort::Read(unsigned char *data, unsigned int maxsize)
{
	if(Handle == INVALID_HANDLE_VALUE) 
		return COMM_ERROR;

	unsigned long numread;
	if( ReadFile(Handle, data, maxsize, &numread, NULL))
	{
		return (int)numread;
	}
	return COMM_ERROR;
}

int ComPort::DataWaiting()
{
	if(Handle == INVALID_HANDLE_VALUE) 
		return COMM_ERROR;

	unsigned long errorFlags;
	COMSTAT comStat;

	ClearCommError( Handle, &errorFlags, &comStat );
    if(errorFlags & CE_RXPARITY) //  Bitwise and of the flags--check for parity error
	{
		return COMM_ERROR; //  Some data may have been read, but not necessarily all
	}
	return( (int) comStat.cbInQue );
}
