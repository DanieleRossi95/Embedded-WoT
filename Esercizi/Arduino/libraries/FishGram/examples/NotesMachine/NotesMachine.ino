///////////////////////////////////////////////////////////////
//   Notes Machine - Control a thermal printer with Telegram //
///////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------------------------------
//Libraries inclusion
#include <SPI.h>
#include <Fishino.h>
#include <JSONStreamingParser.h>
#include <FishGram.h>
#include <Adafruit_Thermal.h>
#include <SoftwareSerial.h>

#include "List.h"

#include "cit.h"

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

#endif

// thermal printer connection
// collegamenti della stampante termica
#define PRINTER_TX_PIN		6
#define PRINTER_RX_PIN		5

// citations and romantic phrases stuffs
PROGMEM_STRING(CITATION_HOST, "open-electronics.org");
PROGMEM_STRING(ROMANTIC_PATH, "/apps/notes/RandomQuote.php");
PROGMEM_STRING(CITATION_PATH, "/apps/notes/RandomQuote.php");

//                    END OF CONFIGURATION DATA                      //
//                       FINE CONFIGURAZIONE                         //
///////////////////////////////////////////////////////////////////////

SoftwareSerial printerSerial(PRINTER_RX_PIN, PRINTER_TX_PIN);
Adafruit_Thermal printer(&printerSerial);

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

// the shopping list
// la lista della spesa
struct ShoppingListElement : public ListElement<ShoppingListElement>
{
	char *item;
	ShoppingListElement(const char *s) { item = strdup(s); }
	virtual ~ShoppingListElement() { if(item) free(item); }
};
List<ShoppingListElement>shoppingList;

// the command list
// la lista dei comandi
typedef bool (*CommandHandler)(uint32_t, const char *, const char *, const char *);
struct CommandElement : public ListElement<CommandElement>
{
	const __FlashStringHelper *name;
	CommandHandler handler;
	CommandElement(const __FlashStringHelper *_name, CommandHandler _handler) : name(_name), handler(_handler) {}
};
List<CommandElement> commandList;

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

// some print helpers
void printLine(void)
{
	printer.justify('C');
	for(uint8_t i = 0; i < 16; i++)
		printer.print("- ");
	printer.println();
	printer.justify('L');
}

void sendToPrint(String mes)
{
	printLine();
	printer.println(mes);
	printLine();
	printer.println();
	printer.println();
}

static String helloMsg(const char *firstName, const char *lastName)
{
	String ans = F("Ciao ");
	ans += firstName;
	ans += ", ";
	return ans;
}

// command handlers
bool Cmd_Stampa(uint32_t id, const char *firstName, const char *lastName, const char *str)
{
	// print the message
	sendToPrint(str);

	// send a confirmation back to bot
	String ans = helloMsg(firstName, lastName);
	ans += F("ho stampato il tuo messaggio '");
	ans += str;
	ans += "'";
	FishGram.sendMessage(id, ans.c_str());
	return false;
}

static bool _stricmp(const char *s1, const char *s2)
{
	while(*s1 && *s2)
	{
		if(toupper(*s1++) != toupper(*s2++))
			return true;
	}
	return false;
}

static ShoppingListElement *findElement(const char *str)
{
	ShoppingListElement *elem = shoppingList.head();
	while(elem)
	{
		if(!_stricmp(elem->item, str))
			return elem;
		elem = elem->next();
	}
	return NULL;
}

bool Cmd_AggiungiLista(uint32_t id, const char *firstName, const char *lastName, const char *str)
{
	String ans = helloMsg(firstName, lastName);
	if(findElement(str))
	{
		ans += F("'");
		ans += str;
		ans += F("' è già presente nella lista");
	}
	else
	{
		shoppingList.add(new ShoppingListElement(str));
		ans += F(" ho aggiunto '");
		ans += str;
		ans += F("' alla lista");
	}
	FishGram.sendMessage(id, ans.c_str());
	return true;
}

