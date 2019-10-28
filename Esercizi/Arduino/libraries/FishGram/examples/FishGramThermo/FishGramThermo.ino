//////////////////////////////////////////////////////////////////////////////////////
//						 	FishGramThermo.ino										//
//																					//
//					a Telegram controlled Thermostat								//
//				Termostato controllabile via Telegram								//
//																					//
//		Copyright (c) 2017 Andrea S. Costa.  All rights reserved.					//
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
//  Version 5.0.1 -- May 2017   -- Initial version									//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
//________________________________________________________________________________________________
//Il relé sulla PCB è collegato al pin 2 del Fishino, ma può essere usato un pin qualsiasi in altre casistiche
#define RELAY 2

//________________________________________________________________________________________________
//Variabili globali

//Per salvare il tempo trascorso dall'accensione
uint32_t tim, tim1;

//Per salvare temperatura
int8_t temp;

//Per memorizzare stagione calda o fredda; preset falsa
boolean hot  = false;

//Le due soglie inferiore e superiore presettate
int8_t sup = 100;
int8_t inf = 0;

//Temperatura da mantenere e differenziale presettati
int8_t setTemp = 20;
uint8_t diff = 2;


//Per sapere se si è nella fase di setup della soglia superiore.
boolean set_up = false;

//Per sapere se si è nella fase di setup della soglia inferiore.
boolean set_down = false;

//Per sapere se si è nella fase di setup della temperatura da mantenere.
boolean set_temp = false;

//Per sapere se si è nella fase di setup del differenziale.
boolean set_diff = false;

//Per sapere se il messaggio che avverte se una delle due soglie di avviso è stata superata è già stato inviato.
boolean supExceededMessage = false;
boolean infExceededMessage = false;

//Il mio id
uint32_t myId = 0000;


//________________________________________________________________________________________________
//Prototipo funzione per gestire il relé
void relay();
//________________________________________________________________________________________________


// Inclusione librerie
#include <OneWire.h>
#include <FishinoDallasTemperature.h>
#include <FishinoThingSpeak.h>
#include <Fishino.h>
#include <SPI.h>
#include <FishGram.h>

//________________________________________________________________________________________________

// Configurazione WIFI

#define MY_SSID "****"
#define MY_PASS "****"

#define IPADDR  192, 168,   1, 251
#define GATEWAY 192, 168,   1, 1

#ifdef IPADDR
IPAddress ip(IPADDR);
#endif
#ifdef GATEWAY
IPAddress gw(GATEWAY);
#endif

//________________________________________________________________________________________________
// Configurazione ThingSpeak

FishinoClient client;

unsigned long myChannelNumber =  123;
const char * myReadAPIKey = "******";
const char * myWriteAPIKey = "******";

//________________________________________________________________________________________________

//Configurazione Fishgram

#define MY_TELEGRAM_TOKEN "******"

//________________________________________________________________________________________________

//Configurazione sensore di temperatura  DS18B20


//Il sensore di temperatura digitale è collegato al pin 5 della PCB
#define TEMP 5

//Inizializziamo un'istanza oneWire per communicare con qualsiasi dispositivo oneWire
OneWire oneWire(TEMP);

//Passiamo il riferimento dell'istanza oneWire all'stanza della DallasTemperature
DallasTemperature sensors(&oneWire);

//________________________________________________________________________________________________

