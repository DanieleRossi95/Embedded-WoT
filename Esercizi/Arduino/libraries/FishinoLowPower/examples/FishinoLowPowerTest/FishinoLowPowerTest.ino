#include <FishinoLowPower.h>
#include <Fishino.h>
#include <SPI.h>

#ifdef _FISHINO_PIC32_
#include <FishinoRTC.h>
#endif

#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CONFIGURATION DATA		-- ADAPT TO YOUR NETWORK !!!
// DATI DI CONFIGURAZIONE	-- ADATTARE ALLA PROPRIA RETE WiFi !!!
#ifndef __MY_NETWORK_H

// here pur SSID of your network
// inserire qui lo SSID della rete WiFi
#define MY_SSID	""

// here put PASSWORD of your network. Use "" if none
// inserire qui la PASSWORD della rete WiFi -- Usare "" se la rete non ￨ protetta
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

// Initialize the Fishino client library
FishinoClient client;

// server address:
char server[] = "www.fishino.it";

// delay between updates, in milliseconds
// ritardo tra gli aggiornamenti, in millisecondi
const unsigned long postingInterval = 15L * 1000L;

// this method makes a HTTP connection to the server:
void httpRequest()
{
	// close any connection before send a new request.
	// This will free the socket on the WiFi module
	// chiude ogni ventuale connessione prima di inviare una nuova richiesta
	// quest libera il socket sull modulo WiFi
	client.stop();

	// if there's a successful connection:
	// se la connessione è riuscita:
	if (client.connect(server, 80))
	{
		DEBUG_INFO("connecting...\n");
		
		// send the HTTP PUT request:
		// invia la richiesta HTTP:
		client << F("GET / HTTP/1.1\r\n");
		client << F("Host: www.fishino.it\r\n");
		client << F("User-Agent: FishinoWiFi/1.1\r\n");
		client << F("Connection: close\r\n");
		client.println();
	}
	else
	{
		// if you couldn't make a connection:
		// se la connessione non è riuscita:
		DEBUG_INFO("connection failed\n");
	}
}

void printWifiStatus()
{
	// print the SSID of the network you're attached to:
	// stampa lo SSID della rete:
	Serial.print("SSID: ");
	Serial.println(Fishino.SSID());

	// print your WiFi shield's IP address:
	// stampa l'indirizzo IP della rete:
	IPAddress ip = Fishino.localIP();
	DEBUG_INFO("IP Address: %d:%d:%d:%d\n", ip[0], ip[1], ip[2], ip[3]);

	// print the received signal strength:
	// stampa la potenza del segnale di rete:
	DEBUG_INFO("signal strength (RSSI): %ld dBm\n", Fishino.RSSI());
}


void wifiConnect(void)
{
	Fishino.setPhyMode(PHY_MODE_11N);

	// go into station mode
	// imposta la modalità stazione
	Fishino.setMode(STATION_MODE);

	// try forever to connect to AP
	// tenta la connessione finchè non riesce
	DEBUG_INFO("Connecting to AP...");
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
	DEBUG_INFO("Waiting for IP...");
	while(Fishino.status() != STATION_GOT_IP)
	{
		DEBUG_INFO_N(".");
		delay(500);
	}
	DEBUG_INFO_N("OK\n");
	
	// print connection status on serial port
	// stampa lo stato della connessione sulla porta seriale
	printWifiStatus();
}

void setup()
{
#ifdef _FISHINO_PIC32_
	// Initialize serial and wait for port to open
	Serial0.begin(115200);
	DEBUG_SET_STREAM(Serial0);
#else
	Serial.begin(115200);
#endif

	
#ifdef _FISHINO_PIC32_
	if(!RTC.isrunning())
		RTC.adjust(DateTime(__DATE__, __TIME__));
#endif
	
	// reset and test WiFi module
	// resetta e testa il modulo WiFi
	while(!Fishino.reset())
		DEBUG_INFO("Fishino RESET FAILED, RETRYING...\n");
	DEBUG_INFO("Fishino WiFi RESET OK\n");
	
	// enable serial port wakeup
//	FishinoLowPower::enableSerialWakeup();

#ifdef _FISHINO_MEGA_
	FishinoLowPower.enableInterruptPin(A8);
#elif defined(_FISHINO_UNO_) || defined(_FISHINO_GUPPY_)
	FishinoLowPower.enableInterruptPin(A0);
#elif defined(_FISHINO_PIC32_)
	FishinoLowPower.enableInterruptPin(2);
	FishinoLowPower.enableInterruptPinPullup(2);
	FishinoLowPower.enableSerial0Wakeup();
#endif

	// for FISHINO32 use full power off
	// REMEMBER, you MUST have a battery connected for it...
	// otherwise, just comment following line
#ifdef _FISHINO_PIC32_
	FishinoLowPower.enableFullPowerOff();
#endif

	DEBUG_INFO("Initialized... wait 5 seconds before shutdown\n");
	delay(5000);
	DEBUG_INFO("going to loop\n");

}

void loop()
{
	// go in sleep mode for 10 seconds
	// (or less if external interrupt wakes us up)
	FishinoLowPower.deepSleep(0);

	DEBUG_INFO("Connecting to access point\n");
	wifiConnect();
		
	DEBUG_INFO("Sending request\n");
	httpRequest();

	DEBUG_INFO("Waiting for data\n");
	uint32_t tim = millis() + 500;
	while(!client.available() && millis() < tim)
		;

	DEBUG_INFO("Reading and printing data\n");
	while (client.available())
	{
		char c = client.read();
		DEBUG_INFO_N("%c", c);
	}
	DEBUG_INFO_N("\n");
	DEBUG_INFO("Stopping client\n");
	client.stop();
}