bool Cmd_RimuoviLista(uint32_t id, const char *firstName, const char *lastName, const char *str)
{
	String ans = helloMsg(firstName, lastName);
	ShoppingListElement *elem = findElement(str);
	if(!elem)
	{
		ans += F("'");
		ans += str;
		ans += F("'");
		ans += F(" non è nella lista");
	}
	else
	{
		shoppingList.remove(elem);
		ans += F("ho eliminato '");
		ans += str;
		ans += F("' dalla lista");
	}
	FishGram.sendMessage(id, ans.c_str());
	return true;
}

bool Cmd_MostraLista(uint32_t id, const char *firstName, const char *lastName, const char *str)
{
	String ans = helloMsg(firstName, lastName);
	if(!shoppingList.count())
		ans += F("la lista della spesa è vuota");
	else
	{
		ans += F("lista della spesa :");
		ShoppingListElement *elem = shoppingList.head();
		while(elem)
		{
			ans += F("\r\n");
			ans += elem->item;
			elem = elem->next();
		}
	}
	FishGram.sendMessage(id, ans.c_str());
	return true;
}

bool Cmd_StampaLista(uint32_t id, const char *firstName, const char *lastName, const char *str)
{
	String ans;
	if(shoppingList.count())
	{
		ans = F("Lista della spesa :");
		ShoppingListElement *elem = shoppingList.head();
		while(elem)
		{
			ans += F("\n");
			ans += elem->item;
			elem = elem->next();
		}
		sendToPrint(ans);
	}

	ans = helloMsg(firstName, lastName);
	if(!shoppingList.count())
		ans += F("la lista della spesa è vuota");
	else
	{
		ans += F("ho stampato la lista della spesa");
	}
	FishGram.sendMessage(id, ans.c_str());
	
	return true;
}

bool Cmd_SvuotaLista(uint32_t id, const char *firstName, const char *lastName, const char *str)
{
	shoppingList.clear();
	String ans = helloMsg(firstName, lastName);
	ans += F("ho cancellato la lista della spesa");
	FishGram.sendMessage(id, ans.c_str());
	return false;
}

bool Cmd_Frase(const __FlashStringHelper *path, uint32_t id, const char *firstName, const char *lastName, const char *str)
{
	Cit cit;
	cit
		.setHost(CITATION_HOST)
		.setPath(path)
	;
	if(!cit.query())
		return false;
	
	String ans = helloMsg(firstName, lastName);
	uint16_t len = ans.length() + cit.getLen();
	FishGram.startMessage(id, len);
	FishGram.contMessage(ans.c_str());
	while(cit.available())
		FishGram.contMessage(cit.read());
	return FishGram.endMessage();
}

bool Cmd_StampaFrase(const __FlashStringHelper *path, uint32_t id, const char *firstName, const char *lastName, const char *str)
{
	Cit cit;
	cit
		.setHost(CITATION_HOST)
		.setPath(path)
	;
	if(!cit.query())
		return false;
	
	printLine();
	while(cit.available())
		printer.print((char)cit.read());
	printer.println();
	printLine();
	printer.println();
	printer.println();
	cit.stop();
	if(!cit.query(cit.getId()))
		return false;
	
	String ans = helloMsg(firstName, lastName);
	ans += String(F("ho stampato la frase :\r\n"));
	uint16_t len = ans.length() + cit.getLen();
	FishGram.startMessage(id, len);
	FishGram.contMessage(ans.c_str());
	while(cit.available())
		FishGram.contMessage(cit.read());
	return FishGram.endMessage();
}

bool Cmd_Romantico(uint32_t id, const char *firstName, const char *lastName, const char *str)
{
	return Cmd_Frase(ROMANTIC_PATH, id, firstName, lastName, str);
}

bool Cmd_StampaRomantico(uint32_t id, const char *firstName, const char *lastName, const char *str)
{
	return Cmd_StampaFrase(ROMANTIC_PATH, id, firstName, lastName, str);
}

