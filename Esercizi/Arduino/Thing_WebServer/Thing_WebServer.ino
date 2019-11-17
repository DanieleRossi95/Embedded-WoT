#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

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
bool subscribe_event1 = false;
int property1_lastValue = property1_value;

//requests:
String req1 = "/";
String req2 = "/" + thingName;
String req3 = "/" + thingName + "/properties/" + property1_name;
String req4 = "/" + thingName + "/actions/" + action1_name;
String req5 = "/" + thingName + "/events/" + event1_name;

String header;
String json;

IPAddress ipS;
ESP8266WebServer server(portServer);
WebSocketsServer webSocket = WebSocketsServer(portSocket);


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

  // richieste server  
  // con on si specifica come viene gestita (funzione passata econdo parametro) la richiesta da parte di un 
  // client all'indirizzo specificato come primo parametro
  server.on(req1, HTTP_GET, handleReq1);
  server.on(req2, HTTP_GET, handleReq2);
  server.on(req3, HTTP_GET, handleReq3);
  server.on(req4, HTTP_POST, handleReq4);

  // il server si deve far partire una volta terminato di gestire tutte le richieste
  server.begin();
  webSocket.begin();
  
  webSocket.onEvent(webSocketEvent);
  
  Serial.println("Server started");
  Serial.println(urlServer);
  
}


void loop() {

  webSocket.loop();

  // gestione delle richieste dei client
  server.handleClient();
  
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

  urlServer = protocolServer + "://" + ipS.toString() + ":" + portServer + "/" + thingName;
  urlSocket = protocolSocket + "://" + ipS.toString() + ":" + portSocket + "/" + thingName;
  
}


// se si vuole modificare una variabile globale, non la si deve passare come parametro
// poiché, altrimenti, la modifica viene persa quando si esce dallo scope della funzione 
void increment() {
  property1_value = property1_value + 1;
}


// funzione per gestire le richieste sulla root del server
void handleReq1() {

  Serial.println("\nGET thing url");
  
  json = "[\"" + urlServer + "\"]";
  server.send(200, "application/ld+json", json);
  
}


void handleReq2() {
  
  Serial.println("\nGET Thing Description"); 
  server.send(200, "application/ld+json", td);
 
}


void handleReq3() {
  
  Serial.print("\nGET ");
  Serial.print(property1_name);
  Serial.println(" value");
  
  json = "{\"" + property1_name + "\":" + property1_value + "}";
  server.send(200, "application/ld+json", json);
 
}


void handleReq4() {

  String body = "Body received: " + server.arg("plain");
  Serial.println(body);
  
  // controllo se il body della richiesta è stato ricevuto
  // se si vuole controllare la presenza di un determinato argomento del body, lo si deve
  // eguagliare a false invece che true
  // plain sta ad indicare l'assenza di argomenti nel body della richiesta (body vuoto)
  // qualsiasi sia il content-type della richiesta, il server lo vedrà come una stringa
  /*if (server.hasArg("plain") == false) { 
    
    server.send(400, "text/plain", "Bad Request: Wrong Body");
    return;
 
  }*/

  Serial.print("\nPOST invokeaction ");
  Serial.println(action1_name);
  
  increment();
  Serial.print("New value: ");
  Serial.println(property1_value);
  json = "{\"" + property1_name + "\":" + property1_value + "}";

  server.send(200, "application/ld+json", json);
 
}


void handleReq5() {
  
  Serial.println("\nGET Thing Description"); 
  server.send(200, "application/ld+json", td);
 
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {

  IPAddress ip;
   
  switch(type) {
    case WStype_DISCONNECTED: 
    {
      Serial.print(num);
      Serial.println(" -> Disconnected");  
    }
    break;

    case WStype_CONNECTED:
    {
      ip = webSocket.remoteIP(num);
      Serial.print(num);
      Serial.print(" -> Subscribe request from ");
      Serial.println(ip);
      Serial.print("Payload: ");
      Serial.println((char *) payload);
    
      // invia la risposta al client
      webSocket.sendTXT(num, "Subscribed");
    }
    break;

    case WStype_TEXT:
    { 
      ip = webSocket.remoteIP(num);
      Serial.print(num);  
      Serial.println(" -> Get Text from ");
      Serial.println(ip);
      Serial.print("Payload: ");
      Serial.println((char *) payload);

      // invia la risposta al client
      // webSocket.sendTXT(num, "message here");

      // invia la risposta a tutti i client connessi
      // webSocket.broadcastTXT("message here");
    }
   
    break;
  }
} 
