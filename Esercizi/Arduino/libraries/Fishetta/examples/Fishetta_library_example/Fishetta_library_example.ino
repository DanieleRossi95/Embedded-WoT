///////////////////////////////////////////////////////////////////
//   Fishetta library example                                    //
//   Created by Emanuele Signoretta, August 9, 2017.             //
//   Send push notification by using the Pushetta webapp.        //
///////////////////////////////////////////////////////////////////

#include <SPI.h>
#include <Fishino.h> 
#include <Fishetta.h>


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CONFIGURATION DATA    -- ADAPT TO YOUR NETWORK !!!
// DATI DI CONFIGURAZIONE -- ADATTARE ALLA PROPRIA RETE WiFi !!!
#ifndef __MY_NETWORK_H

// here pur SSID of your network
// inserire qui lo SSID della rete WiFi
#define MY_SSID ""

// here put PASSWORD of your network. Use "" if none
// inserire qui la PASSWORD della rete WiFi -- Usare "" se la rete non è protetta
#define MY_PASS ""

// here put required IP address (and maybe gateway and netmask!) of your Fishino
// comment out this lines if you want AUTO IP (dhcp)
// NOTE : if you use auto IP you must find it somehow !
// inserire qui l'IP desiderato ed eventualmente gateway e netmask per il fishino
// commentare le linee sotto se si vuole l'IP automatico
// nota : se si utilizza l'IP automatico, occorre un metodo per trovarlo !
#define IPADDR  192, 168,   1, 132
#define GATEWAY 192, 168,   1, 1
#define NETMASK 255, 255, 255, 0


const char * APIKEY  = "" ; // Put here your API key 
const char * CHANNEL  = ""; // Put here your channel name

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

// this one is optional, just to show wifi connection details
// questa è opzionale, solo per mostrare i dettagli della connessione wifi
void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  // stampa lo SSID della rete:
  Serial.print("SSID: ");
  Serial.println(Fishino.SSID());

  // print your WiFi shield's IP address:
  // stampa l'indirizzo IP della rete:
  IPAddress ip = Fishino.localIP();
  Serial << F("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  // stampa la potenza del segnale di rete:
  long rssi = Fishino.RSSI();
  Serial << F("signal strength (RSSI):");
  Serial.print(rssi);
  Serial << F(" dBm\n");
}

Fishetta push(CHANNEL,APIKEY); 

void setup()
{
// Initialize serial and wait for port to open
  // Inizializza la porta seriale e ne attende l'apertura
  Serial.begin(115200);
  
  // only for Leonardo needed
  // necessario solo per la Leonardo
  while (!Serial)
    ;

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
}

void loop()
{

if (push.sendPushNotification("Fishetta library test!")){
  String serial_print = "Sent push notification at ";
  serial_print += CHANNEL;
  serial_print += " channel !";
  Serial.println(serial_print.c_str());} 

delay(10000);

}
