//////////////////////////////////////////////////////////////////////
//		PhotoFish													//
//		Created by Massimo Del Fedele, May 2017		.               //
//		Listen for text, image and audio Telegram messages			//
//		and show them on a TFT display								//
//////////////////////////////////////////////////////////////////////
//Libraries inclusion
#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

#include <SPI.h>
#include <Fishino.h>
#include <FishinoHttpInStream.h>
#include <JSONStreamingParser.h>
#include <FishGram.h>

#include <FishinoGFX.h>
#include <FishinoILI9341.h>
#include <FishinoJPEGDecoder.h>

#define tft		FishinoILI9341

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

// here put your bot's Telegram token -- see pdf instructions for setup
// inserire qui la token del bot Telegram -- vedere le istruzioni nel pdf allegato
#define MY_TELEGRAM_TOKEN ""

// define this one if you want the output on Serial0 port for Fishino32
//#define USE_SERIAL0_AS_DEBUG

#endif
//                    END OF CONFIGURATION DATA                      //
//                       FINE CONFIGURAZIONE                         //
///////////////////////////////////////////////////////////////////////

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

bool consumeLine(JDR_RECT const &rec, void const *buf)
{
	if(rec.left + rec.width > 240 || rec.top + rec.height > 320)
		return true;
	FishinoILI9341Class &t = FishinoILI9341;
	t.setAddrWindow(rec.left, rec.top, rec.left + rec.width - 1, rec.top + rec.height - 1);
	uint16_t const *buf16 = (uint16_t const *)buf;
	t.pushColors(rec.width * rec.height, buf16);
	return true;
}

// this one is optional, just to show wifi connection details
// questa è opzionale, solo per mostrare i dettagli della connessione wifi
void printWifiStatus()
{
	// print the SSID of the network you're attached to:
	// stampa lo SSID della rete:
	Serial << F("SSID: ") << Fishino.SSID() << "\n";

	// print your WiFi shield's IP address:
	// stampa l'indirizzo IP della rete:
	IPAddress ip = Fishino.localIP();
	Serial << F("IP Address: ") << ip << "\n";

	// print the received signal strength:
	// stampa la potenza del segnale di rete:
	long rssi = Fishino.RSSI();
	Serial << F("signal strength (RSSI):") << rssi << F(" dBm\n");
}

// fishgram text event handler -- just display message on serial port
bool FishGramTextHandler(uint32_t id, const char *firstName, const char *lastName, const char *message)
{
	if(firstName)
		Serial << firstName << " ";
	else
		Serial << F("Nobody ");
	Serial << F("sent you ") << message << "\n";
	
	String ans = "Ciao ";
	ans += firstName;
	ans += ", ho ricevuto il tuo messaggio '";
	ans += message;
	ans += "'";
	FishGram.sendMessage(id, ans.c_str());
	
	return true;
}

// fishgram picture event handler -- show image on TFT display and send back a confirmation message
bool FishGramPictureHandler(uint32_t id, const char *firstName, const char *lastName, const char *remotePath, uint16_t w, uint16_t h, const char *caption)
{
/*
	if(firstName)
		Serial << firstName << " ";
	else
		Serial << F("Nobody ");
	Serial << F("sent you a picture");
	if(caption && *caption)
		Serial << F(" with title '") << caption << "'\n";
	else
		Serial << "\n";
	Serial << F("File path is '") << file << "'\n";
	Serial << F("File size is ") << size << "\n";
	Serial << F("Image size is ") << w << " x " << h << "\n";
*/
	DEBUG_INFO("Entering FishGramPictureHandler\n");
	
	// create and open the remote stream
	FishinoHttpInStream stream;
	
// enable buffering ONLY for 32 bit Fishino boards
#ifdef _FISHINO_PIC32_
	stream.setBuffered(2000);
#endif

	if(!stream.open(remotePath))
	{
		DEBUG_ERROR("Error opening stream '%s'\n", remotePath);
		return false;
	}
	
	FishinoJPEGDecoder decoder;
	decoder.setConsumer(consumeLine, 240, 320, JDR_COLOR565);
	decoder.setAutoCenter(JDR_CENTER_BOTH);
	
	DEBUG_INFO("Setting provider\n");
	if(decoder.setProvider(stream) != JDR_OK)
	{
		DEBUG_ERROR("Stream error\n");
		return false;
	}
	DEBUG_INFO("Decoding\n");
	decoder
		.setAutoScale(true)
		.setRotation(JDR_ROT_AUTOCW)
	;
	tft.fillScreen(ILI9341_BLACK);
	decoder.decode();
	
	// remember to close the stream upon decoding
	// as FishGram needs to open a new https connection
	// and only ONE https connection can be done at once
	stream.close();	
	
	String ans = "Ciao ";
	ans += firstName;
	ans += ", ho ricevuto la tua foto";
	FishGram.sendMessage(id, ans.c_str());
	
	return true;
}

