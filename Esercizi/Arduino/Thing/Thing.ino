#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

//char* ssid = "Infostrada-2.4GHz-9454A5";
//char* password = "0525646993722559";
//char* ssid = "AndroidAP5ad0";
//char* password = "gbuy4505";
const char* ssid = "TIM-31734817";
const char* password = "7dZ7sh43mfBiibn5";
const char* thingName = "counter";
const char* protocol = "http";
int portNumber = 80;
String header;

char myString[250];

IPAddress ipS, ipC;
WiFiServer server(portNumber);

//TODO: 
//gestire i link alle property e alle action presenti nella thing description nello stesso modo di come è stato gestito il click dei bottoni nello skatch web server,
//cioè che al click sul link della property o della action, corrisponda il contenuto della property nel primo caso e l'esecuzione della funzione collegata all'azione nel secondo caso
const char* td = "{\"title\":\"Contatore\",\"description\":\"Contatore Esempio\",\"support\":\"git://github.com/eclipse/thingweb.node-wot.git\",\"@context\":[\"https://www.w3.org/2019/wot/td/v1\",{\"iot\":\"http://example.org/iot\"},{\"@language\":\"it\"}],\"properties\":{\"count\":{\"type\":\"integer\",\"iot:Custom\":\"example annotation\",\"observable\":true,\"readOnly\":true,\"writeOnly\":false,\"forms\":[{\"href\":\"http://192.168.1.5:8080/counter/properties/count\",\"contentType\":\"application/json\",\"op\":[\"readproperty\"],\"htv:methodName\":\"GET\"},{\"href\":\"http://192.168.1.5:8080/counter/properties/count/observable\",\"contentType\":\"application/json\",\"op\":[\"observeproperty\"],\"subprotocol\":\"longpoll\"},{\"href\":\"coap://192.168.1.5:5683/counter/properties/count\",\"contentType\":\"application/json\",\"op\":[\"readproperty\"]}],\"description\":\"valore attuale del contatore\"},\"lastChange\":{\"type\":\"string\",\"observable\":true,\"readOnly\":true,\"writeOnly\":false,\"forms\":[{\"href\":\"http://192.168.1.5:8080/counter/properties/lastChange\",\"contentType\":\"application/json\",\"op\":[\"readproperty\"],\"htv:methodName\":\"GET\"},{\"href\":\"http://192.168.1.5:8080/counter/properties/lastChange/observable\",\"contentType\":\"application/json\",\"op\":[\"observeproperty\"],\"subprotocol\":\"longpoll\"},{\"href\":\"coap://192.168.1.5:5683/counter/properties/lastChange\",\"contentType\":\"application/json\",\"op\":[\"readproperty\"]}],\"description\":\"ultima modifica del valore\"}},\"actions\":{\"increment\":{\"uriVariables\":{\"step\":{\"type\":\"integer\",\"minimum\":1,\"maximum\":250}},\"forms\":[{\"href\":\"http://192.168.1.5:8080/counter/actions/increment{?step}\",\"contentType\":\"application/json\",\"op\":[\"invokeaction\"],\"htv:methodName\":\"POST\"},{\"href\":\"coap://192.168.1.5:5683/counter/actions/increment\",\"contentType\":\"application/json\",\"op\":\"invokeaction\"}],\"idempotent\":false,\"safe\":false,\"description\":\"incrementare valore\"},\"decrement\":{\"uriVariables\":{\"step\":{\"type\":\"integer\",\"minimum\":1,\"maximum\":250}},\"forms\":[{\"href\":\"http://192.168.1.5:8080/counter/actions/decrement{?step}\",\"contentType\":\"application/json\",\"op\":[\"invokeaction\"],\"htv:methodName\":\"POST\"},{\"href\":\"coap://192.168.1.5:5683/counter/actions/decrement\",\"contentType\":\"application/json\",\"op\":\"invokeaction\"}],\"idempotent\":false,\"safe\":false,\"description\":\"decrementare valore\"},\"reset\":{\"forms\":[{\"href\":\"http://192.168.1.5:8080/counter/actions/reset\",\"contentType\":\"application/json\",\"op\":[\"invokeaction\"],\"htv:methodName\":\"POST\"},{\"href\":\"coap://192.168.1.5:5683/counter/actions/reset\",\"contentType\":\"application/json\",\"op\":\"invokeaction\"}],\"idempotent\":false,\"safe\":false,\"description\":\"resettare valore\"}},\"events\":{\"change\":{\"forms\":[{\"href\":\"http://192.168.1.5:8080/counter/events/change\",\"contentType\":\"application/json\",\"subprotocol\":\"longpoll\",\"op\":[\"subscribeevent\"]},{\"href\":\"ws://192.168.1.5:8080/counter/events/change\",\"contentType\":\"application/json\",\"op\":\"subscribeevent\"},{\"href\":\"coap://192.168.1.5:5683/counter/events/change\",\"contentType\":"
"\"application/json\",\"op\":\"subscribeevent\"}],\"description\":\"resettare valore\"}},\"@type\":\"Thing\",\"security\":[\"nosec_sc\"],\"id\":\"urn:uuid:038384cc-947c-4955-b348-e470b57114f0\",\"forms\":[{\"href\":\"http://192.168.1.5:8080/counter/all/properties\",\"contentType\":\"application/json\",\"op\":[\"readallproperties\",\"readmultipleproperties\",\"writeallproperties\",\"writemultipleproperties\"]}],\"securityDefinitions\":{\"nosec_sc\":{\"scheme\":\"nosec\"}}}";

void connection(const char* ssid, const char* password) {
  
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

            //client.println("{");
            //sprintf(myString, "\"name\":\"%s\",", thingName);
            //client.println(myString);
            //client.println("\"@context\":\"https://www.w3.org/2019/wot/td/v1\"");
            //client.println("}");
            client.println(td);
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
