#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

const char* ssid = "Infostrada-2.4GHz-9454A5";
const char* password = "0525646993722559";
//const char* ssid = "AndroidAP5ad0";
//const char* password = "gbuy4505";
//const char* ssid = "TIM-31734817";
//const char* password = "7dZ7sh43mfBiibn5";
String protocol = "http";
int portNumber = 80;

String thingName = "counter";
String url;
String td;

//proprietà:
String property1_name = "count";
String property1_type = "integer";
int property1_value = 0;

//azioni:
String action1_name = "increment";

//richieste:
String req1 = "GET /";
String req2 = "GET /" + thingName;
String req3 = "GET /" + thingName + "/properties/" + property1_name;
String req4 = "POST /" + thingName + "/actions/" + action1_name;

String header;
String json;

IPAddress ipS, ipC;
WiFiServer server(portNumber);


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

  //char* url; 
  //sprintf(url, "%s://%s:%i/%s", protocol, ipS.toString().c_str(), portNumber, thingName);

  url = protocol + "://" + ipS.toString() + ":" + portNumber + "/" + thingName;
  Serial.print("URL: ");
  Serial.println(url);

  //TODO: 
  //gestire i link alle property e alle action presenti nella thing description nello stesso modo di come è stato gestito il click dei bottoni nello skatch web server,
  //cioè che al click sul link della property o della action, corrisponda il contenuto della property nel primo caso e l'esecuzione della funzione collegata all'azione nel secondo caso
  td = 
  "{"
      "\"title\": \"Contatore\","
      "\"description\": \"Contatore Esempio\","
      "\"support\": \"git://github.com/eclipse/thingweb.node-wot.git\","
      "\"@context\": ["
          "\"https://www.w3.org/2019/wot/td/v1\","
          "{"
              "\"iot\": \"http://example.org/iot\""
          "},"
          "{"
              "\"@language\": \"it\""
          "}"
      "],"
      "\"properties\": {"
          "\"count\": {"
              "\"type\": \"integer\","
              "\"iot:Custom\": \"example annotation\","
              "\"observable\": true,"
              "\"readOnly\": true,"
              "\"writeOnly\": false,"
              "\"forms\": ["
                  "{"
                      "\"href\": \"" + url + "/properties/" + property1_name + "\","
                      "\"contentType\": \"application/json\","
                      "\"op\": ["
                          "\"readproperty\""
                      "],"
                      "\"htv:methodName\": \"GET\""
                  "}"
              "],"
              "\"description\": \"valore attuale del contatore\""
          "}"
      "},"
      "\"actions\": {"
          "\"increment\": {"
              "\"input\" : ["
                  "{"
                      "\"name\": \"" + property1_name + "\","
                      "\"type\": \"" + property1_type + "\""
                   "}"
               "],"
               "\"forms\": ["
                  "{"
                    "\"href\": \"" + url + "/actions/" + action1_name + "\","
                    "\"contentType\": \"application/json\","
                    "\"op\": ["
                        "\"invokeaction\""
                    "],"
                    "\"htv:methodName\": \"POST\""
                  "}"
              "],"
            "\"idempotent\": false,"
            "\"safe\": false,"
            "\"description\": \"incrementa il valore di " + property1_name + "\""    
          "}"      
      "}"  
  "}";
  
}


void loop() {

  WiFiClient client = server.available();

  if (client) {

    Serial.println("\nNew Client");
    Serial.print("IP address: ");
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
            
            if (header.indexOf(req1) >= 0 && header.indexOf("H") == req1.length() + 1) {
              Serial.println("GET thing url");
              json = "[\"" + url + "\"]";
              client.println(json);
            }
            else if (header.indexOf(req2) >= 0 && header.indexOf("H") == req2.length() + 1) {
               Serial.println("GET thing description");
               client.println(td);
            }
            else if (header.indexOf(req3) >= 0 && header.indexOf("H") == req3.length() + 1) {
              Serial.print("GET ");
              Serial.print(property1_name);
              Serial.println(" value");
              client.println(property1_value);
            }
            //else if (header.indexOf(req4) >= 0 && header.indexOf("H") == req4.length() + 1) {
            //}
            //nel caso in cui l'indirizzo a cui si è fatta la richiesta non esiste, si restituisce
            //l'oggetto vuoto
            else {
              client.println("{}");
            }
            
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
