// Progetto : JSonStreamingParserTest
#include "JSONStreamingParser.h"

const char *json =
	"{'coord':{'lon':-122.09,'lat':37.39},"
	"'sys':{'type':3,'id':168940,'message':0.0297,'country':'US','sunrise':1427723751,'sunset':1427768967},"
	"'weather':[{'id':800,'main':'Clear','description':'Sky is Clear','icon':'01n'}],"
	"'base':'stations',"
	"'main':{'temp':285.68,'humidity':74,'pressure':1016.8,'temp_min':284.82,'temp_max':286.48},"
	"'wind':{'speed':0.96,'deg':285.001},"
	"'clouds':{'all':0},"
	"'dt':1427700245,"
	"'id':0,"
	"'name':'Mountain View',"
	"'cod':200,"
	"'empty_object':{},"
	"'empty_array':[],"
	"'not_empty':[12,13,14,15],"
	"'simple_string':'I am a simple string'"
	"}";
	
const char *jsonPtr;

JSONStreamingParser parser;

// la callback di prova
void callback(uint8_t filter, uint8_t level, const char *name, const char *value, void *cbObj)
{
	Serial.print("Level= ");
	Serial.println(level);
	Serial.print("Name = ");
	Serial.println(name);
	Serial.print("Value= ");
	Serial.println(value);
	Serial.println("--------------");
}

// Codice di inizializzazionecode
void setup(void)
{
	Serial.begin(115200);
	while(!Serial)
		;
	
	// inizializza il parser
	parser.setCallback(callback, NULL);
	
	// parte da inizio stringa di prova
	jsonPtr = json;
	
}

// ciclo infinito
void loop(void)
{
	if(jsonPtr == NULL)
	{
		Serial.println("FINITO!");
		while(true)
			;
	}
	bool res = parser.feed(*jsonPtr++);
	if(res == -1)
	{
		Serial.println("ERRORE!!");
		while(true)
			;
	}
	else if(res == 0)
	{
		Serial.println("FINITO!");
		while(true)
			;
	}
}
