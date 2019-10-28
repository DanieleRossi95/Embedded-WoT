#include<FishinoCoroutine.h>
#include <FishinoStl.h>
#include <Fishino.h>
#include <FishinoWebServer32.h>
#include <JSONStreamingParser.h>

#include <FishinoSquirrel.h>
#include <FishinoSquirrelServer.h>

#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

#include <vector>

#include<typeinfo>

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CONFIGURATION DATA		-- ADAPT TO YOUR NETWORK !!!
// DATI DI CONFIGURAZIONE	-- ADATTARE ALLA PROPRIA RETE WiFi !!!
#ifndef __MY_NETWORK_H

// OPERATION MODE :
// NORMAL (STATION)	-- NEEDS AN ACCESS POINT/ROUTER
// STANDALONE (AP)	-- BUILDS THE WIFI INFRASTRUCTURE ON FISHINO
// COMMENT OR UNCOMMENT FOLLOWING #define DEPENDING ON MODE YOU WANT
// MODO DI OPERAZIONE :
// NORMAL (STATION)	-- HA BISOGNO DI UNA RETE WIFI ESISTENTE A CUI CONNETTERSI
// STANDALONE (AP)	-- REALIZZA UNA RETE WIFI SUL FISHINO
// COMMENTARE O DE-COMMENTARE LA #define SEGUENTE A SECONDA DELLA MODALITÀ RICHIESTA
// #define STANDALONE_MODE

// here pur SSID of your network
// inserire qui lo SSID della rete WiFi
#define MY_SSID	""

// here put PASSWORD of your network. Use "" if none
// inserire qui la PASSWORD della rete WiFi -- Usare "" se la rete non ￨ protetta
#define MY_PASS	""

// here put required IP address of your Fishino
// comment out this line if you want AUTO IP (dhcp)
// NOTES :
//		for STATION_MODE if you use auto IP you must find it somehow !
//		for AP_MODE you MUST choose a free address range
// 
// here put required IP address (and maybe gateway and netmask!) of your Fishino
// comment out this lines if you want AUTO IP (dhcp)
// NOTE : if you use auto IP you must find it somehow !
// inserire qui l'IP desiderato ed eventualmente gateway e netmask per il fishino
// commentare le linee sotto se si vuole l'IP automatico
// nota : se si utilizza l'IP automatico, occorre un metodo per trovarlo !
#define IPADDR	192, 168,   1, 251
#define GATEWAY	192, 168,   1, 1
#define NETMASK	255, 255, 255, 0

#endif
// 
// NOTE : for prototype green version owners, set SD_CS to 3 !!!
// NOTA : per i possessori del prototipo verde di Fishino, impostare SD_CS a 3 !!!
#ifdef SDCS
const int SD_CS = SDCS;
#else
const int SD_CS = 4;
#endif
//
// END OF CONFIGURATION DATA
// FINE DATI DI CONFIGURAZIONE
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

// define ip address if required
// NOTE : if your network is not of type 255.255.255.0 or your gateway is not xx.xx.xx.1
// you should set also both netmask and gateway
#ifdef IPADDR
	IPAddress ip(IPADDR);
	#ifdef GATEWAY
		IPAddress gw(GATEWAY);
	#else
		IPAddress gw(ip[0], ip[1], ip[2], 1);
	#endif
	#ifdef NETMASK
		IPAddress nm(NETMASK);
	#else
		IPAddress nm(255, 255, 255, 0);
	#endif
#endif

// error message loop -- continue sending error message to serial port
void ErrorLoop(String msg)
{
	while(1)
	{
		Serial.println();
		Serial.println();
		Serial.println(msg);
		delay(2000);
	}
}

SdFat sd;

// read a line from sd, expecting a name and getting a pointer to value
String readParameter(SdFile &f, const char *expect)
{
	char buf[101];
	int len = f.fgets(buf, 100);
	if(len <= 0)
		ErrorLoop("Error reading 'config.txt' file!");
	if(buf[strlen(buf) - 1] == '\n')
		buf[strlen(buf) - 1 ] = 0;
	if(strncmp(buf, expect, strlen(expect)))
	{
		buf[strlen(expect)] = 0;
		ErrorLoop(String("Error in 'config.txt' file\nExpecting '") + expect + "', got '" + buf + "'");
	}
	return buf + strlen(expect);
}

