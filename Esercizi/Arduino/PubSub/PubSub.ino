#include <Fishino.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

void connection() {
  boolean con = false;
  
  Fishino.begin("raspAP", "raspAPuino");

  Fishino.staStartDHCP();

  while(!con) {
    if(Fishino.status() == STATION_GOT_IP) {
      con = true;   
    }
    else {
      Serial.println(Fishino.status());
    } 
  }

  Serial.println(Fishino.localIP());

}

#define DHTPIN 7 // what pin we're connected to
#define DHTTYPE DHT22 // DHT 22  (AM2302)
#define MQTT_SERVER "192.168.1.1"
#define MQTT_SERVER_PORT 1883

DHT dht(DHTPIN, DHTTYPE);

FishinoClient clientFishino;
PubSubClient clientMQTT;

//Variables
float hum;  //Stores humidity value
float temp; //Stores temperature value

int clientID = "DHT22";
boolean connected;
int succeed;

char* topic = "sensor/DHT22";
char payload[255];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dht.begin();
  connection();
  clientMQTT.setClient(clientFishino);
  clientMQTT.setServer(MQTT_SERVER, MQTT_SERVER_PORT);
}

void loop() {
  // put your main code here, to run repeatedly:
  connected = clientMQTT.connected();
  if(!connected) {
    connected = clientMQTT.connect(clientID);
  }
  if(connected) {
    //Read data and store it to variables hum and temp
    //hum = dht.readHumidity();
    temp = dht.readTemperature();
    sprintf(payload, "%f", temp);
    succeed = clientMQTT.publish(topic, payload);
    clientMQTT.loop();
  
    if(succeed) {
      Serial.println("Messaggio inviato");
    }
  }
  else {
    return(false);
  }
  
  delay(2000);
}
