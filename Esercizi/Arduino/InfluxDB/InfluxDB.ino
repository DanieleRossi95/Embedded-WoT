#include <Fishino.h>
#include <FishinoClient.h>
#include <DHT.h>

//Grafana server 192.168.1.1/3000

char Response[300];
FishinoClient client;
String POSTData; 

float temp; //valore temperatura
float hum; //valore umidità

#define DHTPIN 7 // what pin we're connected to
#define DHTTYPE DHT11 // DHT 11

DHT dht(DHTPIN, DHTTYPE);

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

  Serial.print("IP address: ");
  Serial.println(Fishino.localIP());

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  connection();
  dht.begin();

} 

void loop() {
  // put your main code here, to run repeatedly:

  temp = dht.readTemperature();
  hum = dht.readHumidity();
  Serial.println(temp);
  Serial.println(hum);
  Serial.println();

  POSTData = "testDR value_a=" + String(temp) + "," + "value_b=" + String(hum);
  
  if (client.connect("192.168.1.1", 8086)) {
    Serial.println("Connected to INFLUXDB server");
  
    // esegue un request Http
    client.println("POST /write?db=iot2019 HTTP/1.1");
    client.println("User-Agent: Arduino/1.6");
    client.print("Host: ");
    client.print(Fishino.localIP());
    client.println(":8086");
    client.println("Accept: */*");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(POSTData.length());
    client.println();
    client.println(POSTData);
  
    delay(500);

    if (client.available()) {
      client.readBytes(Response, client.available());
      Serial.println(Response);
    }
  }

  delay(5000);
} 