// fishgram event handler -- just display message on serial port
bool FishGramHandler(uint32_t id, const char *firstName, const char *lastName, const char *message)
{

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	if (set_up)
	{
		/*
		 * Se vogliamo mantenere 20°C, con un differenziale
		 * impostato a 2°C, la soglia superiore del ciclo di
		 * isteresi automaticamente sarà pari a 21°C, cioè la
		 * metà del differenziale sommata alla temperatura
		 * da mantenere. Per questo sarebbe inadeguata una
		 * soglia superiore di allerta per malfunzionamenti
		 * inferiore ai 21°C.
		 */
		if ((atoi(message) >= (setTemp + diff / 2)))
		{
			sup = atoi(message);
			String ans = F("Soglia superiore impostata a ");
			ans += sup;
			ans += "°C";
			FishGram.sendMessage(id, ans.c_str());
			set_up = false;
		}
		else
		{
			String ans = F("Soglia superiore non valida, riprova");
			FishGram.sendMessage(id, ans.c_str());
		}

		return true;
	}

	if (set_down)
	{
		/*
		 * Se vogliamo mantenere 20°C, con un differenziale
		 * impostato a 2°C, la soglia inferiore del ciclo di
		 * isteresi automaticamente sarà pari a 19°C, cioè la
		 * metà del differenziale sotratta alla temperatura
		 * da mantenere. Per questo sarebbe inadeguata una
		 * soglia inferiore di allerta per malfunzionamenti
		 * superiore ai 19°C.
		 */
		if ((atoi(message) <= (setTemp - diff / 2)))
		{
			inf = atoi(message);
			String ans = F("Soglia inferiore impostata a ");
			ans += inf;
			ans += "°C";
			FishGram.sendMessage(id, ans.c_str());
			set_down = false;
		}
		else
		{
			String ans = F("Soglia inferiore non valida, riprova");
			FishGram.sendMessage(id, ans.c_str());
		}

		return true;
	}

	if (set_temp)
	{

		/*
		* Anche la temperatura da mantenere ha i suoi limiti. Essa deve mantenersi SIA inferiore o al massimo uguale alla soglia di allerta superiore
		* meno la metà del differenziale SIA superiore o uguale alla soglia di allerta inferiore più la metà del differenziale.
		* Esempio chiarificatore.
		* Se il differenziale vale 6°C e le due soglie di allerta rispettivamente 10°C e 30°C la temperatura da mantenere dovrà essere inserita all’interno
		* di questo intervallo. Ciò non è comunque sufficiente. Impostandola ad esempio pari a 28°C, la soglia superiore del ciclo di isteresi sarà pari a 31°C.
		* Come già accennato nell’esempio precedente, non avrebbe senso una soglia superiore di allerta (30°C) inferiore alla soglia superiore del ciclo di isteresi (31°C).
		* La temperatura da mantenere dovrebbe avere come massimo valore impostabile i 27°C, nei quali le due soglie superiori verrebbero a coincidere.
		* Sebbene nemmeno questa scelta abbia molto senso ci permette di trasformare la soglia di allerta superiore, o inferiore, in un avviso di attivazione o disattivazione del relé.
		*/
		if ((atoi(message) <= (sup - diff / 2)) && (atoi(message) >= (inf + diff / 2)))
		{
			setTemp = atoi(message);
			String ans = F("Temperatura da mantenere impostata a ");
			ans += setTemp;
			ans += "°C";
			FishGram.sendMessage(id, ans.c_str());
			set_temp = false;
		}
		else
		{
			String ans = F("Temperarura non valida. Assicurati che cada nell'intervallo tra le due soglie di allerta e controlla che il differenziale non sia troppo elevato.");
			FishGram.sendMessage(id, ans.c_str());
		}
		return true;
	}

	if (set_diff)
	{
		/*
		 * Riguardo al differenziale il discorso è leggermente più elaborato. I vari controlli sono stati suddivisi in due costrutti if.
		 * Il primo controlla che il differenziale sia diverso da zero, per non far coincidere le soglie del ciclo di isteresi, e che sia divisibile per due.
		 * Dopodiché controlliamo che la metà del differenziale sommato alla temperatura da mantenere sia minore o uguale alla soglia di allerta superiore e
		 * controlliamo che la metà del differenziale sottratto alla temperatura da mantenere sia superiore o uguale alla soglia di allerta inferiore.
		 * Esempio chiarificatore. Se la temperatura da mantenere vale 20°C, la soglia di allerta inferiore a 15°C e quella superiore a 25° il differenziale
		 * scelto dovrà essere minore o uguale a 10°C. Se si scegliesse 12°C le due soglie del ciclo di isteresi verrebbero posizionate rispettivamente a 14°C e a 26°C e
		 * le soglie di allerta malfunzionamento verrebbero continuamente raggiunte.
		 */
		if (!(atoi(message) % 2) && (atoi(message)))
		{
			if ((setTemp + (atoi(message) / 2) <= sup) && (setTemp - (atoi(message) / 2) >= inf))
			{
				diff = atoi(message);
				String ans = F("Il differenziale è stato impostato a ");
				ans += diff;
				ans += "°C";
				FishGram.sendMessage(id, ans.c_str());
				set_diff = false;
			}
			else
			{
				String ans = F("Differenziale non valido, riprova");
				FishGram.sendMessage(id, ans.c_str());
			}
		}
		else
		{
			String ans = F("Inserisci un differenziale divisibile per due diverso da 0.");
			FishGram.sendMessage(id, ans.c_str());
		}

		return true;
	}


	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -



	if (!strcmp(message, "/temp"))
	{
		String ans = F("La temperatura vale: ");
		ans += temp;
		ans += "°C";
		FishGram.sendMessage(id, ans.c_str());
		return true;
	}


	if (!strcmp(message, "/up"))
	{
		String ans = F("La soglia superiore vale: ");
		ans += sup;
		ans += "°C";
		FishGram.sendMessage(id, ans.c_str());
		return true;
	}

	if (!strcmp(message, "/down"))
	{
		String ans = F("La soglia inferiore vale: ");
		ans += inf;
		ans += "°C";
		FishGram.sendMessage(id, ans.c_str());
		return true;
	}

	if (!strcmp(message, "/stemp"))
	{
		String ans = F("La temperatura da mantenere vale: ");
		ans += setTemp;
		ans += "°C";
		FishGram.sendMessage(id, ans.c_str());
		return true;
	}
	if (!strcmp(message, "/diff"))
	{
		String ans = F("Il differenziale vale: ");
		ans += diff;
		ans += "°C";
		FishGram.sendMessage(id, ans.c_str());
		return true;
	}



	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -




	if (!strcmp(message, "/set_hot"))
	{
		hot = true;
		String ans = F("Impostata stagione calda");
		FishGram.sendMessage(id, ans.c_str());
		return true;
	}

	if (!strcmp(message, "/set_cold"))
	{
		hot = false;
		String ans = F("Impostata stagione fredda");
		FishGram.sendMessage(id, ans.c_str());
		return true;
	}

	if (!strcmp(message, "/set_temp"))
	{
		set_temp = true;
		set_diff = false;
		set_up = false;
		set_down = false;
		String ans = F("Imposta la temperatura da mantenere (setpoint): ");
		FishGram.sendMessage(id, ans.c_str());
		return true;
	}

	if (!strcmp(message, "/set_diff"))
	{
		set_diff = true;
		set_temp = false;
		set_up = false;
		set_down = false;
		String ans = F("Imposta il differenziale:");
		FishGram.sendMessage(id, ans.c_str());
		return true;
	}

	if (!strcmp(message, "/set_up"))
	{
		set_up = true;
		set_temp = false;
		set_diff = false;
		set_down = false;
		String ans = F("Imposta la soglia superiore:");
		FishGram.sendMessage(id, ans.c_str());
		return true;
	}

	if (!strcmp(message, "/set_down"))
	{
		set_down = true;
		set_temp = false;
		set_diff = false;
		set_up = false;
		String ans = F("Imposta la soglia inferiore:");
		FishGram.sendMessage(id, ans.c_str());
		return true;
	}


	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	return true;

}
//________________________________________________________________________________________________

