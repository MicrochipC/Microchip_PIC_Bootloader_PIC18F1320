
#include "Bootloader.h"
#include <stdio.h>	//for sprintf
//#include <fstream.h>

#define PAUSE 60


Bootloader::Bootloader()
{
	Programming = 0;
	Progress = 0;
	cancelled = 0;
	major_address = 0;
	sprintf(Status, "");
}

char *Bootloader::GetStatus()
{
	return Status;
}
int Bootloader::IsProgramming()
{
	return Programming;
}

int Bootloader::GetPercent()
{
	return Progress;
}

void Bootloader::SetComPort(ComPort *port)
{
	comPort = port;
}

void Bootloader::SetFilename(char *filename)
{
	strcpy(firmware_filename, filename);
}


int Bootloader::hexchartoint(char val)
{
	int num = 0;
    switch(val)
    {
    case 'F': num = 15; break;
    case 'E': num = 14; break;
    case 'D': num = 13; break;
    case 'C': num = 12; break;
    case 'B': num = 11; break;
    case 'A': num = 10; break;
    default: if(val >= '0' && val <= '9') num = val - 0x30; break;
    };
	return num;
}

int Bootloader::SendBuffer(char *buf)
{
//	UpdateData(TRUE);
	unsigned char SendBuf[256];
	int checksum = 0;
	int SendLength = 0;


	int address = 0;
	int i, j;

	if(buf[7] == '0' && buf[8] == '4')	//Linear Address Record
	{
		i = hexchartoint(buf[9]) * 16 + hexchartoint(buf[10]);
		j = hexchartoint(buf[11]) * 16 + hexchartoint(buf[12]);
		//note that the address is stored in the hex file as low byte, high byte
		major_address = (j*256 + i) * 0x100;	
		//the configuration word is at 0x300000

		//we don't need to send anything here.
		return 1;
	}
	if(buf[7] == '0' && buf[8] == '1')	//End of File Record
	{
		//we don't need to send anything here.
		return 1;
	}
	if(buf[7] == '0' && buf[8] == '2')	//Segment Address Record
	{
		//we don't need to send anything here.
		return 1;
	}

	//sends the buffer to the serial port after parsing the data out
	if(buf[7] == '0' && buf[8] == '0')
	{
		i = hexchartoint(buf[3]) * 16 + hexchartoint(buf[4]);
		j = hexchartoint(buf[5]) * 16 + hexchartoint(buf[6]);
		address = (i*256 + j); // /2;

		SendBuf[0] = WRITE_COMMAND; //write instruction
		SendBuf[1] = (unsigned char)(((address + major_address) >> 16) & 0xff);  //upper byte
		SendBuf[2] = (unsigned char)(((address + major_address) >> 8) & 0xff);	//high bytes
		SendBuf[3] = (unsigned char)((address + major_address) & 0xff);			//low byte

		int numdata = hexchartoint(buf[1]) * 16 + hexchartoint(buf[2]);
		SendBuf[4] = (numdata & 0xff);

		int iterations = numdata/2;
		//if(numdata & 0x01)	//if it is odd, it won't divide right
		//	iterations++;
		for(int n = 0; n < iterations; n++)
		{
			int pointer = n*4;
			i = hexchartoint(buf[9 + pointer]) * 16 + hexchartoint(buf[10 + pointer]);
			SendBuf[6 + n*2] = (unsigned char)(i & 0xff);
			checksum += i;

			i = hexchartoint(buf[11 + pointer]) * 16 + hexchartoint(buf[12 + pointer]);
			SendBuf[7 + n*2] = (unsigned char)(i & 0xff);
			checksum += i;

		}
		SendBuf[5] = (unsigned char)(checksum & 0xff);
		SendLength = 6 + numdata;
	}
	//0x0EE0 * 2 = 0x1DC0
	//don't write over boot loader or eeprom
	if((address + major_address) >= (0x0EE0 * 2) && (address + major_address) <= (0x2000) )		
		return 1;


	//Sleep(PAUSE);

	//try to send SendBuf up to 5 times.
	for(int tries = 0; tries < 5; tries++)
	{

		unsigned char byte;
		while(comPort->DataWaiting() > 0)
			comPort->Read(&byte, 1);

		FILE *logfile;
		logfile = fopen("log.txt", "a");
		for(int e = 0; e < SendLength; e++)
		{
			fprintf(logfile, "%02x ", SendBuf[e]);
			fflush(logfile);
		}
		fprintf(logfile, "\n");


		for(int r = 0; r < SendLength; r++)
		{
			comPort->Write(SendBuf + r, 1);
			Sleep(10);	//this MUST be greater then 2ms to work
		}

		unsigned char response_buffer[255];
		unsigned char *response = (unsigned char *)&response_buffer;
		
		Clock clock; //start the clock
		clock.Start();
		int recieved = 0;
		int found = 0;
		int bad_data = false;

		unsigned char character;

		while((clock.Elapsed() < (PAUSE*2.0f/1000.0f)) && !found && !bad_data)
		{
			int data_waiting = comPort->DataWaiting();
			while(data_waiting > 0)
			{
				data_waiting--;

				comPort->Read((unsigned char *)&character, 1); //read a byte
				recieved++;
				*response = character;
				response++;
				if(character == DATA_BAD) //data bad byte
				{
					bad_data = true; //get out of the while loop
					break;
				}
				if(recieved == 2)
				{
					found = TRUE;
				}
			}
			Sleep(5);
		}

		fclose(logfile);
		if(bad_data)
			continue;

		if(response - ((unsigned char *)&response_buffer) < 2)
			continue;  //it timed out so try again

		if((response_buffer[0] == (unsigned char)DATA_OK) && (response_buffer[1] == (unsigned char)WRITE_OK))
		{
			return 1;
		}
	}//end for tries
	return 0;

}

