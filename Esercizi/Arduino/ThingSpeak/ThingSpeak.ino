#include <Fishino.h>
#include <FishinoClient.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <ThingSpeak.h>

FishinoClient client;

float temp; //valore temperatura
float hum; //valore umidità

float prevTemp = 0; //previsione temperatura
float prevHum = 0; //previsione umidità
float alpha = 0.5;

#define DHTPIN 6 // what pin we're connected to
#define DHTTYPE DHT11 // DHT 11

char ssid[] = "iPhone di Daniele";     // your network SSID (name) 
char pass[] = "02ei35jzufpvp";        // your network password
unsigned long myChannelNumber = 782699;   // Replace the 0 with your channel number
const char * myWriteAPIKey = "GUX0CRJL83J8CZYH";    // Paste your ThingSpeak Write API Key between the quotes 

DHT dht(DHTPIN, DHTTYPE);

void connection() {
  boolean con = false;
  
  Fishino.begin(ssid, pass);

  Fishino.staStartDHCP();

  while(!con) {
    if(Fishino.status() == STATION_GOT_IP) {
      con = true;   
    }
    else {
      Serial.println(Fishino.status());
    } 
  }

  Serial.print("IP address: ");
  Serial.println(Fishino.localIP());
  Serial.println();

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  connection();
  
  dht.begin();
  ThingSpeak.begin(client); 

} 

void loop() {
  // put your main code here, to run repeatedly:
  
  temp = dht.readTemperature();
  prevTemp = ((1-alpha)*prevTemp) + (alpha*temp);
  delay(200);
  
  hum = dht.readHumidity();
  prevHum = ((1-alpha)*prevHum) + (alpha*hum);
  delay(200);
  
  Serial.println(temp);
  Serial.println(hum);
  Serial.println();
  Serial.println(prevTemp);
  Serial.println(prevHum);
  Serial.println();
  
  if(Fishino.status() == STATION_GOT_IP) {
    ThingSpeak.setField(1, temp);
    ThingSpeak.setField(2, hum);
    ThingSpeak.setField(3, prevTemp);
    ThingSpeak.setField(4, prevHum);
    
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  
    // Check the return code
    if(x == 200){
      Serial.println("Channel update successful.");
      Serial.println();
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
      Serial.println();
    }
  }

  delay(20000);
} 
