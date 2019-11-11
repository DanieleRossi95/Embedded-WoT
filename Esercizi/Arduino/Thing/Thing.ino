#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>

/* TODO:
-gestire gli eventi attraverso una variabile booleana che inizialmente è impostata su false, 
poi quando un client fa una richiesta GET sulla risorsa (url) dell'evento, settare la variabile
a true e ogni volta che la condizione dell'evento è vera e il valore della variabile è true
si esegue il codice del'evento

-vedere se è possibile far connettere la board alla stessa rete del PC invece che fare la connesione manuale
*/

const char* ssid = "Infostrada-2.4GHz-9454A5";
const char* password = "0525646993722559";
//const char* ssid = "AndroidAP5ad0";
//const char* password = "gbuy4505";
//const char* ssid = "TIM-31734817";
//const char* password = "7dZ7sh43mfBiibn5";
String protocolServer = "http";
String protocolSocket = "ws";
String urlServer;
String urlSocket;
int portServer = 80;
int portSocket = 81;

String thingName = "counter";
String td;

// properties:
String property1_name = "count";
String property1_type = "integer";
int property1_value = 0;

// actions:
String action1_name = "increment";

// events:
String event1_name = "change";
int property1_lastValue = property1_value;

//requests:
String req1 = "GET /";
String req2 = "GET /" + thingName;
String req3 = "GET /" + thingName + "/properties/" + property1_name;
String req4 = "POST /" + thingName + "/actions/" + action1_name;
String req5 = "GET /" + thingName + "/events/" + event1_name;

String header;
String json;

IPAddress ipS, ipC;
WiFiServer server(portServer);
//WebSocketsServer webSocket = WebSocketsServer(portSocket);


void setup() {
  
  Serial.begin(115200);
  
  connection(ssid, password);

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
                      "\"href\": \"" + urlServer + "/properties/" + property1_name + "\","
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
               "\"forms\": ["
                  "{"
                    "\"href\": \"" + urlServer + "/actions/" + action1_name + "\","
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
      "},"
      "\"events\": {"
          "\"change\": {"
              "\"forms\": ["
                "{"
                    "\"href\": \"" + urlServer + "/events/" + event1_name + "\","
                    "\"contentType\": \"application/json\","
                    "\"subprotocol\": \"longpoll\","
                    "\"op\": ["
                        "\"subscribeevent\""
                    "]"
                "},"
                "{"
                    "\"href\": \"" + urlSocket + "/events/" + event1_name + "\","
                    "\"contentType\": \"application/json\","
                    "\"op\": ["
                        "\"subscribeevent\""
                    "]"
                "}"
              "],"
              "\"description\": \"notifica cambiamento valore della property " + property1_name + "\""
          "}"
      "}"          
  "}";
  
}


void loop() {

  //webSocket.loop();
  
  WiFiClient client = server.available();

  if (client) {

    Serial.println("\nNew Client");
    Serial.print("IP address: ");
    ipC = client.remoteIP();
    Serial.println(ipC);
    Serial.println("");
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        
        char c = client.read();
        Serial.write(c);
        header += c;

        if (c == '\n') {
          if (currentLine.length() == 0) {

            // con client.println(stringa) si invia al client una risposta, in seguito alla sua richiesta,
            // contenuta nell'header
            // il contenuto della risposta corrisponde alla stringa passata come parametro a println
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
              Serial.println("GET thing urlServer");
              json = "[\"" + urlServer + "\"]";
              client.println(json);
            }
            else if (header.indexOf(req2) >= 0 && header.indexOf("H") == req2.length() + 1) {
               Serial.println("GET Thing Description");
               client.println(td);
            }
            else if (header.indexOf(req3) >= 0 && header.indexOf("H") == req3.length() + 1) {
              Serial.print("GET ");
              Serial.print(property1_name);
              Serial.println(" value");
              json = "{\"" + property1_name + "\":" + property1_value + "}";
              client.println(json);
            }
            //TODO: 
            // non è stato implementato il controllo sul body della POST Request: la POST Request su una action
            // deve essere accettata solamente nel caso in cui il body sia "invokeaction" 
            else if (header.indexOf(req4) >= 0 && header.indexOf("H") == req4.length() + 1) {
              Serial.print("POST invokeaction ");
              Serial.println(action1_name);
              increment();
              Serial.print("New value: ");
              Serial.println(property1_value);
              json = "{\"" + property1_name + "\":" + property1_value + "}";
              client.println(json);
              // event change
              if (property1_lastValue != property1_value) {
                property1_lastValue = property1_value;
                json = 
                "{"
                  "\"count\": {"
                    "\"lastValue\":" + String(property1_lastValue) + ","
                    "\"currentValue\":" + String(property1_value) + "}"
                "}";    
                client.println(json);  
              }
            }
            else if (header.indexOf(req5) >= 0 && header.indexOf("H") == req5.length() + 1) {
              // pagina HTML con funzione javascript per la gestione della WebSocket
              client.println("Content-Type: text/html");
              client.println("");
              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<link rel=\"icon\" href=\"data:,\">");
              client.println("<body><h1>ESP8266 Web Server</h1></body></html>");
            }
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
    Serial.println("\nClient disconnected");
    Serial.print("IP address: ");
    Serial.println(ipC);
    
  }
}


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
  //webSocket.begin();
  //webSocket.onEvent(change);
  Serial.println("Server started");
  
  //char* urlServer; 
  //sprintf(urlServer, "%s://%s:%i/%s", protocolServer, ipS.toString().c_str(), portNumber, thingName);

  urlServer = protocolServer + "://" + ipS.toString() + ":" + portServer + "/" + thingName;
  urlSocket = protocolSocket + "://" + ipS.toString() + ":" + portSocket + "/" + thingName;
  Serial.println(urlServer);
  
}


// se si vuole modificare una variabile globale, non la si deve passare come parametro
// poiché, altrimenti, la modifica viene persa quando si esce dallo scope della funzione 
void increment() {
  property1_value = property1_value + 1;
}


void change(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
   
  if (type == WStype_TEXT) { 
    for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]); 
    }
    
    Serial.println(); 
  } 
} 