bool Cmd_Citazione(uint32_t id, const char *firstName, const char *lastName, const char *str)
{
	return Cmd_Frase(CITATION_PATH, id, firstName, lastName, str);
}

bool Cmd_StampaCitazione(uint32_t id, const char *firstName, const char *lastName, const char *str)
{
	return Cmd_StampaFrase(CITATION_PATH, id, firstName, lastName, str);
}

bool Cmd_Help(uint32_t id, const char *firstName, const char *lastName, const char *str)
{
	PROGMEM_STRING(title, "Lista dei comandi:\r\n");
	
	uint16_t len = strlen_P((const char *)title);
	CommandElement *elem = commandList.head();
	while(elem)
	{
		len += strlen_P((const char *)elem->name) + 2;
		elem = elem->next();
	}

	FishGram.startMessage(id, len);
	FishGram.contMessage(title);
	elem = commandList.head();
	while(elem)
	{
		FishGram.contMessage(elem->name);
		FishGram.contMessage("\r\n");
		elem = elem->next();
	}
	FishGram.endMessage();

	return true;
}

bool starts(const char *msg, const __FlashStringHelper *cmd)
{
	char c;
	uint16_t i = 0;
	while( (c = charAt(cmd, i++)) != 0)
		if(toupper(*msg++) != toupper(c))
			return false;
	return true;
}

// fishgram event handler -- just display message on serial port
bool FishGramHandler(uint32_t id, const char *firstName, const char *lastName, const char *message)
{
	CommandElement *elem = commandList.head();
	while(elem)
	{
		if(starts(message, elem->name))
		{
			message += strlen_P((const char *)elem->name);
			while(*message && isspace(*message))
				message++;
			return elem->handler(id, firstName, lastName, message);
		}
		elem = elem->next();
	}

	// command not found
	FishGram.sendMessage(id, F("Comando sconosciuto - inviare 'help' per lista comandi"));
	return false;
}

uint32_t tim;
void setup(void)
{
	// Initialize serial and wait for port to open
	// Inizializza la porta seriale e ne attende l'apertura
	Serial.begin(115200);
	
	// only for Leonardo needed
	// necessario solo per la Leonardo
	while (!Serial)
		;

	//  Initialize printer
	printerSerial.begin(19200);
	printer.begin();

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
		delay(500);
	}
	Serial << "OK\n";
	
	// setup IP or start DHCP client
	// imposta l'IP statico oppure avvia il client DHCP
#ifdef IPADDR
	Fishino.config(ip);
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
	
	// setup command list
	commandList.add(new CommandElement(F("aggiungi")			, Cmd_AggiungiLista));
	commandList.add(new CommandElement(F("rimuovi")				, Cmd_RimuoviLista));
	commandList.add(new CommandElement(F("mostra lista")		, Cmd_MostraLista));
	commandList.add(new CommandElement(F("stampa lista")		, Cmd_StampaLista));
	commandList.add(new CommandElement(F("svuota lista")		, Cmd_SvuotaLista));
	commandList.add(new CommandElement(F("stampa romantico")	, Cmd_StampaRomantico));
	commandList.add(new CommandElement(F("romantico")			, Cmd_Romantico));
	commandList.add(new CommandElement(F("stampa citazione")	, Cmd_StampaCitazione));
	commandList.add(new CommandElement(F("citazione")			, Cmd_Citazione));
	commandList.add(new CommandElement(F("stampa")				, Cmd_Stampa));
	commandList.add(new CommandElement(F("help")				, Cmd_Help));
	
	// start FishGram
	FishGram.messageEvent(FishGramHandler);
	FishGram.begin(F(MY_TELEGRAM_TOKEN));
	
	tim = millis();
}

void loop(void)
{
	// process FishGram data
	FishGram.loop();
	
	if(millis() > tim)
	{
		Serial << F("Free ram:") << Fishino.freeRam() << "\n";
		tim = millis() + 5000;
	}
}