//Update is designed to run in it's own thread
void Bootloader::Update()
{
	FILE *hexfile;
	if((hexfile = fopen(firmware_filename, "r")) == NULL)
	{
		Progress = 0;
		return;
	}
	if(feof(hexfile))
	{
		//there was a problem opening the file
		Progress = 0;
		return ;
	}
	fseek(hexfile, 0, SEEK_SET);
	char buff[256];
	int line = 0;

	//count the number of lines
	while(!feof(hexfile))
	{
		if( fgets( buff, 255, hexfile ) == NULL)
			break;
		line++;
	}

	int max_lines = line;
	fclose( hexfile );

	hexfile = fopen(firmware_filename, "r");

	fseek(hexfile, 0, SEEK_SET);

	int Canceled = false;
	int Waiting = true;
	Programming = false;

	unsigned char rbyte;
	line = 0;
	major_address = 0;

	Clock clock; //start the clock
	clock.Start();
	while(Waiting && !cancelled)
	{
		strcpy(Status, "Waiting for Hardware Reset.");
		//send a byte
		unsigned char data2 = IDENT;
		comPort->Write(&data2, 1);
		Sleep(5);
		if(comPort->DataWaiting() > 0)
		{
			comPort->Read(&rbyte, 1);
			if(rbyte == IDACK)
			{
				Waiting = false;
				Programming = true;
			}
		}
		Sleep(5);
		Progress = 0;

	}
	while(Programming)
	{
		Sleep(10);	//programming should take 2ms+2ms=4ms round up to 10ms
		sprintf(Status, "Writing..  %d%%", Progress);

		char buffer[256];
		unsigned char done = (unsigned char)BL_DONE;

		if( fgets( buffer, 255, hexfile ) == NULL)
		{
			Programming = false;
		}

		if(strlen(buffer) > 0)
		{
			if(buffer[0] == ':')
			{
				line++;
				Progress = (int)((float)line/(float)max_lines * 100.0f);
				if(buffer[7] == '0' && buffer[8] == '1')
				{
					comPort->Write(&done, 1);	//send the DONE character
					strcpy(Status, "Programming complete!");
					fclose( hexfile);
				}
				else
				{
					if(SendBuffer(buffer) == 0)
					{
						Programming = false;
						Canceled = true;
						fclose(hexfile);
						Progress = 0;
						strcpy(Status, "Error durring programming.");
					}
				}

			}
			if(feof(hexfile))
			{
				comPort->Write(&done, 1);	//send the DONE character
				strcpy(Status, "Programming complete!");

				Programming = false;
				Canceled = false;
				fclose( hexfile);
			}
		}
		else
		{
			//empty line
		}

	}
	Progress = 0;
	Programming = false;

}