void relay()
{


	//controllo innanzitutto se siamo in estate o inverno
	if (hot)
	{
		if (temp > (setTemp + diff / 2))
		{
			//In estate se supero x gradi è meglio accendere il ventilatore
			digitalWrite(RELAY, HIGH);
		}

		if (temp < (setTemp - diff / 2))
		{

			//In estate se scendo sotto y gradi è meglio spegnere il ventilatore
			digitalWrite(RELAY, LOW);
		}

	}
	else
	{

		if (temp > (setTemp + diff / 2))
		{
			//In inverno se supero x gradi è meglio spegnere la stufa
			digitalWrite(RELAY, LOW);
		}

		if (temp < (setTemp - diff / 2))
		{
			//In inverno se scendo sotto y gradi è meglio accendere la stufa
			digitalWrite(RELAY, HIGH);
		}
	}

	//Per malfunzionamenti
	if ((temp > sup) && (!supExceededMessage))
	{
		String ans = F("Attenzione! La temperatura e' maggiore della soglia superiore, possibile malfunzionamento del relé o dell'impianto di condizionamento/riscaldamento.");
		FishGram.sendMessage(myId, ans.c_str());
		supExceededMessage = true;
	}
	if ((temp < inf) && (!infExceededMessage))
	{
		String ans = F("Attenzione! La temperatura e' minore della soglia inferiore, possibile malfunzionamento del relé o dell'impianto di condizionamento/riscaldamento.");
		FishGram.sendMessage(myId, ans.c_str());
		infExceededMessage = true;
	}

	//Reset malfunzionamento
	if ((supExceededMessage) && (temp < sup))
	{
		String ans = F("Anormalita' risolta, la temperatura e' di nuovo minore della soglia superiore.");
		FishGram.sendMessage(myId, ans.c_str());
		supExceededMessage = false;
	}

	if ((infExceededMessage) && (temp > inf))
	{
		String ans = F("Anormalita' risolta, la temperatura e' di nuovo maggiore della soglia inferiore.");
		FishGram.sendMessage(myId, ans.c_str());
		infExceededMessage = false;
	}

}
//________________________________________________________________________________________________

