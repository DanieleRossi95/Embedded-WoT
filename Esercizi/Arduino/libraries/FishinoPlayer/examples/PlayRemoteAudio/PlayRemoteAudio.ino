//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								PlayRemoteAudio										//
//				Created by Massimo Del Fedele, May 2017								//
//		Plays a remote audio file given the URL (hard coded here)					//
//		WARNING : VERY EXPERIMENTAL -- HANGS OFTEN AND PLAYS BAD					//
//																					//
//		Copyright (c) 2017 Massimo Del Fedele.  All rights reserved.				//
//																					//
//	Redistribution and use in source and binary forms, with or without				//
//	modification, are permitted provided that the following conditions are met:		//
//																					//
//	- Redistributions of source code must retain the above copyright notice,		//
//	  this list of conditions and the following disclaimer.							//
//	- Redistributions in binary form must reproduce the above copyright notice,		//
//	  this list of conditions and the following disclaimer in the documentation		//
//	  and/or other materials provided with the distribution.						//
//																					//	
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"		//
//	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE		//
//	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE		//
//	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE		//
//	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR				//
//	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF			//
//	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS		//
//	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN			//
//	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)			//
//	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE		//
//	POSSIBILITY OF SUCH DAMAGE.														//
//																					//
//  Version 6.0.0 -- June 2017		Initial version									//
//  Version 6.0.2 -- June 2017		Use new FishinoPlayer in stream mode			//
//																					//
//////////////////////////////////////////////////////////////////////////////////////

//Libraries inclusion
#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

#include <SPI.h>
#include <Fishino.h>
#include <FishinoHttpInStream.h>
#include <FishinoPlayer.h>

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CONFIGURATION DATA		-- ADAPT TO YOUR NETWORK !!!
// DATI DI CONFIGURAZIONE	-- ADATTARE ALLA PROPRIA RETE WiFi !!!
#ifndef __MY_NETWORK_H

// here pur SSID of your network
// inserire qui lo SSID della rete WiFi
#define MY_SSID	""

// here put PASSWORD of your network. Use "" if none
// inserire qui la PASSWORD della rete WiFi -- Usare "" se la rete non è protetta
#define MY_PASS	""

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

// un-comment this one if you want the debug output on serial port 0
// (you need an external usb/serial converter for it)
//#define DEBUG_ON_SERIAL0

//						END OF CONFIGURATION DATA                     	//
//							FINE CONFIGURAZIONE							//
//////////////////////////////////////////////////////////////////////////

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

// this one is optional, just to show wifi connection details
// questa è opzionale, solo per mostrare i dettagli della connessione wifi
void printWifiStatus()
{
	// print the SSID of the network you're attached to:
	// stampa lo SSID della rete:
	DEBUG_INFO_FUNCTION("SSID: %s\n", Fishino.SSID().c_str());

	// print your WiFi shield's IP address:
	// stampa l'indirizzo IP della rete:
	IPAddress ip = Fishino.localIP();
	DEBUG_INFO_FUNCTION("IP Address: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);

	// print the received signal strength:
	// stampa la potenza del segnale di rete:
	long rssi = Fishino.RSSI();
	DEBUG_INFO_FUNCTION("signal strength (RSSI): %ld dBm\n", rssi);
}

// fishgram picture event handler -- just display message on serial port
bool PlayRemote(const char *remotePath)
{
	DEBUG_INFO_FUNCTION("Remote audio path : '%s'\n", remotePath);
	
	FishinoHttpInStream httpStream;
	httpStream.setBuffered(2920);
	
	if(!httpStream.open(remotePath))
	{
		DEBUG_ERROR_FUNCTION("Error opening stream '%s'\n", remotePath);
		return false;
	}
	
	FishinoPlayer.volume(0.7, 0);
	if(!FishinoPlayer.play(httpStream))
	{
		DEBUG_ERROR_FUNCTION("Error playing stream\n");
		return false;
	}
	
	uint32_t tim = millis() + 500;
	DEBUG_PRINT("Playing '%s' ", remotePath);
	while(FishinoPlayer.isPlaying())
	{
		if(millis() > tim)
		{
			DEBUG_PRINT(".");
			tim = millis() + 500;
		}
	}
	DEBUG_PRINT("\n");
	return true;
}

void setup(void)
{
	// per le Fishino32, utilizza una porta alternativa di debug
#if defined(_FISHINO_PIC32_) && defined(DEBUG_ON_SERIAL0)

	// Initialize serial0 port
	// and move debug output to it
	Serial0.begin(115200);
	DEBUG_SET_STREAM(Serial0);

#else

	// Initialize serial and wait for port to open
	// Inizializza la porta seriale e ne attende l'apertura
	Serial.begin(115200);
	while (!Serial)
		;
	
#endif

	// drop a message on debug output
	DEBUG_INFO_FUNCTION("Debugger initialized\n");

	// reset and test WiFi module
	// resetta e testa il modulo WiFi
	while(!Fishino.reset())
		DEBUG_WARNING_FUNCTION("Fishino RESET FAILED, RETRYING...\n");
	DEBUG_INFO_FUNCTION("Fishino WiFi RESET OK\n");

	// go into station mode
	// imposta la modalità stazione
	Fishino.setMode(STATION_MODE);
	
	// set PHY mode to 11G
	// some new access points don't like 11B mode
	Fishino.setPhyMode(PHY_MODE_11G);

	// try forever to connect to AP
	// tenta la connessione finchè non riesce
	DEBUG_INFO_FUNCTION("Connecting to AP...");
	while(!Fishino.begin(MY_SSID, MY_PASS))
	{
		DEBUG_INFO_N(".");
		delay(2000);
	}
	DEBUG_INFO_N("OK\n");
	
	// setup IP or start DHCP client
	// imposta l'IP statico oppure avvia il client DHCP
#ifdef IPADDR
	Fishino.config(ip, gw, nm);
#else
	Fishino.staStartDHCP();
#endif

	// wait till connection is established
	DEBUG_INFO_FUNCTION("Waiting for IP...");
	while(Fishino.status() != STATION_GOT_IP)
	{
		DEBUG_INFO_N(".");
		delay(500);
	}
	DEBUG_INFO_N("OK\n");

	
	// print connection status on serial port
	// stampa lo stato della connessione sulla porta seriale
	printWifiStatus();
	
	if(!PlayRemote("http://www.fishino.it/samba.oga"))
		DEBUG_ERROR_FUNCTION("Error playing file\n");
	
}

void loop(void)
{
}
