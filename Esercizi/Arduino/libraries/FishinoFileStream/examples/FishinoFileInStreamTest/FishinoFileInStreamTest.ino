#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

#include <FishinoFileStream.h>

SdFat sd;
FishinoFileStream s;

#ifndef SDCS
#define SDCS 4
#endif

void halt(void)
{
	DEBUG_ERROR_N("\nSYSTEM HALTED\n");

	Serial.flush();
#ifdef _FISHINO_PIC32_
	Serial0.flush();
#endif

	while(1)
		;
}	

void setup()
{
	// this if you want some debug on secondary serial port
	// (you need an USB-Serial adapter for this!!!)
#ifdef _FISHINO_PIC32_
	Serial0.begin(115200);
	DEBUG_SET_STREAM(Serial0);
#endif

	Serial.begin(115200);
	
	DEBUG_INFO("DEBUG INITIALIZED\n");

	DEBUG_INFO("Initializing SD card...");
	if(!sd.begin(SDCS, SPI_QUARTER_SPEED)) 
	{
		DEBUG_INFO("Card failed, or not present\n");
		halt();
	}
	DEBUG_INFO("OK\n");
	
	// writing some data to sd
	DEBUG_INFO("Create file...");
	sd.mkdir("/ATestFolder");
	if(!s.open("/ATestFolder/MyTestFile.txt", O_CREAT | O_TRUNC | O_WRITE))
	{
		DEBUG_INFO("Creation failed\n");
		halt();
	}
	DEBUG_INFO("OK\n");

	DEBUG_INFO("Writing data to file...");
	const char *text = "This is a short text file\nLet's see if it's all OK\nIt should be displayed on serial port!";
	const uint32_t len = strlen(text);
	if(s.write((uint8_t const *)text, len) != len)
	{
		DEBUG_INFO("Write failed\n");
		s.close();
		halt();
	}
	DEBUG_INFO("OK\n");
	s.close();
	
	// now re-open the file in read mode
	if(!s.open("/ATestFolder/MyTestFile.txt", O_READ))
	{
		DEBUG_INFO("Open failed\n");
		halt();
	}
	DEBUG_INFO("OK\n");
	
	s.seek(0, SEEK_END);
	uint32_t siz = s.tell();
	s.seek(0, SEEK_SET);
	char *buf = (char *)malloc(siz + 1);
	s.read((uint8_t *)buf, siz);
	buf[siz] = 0;
	s.close();
	DEBUG_INFO_N("%s\n", buf);
	free(buf);

}

void loop()
{
}