void setup(void)
{
	pinMode(RELAY, OUTPUT);

	Serial.begin(115200);
	//Inizializza il modulo SPI
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV2);

	//Resetta e testa il modulo WiFi
	while (!Fishino.reset())
		Serial << F("Il RESET del modulo WiFi e' fallito, riprovo...\n");
	Serial << F("Il RESET del modulo WiFi e' andato a buon fine\n");

	//Imposta la modalità stazione: è quindi necessario un access point wifi al quale collegarsi
	Fishino.setMode(STATION_MODE);

	// tenta la connessione finchè non riesce
	Serial << F("Connessione all'Access Point in corso...");
	while (!Fishino.begin(MY_SSID, MY_PASS))
	{
		Serial << ".";
		delay(2000);
	}
	Serial << F("completata.\n");



	Fishino.config(ip, gw);  //Imposto l'IP e il Gateway




	//In attesa fino a che la connessione non viene stabilita
	Serial << F("Collegamento...");
	while (Fishino.status() != STATION_GOT_IP)
	{
		Serial << ".";
		delay(500);
	}
	Serial << F("completato.\n");



	//Avvio ThingSpeak
	ThingSpeak.begin(client);


	//Avvio libreria per il sensore di temperatura DS18B20
	sensors.begin();

	//Start FishGram
	FishGram.messageEvent(FishGramHandler);
	FishGram.begin(F(MY_TELEGRAM_TOKEN));

	//Abilito lista id autorizzati
	bool b = true;
	FishGram.restrict(b);
	//Aggiungo il mio ID
	FishGram.allow(myId);

	tim = millis();
	tim1 = millis();
}
//________________________________________________________________________________________________

void loop(void)
{
	FishGram.loop();

	if (millis() > tim1)
	{
		sensors.requestTemperatures();
		temp = sensors.getTempCByIndex(0);

		relay();

		tim1 = millis() + 29000;
	}

	if (millis() > tim)
	{
		ThingSpeak.setField(1, temp);
		ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

		tim = millis() + 60000;
	}
	/* Perché 29 secondi e non 30?
	 * Essendo 29 un numero primo la comunicazione con ThingSpeak
	 * e la lettura del valore della temperatura verranno eseguite
	 * in successione solamente ogni 1740 secondi, cioè 29 minuti,
	 * anziché ogni minuto se avessimo scelto 30.
	 * Questo agevola FishGram.
	 *
	 */
}
//________________________________________________________________________________________________