// fishgram picture event handler -- just display message on serial port
bool FishGramAudioHandler(uint32_t id, const char *firstName, const char *lastName, const char *remotePath)
{
	if(firstName)
		Serial << firstName << " ";
	else
		Serial << F("Nobody ");
	Serial << F("sent you a voice message\n");
/*
	Serial << F("File path is '") << file << "'\n";
	Serial << F("File size is ") << size << "\n";
*/
	
	String ans = "Ciao ";
	ans += firstName;
	ans += ", ho ricevuto il tuo messaggio vocale";
	FishGram.sendMessage(id, ans.c_str());
	
	return true;
}

uint32_t tim;
void setup(void)
{
	// per le Fishino32, utilizza una porta alternativa di debug
#if defined(_FISHINO_PIC32_) && defined(USE_SERIAL0_AS_DEBUG)
	Serial0.begin(115200);
	DEBUG_SET_STREAM(Serial0);
#else
	// Initialize serial and wait for port to open
	// Inizializza la porta seriale e ne attende l'apertura
	Serial.begin(115200);
	
	// only for Leonardo and Fishino32 boards
	// if you want to wait up to serial monitor is opened
	// necessario solo per la Leonardo e le Fishino32
	// se si vuole attendere l'apertura del monitor seriale
	while (!Serial)
		;
#endif


	// initialize TFT display
	// inizializza il display TFT
	tft.begin();

	// reset and test WiFi module
	// resetta e testa il modulo WiFi
	while(!Fishino.reset())
		Serial << F("Fishino RESET FAILED, RETRYING...\n");
	Serial << F("Fishino WiFi RESET OK\n");

	// go into station mode
	// imposta la modalità stazione
	Fishino.setMode(STATION_MODE);
	
	// set PHY mode to 11G
	// some new access points don't like 11B mode
	Fishino.setPhyMode(PHY_MODE_11G);

	// try forever to connect to AP
	// tenta la connessione finchè non riesce
	Serial << F("Connecting to AP...");
	while(!Fishino.begin(MY_SSID, MY_PASS))
	{
		Serial << ".";
		delay(2000);
	}
	Serial << "OK\n";
	
	// setup IP or start DHCP client
	// imposta l'IP statico oppure avvia il client DHCP
#ifdef IPADDR
	Fishino.config(ip, gw, nm);
#else
	Fishino.staStartDHCP();
#endif

	// wait till connection is established
	Serial << F("Waiting for IP...");
	while(Fishino.status() != STATION_GOT_IP)
	{
		Serial << ".";
		delay(500);
	}
	Serial << F("OK\n");

	
	// print connection status on serial port
	// stampa lo stato della connessione sulla porta seriale
	printWifiStatus();
	
	// start FishGram
	FishGram.messageEvent(FishGramTextHandler);
	FishGram.pictureEvent(FishGramPictureHandler, 240, 320);
	FishGram.audioEvent(FishGramAudioHandler);
	FishGram.begin(F(MY_TELEGRAM_TOKEN));
	
	tim = millis();
}

void loop(void)
{
	// process FishGram data
	FishGram.loop();
	
	if(millis() > tim)
	{
		DEBUG_INFO("Free RAM : %u\n", (unsigned)Fishino.freeRam());
		tim = millis() + 3000;
	}
}