// scans an IP address from a string
bool scanIP(IPAddress &ip, const char *s)
{
	for(int i = 0; i < 4; i++)
	{
		if(!*s && !isdigit(*s) && *s != '.')
			return false;
		ip[i] = 0;
		while(isdigit(*s))
			ip[i] = ip[i] * 10 + *s++ - '0';
		if(*s)
			s++;
	}
	return true;
}

// read parameters from config file and setup WiFi module
void SetupWiFi()
{
	
	// try to open "config.txt" file
	SdFile f("SQUIRREL/config.txt", O_READ);
	if(!f.isOpen())
		ErrorLoop("File 'config.txt' not found on SD card!");
	
	// read wifi parameters data
	String parm;
	bool apMode;
	parm = readParameter(f, "WIFI_MODE:");
	if(parm == "STATION")
		apMode = false;
	else if(parm == "AP")
		apMode = true;
	else
		ErrorLoop(String("Expecting 'STATION' or 'AP', got '") + parm + "'");
	String ssid = readParameter(f, "SSID:");
	String pass = readParameter(f, "PASSWORD:");

	bool dhcp = false;
	IPAddress ip, gw, nm;
	parm = readParameter(f, "IP:");
	if(parm != "AUTO")
	{
		if(!scanIP(ip, parm.c_str()))
			ErrorLoop("Error reading IP Address!");
		parm = readParameter(f, "GW:");
		if(!scanIP(gw, parm.c_str()))
			ErrorLoop("Error reading Gateway Address!");
		parm = readParameter(f, "NM:");
		if(!scanIP(nm, parm.c_str()))
			ErrorLoop("Error reading Netmask!");
	}
	else
	{
		if(apMode)
			ErrorLoop("AUTO IP can't be used in AP mode");
		dhcp = true;
	}
	
	// all ok up to here, setup WiFi network
	if(apMode)
	{
		Fishino.setMode(SOFTAP_MODE);
		Fishino.softApStopDHCPServer();
		Fishino.setApIPInfo(ip, gw, nm);
		Fishino.softApConfig(ssid.c_str(), pass.c_str(), 1, false);
		Fishino.softApStartDHCPServer();
	}
	else
	{
		Fishino.setMode(STATION_MODE);
		uint32_t tim = millis() + 5000;
		while(!Fishino.begin(ssid.c_str(), pass.c_str()))
		{
			if(millis() > tim)
				ErrorLoop("Timeout connecting to access point");
			delay(500);
		}

		if(dhcp)
			Fishino.staStartDHCP();
		else
			Fishino.config(ip, gw, nm);

		// wait for connection completion
		tim = millis() + 5000;
		while(Fishino.status() != STATION_GOT_IP)
		{
			if(millis() > tim)
				ErrorLoop("Timeout connecting to access point");
			delay(500);
		}
	}
}


void setup()
{
	// initialize serial port
	Serial.begin(115200);

	// Initialize at the highest speed supported by the board that is
	// not over 50 MHz. Try a lower speed if SPI errors occur.
	if (!sd.begin(SD_CS, SD_SCK_MHZ(12))) {
		ErrorLoop("Error initializing SD card!");
	}
	
	// reset and test wifi module
	uint32_t tim = millis() + 5000;
	while(!Fishino.reset())
	{
		if(millis() > tim)
			ErrorLoop("Error resetting WiFi module!");
		delay(500);
	}
	
	// set PHY mode 11G
	Fishino.setPhyMode(PHY_MODE_11G);

	// load WiFi parameters from SD and connect to network
	SetupWiFi();

	// start squirrel server
	SQSERVER.begin();
	
	Serial << "Initialized\n";
	
	// run last project
	if(!SQSERVER.runLastProject())
		DEBUG_ERROR("Error running last project\n");
}

void loop()
{
	SQSERVER.loop();
}
