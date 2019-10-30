#include <ESP8266WiFi.h>

//char* ssid = "Infostrada-2.4GHz-9454A5";
//char* password = "0525646993722559";
//char* ssid = "AndroidAP5ad0";
//char* password = "gbuy4505";
char* ssid = "TIM-31734817";
char* password = "7dZ7sh43mfBiibn5";
char* thingName = "counter";
char* protocol = "http";
int portNumber = 80;
String header;

char myString[250];

IPAddress ipS, ipC;
WiFiServer server(portNumber);

void connection(char* ssid, char* password) {
  
  WiFi.begin(ssid, password);
    
  Serial.print("\nConnecting to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    
    delay(1000);
    Serial.print(".");
 
  }

  Serial.println("\nConnected");
  Serial.print("IP address: ");
  ipS = WiFi.localIP();
  Serial.println(ipS);

  server.begin();
  
}

void setup() {
  
  Serial.begin(115200);
  
  connection(ssid, password);

  char url[250];  
  sprintf(url, "%s://%s:%i/%s", protocol, ipS.toString().c_str(), portNumber, thingName);
  Serial.println(url);
  
}

void loop() {

  WiFiClient client = server.available();

  if (client) {

    Serial.println("\nNew Client");
    Serial.print("IP address:");
    ipC = client.remoteIP();
    Serial.println(ipC);
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        
        char c = client.read();
        Serial.write(c);
        header += c;

        if (c == '\n') {
          if (currentLine.length() == 0) {
            
            client.println("HTTP/1.1 200 OK");

            // se si scieglie come Content-Type JSON, non si deve inserire nessun tag HTML
            // ma direttamente il file JSON 
            client.println("Content-Type: application/ld+json");
            client.println("");

            client.println("{");
            sprintf(myString, "\"name\":\"%s\",", thingName);
            client.println(myString);
            client.println("\"@context\":\"https://www.w3.org/2019/wot/td/v1\"");
            client.println("}");
            client.println("");

            break;  
                
          }
          else {
            
            currentLine = "";
          }
        }
        else if (c != '\r') {
          
          currentLine += c;
        }
      }
    }

    header = "";

    client.stop();
    Serial.println("Client disconnected");
    Serial.print("IP address:");
    Serial.println(ipC);
    
  }
}
