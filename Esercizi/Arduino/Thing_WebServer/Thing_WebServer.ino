#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

const char* ssid = "Infostrada-2.4GHz-9454A5";
const char* password = "0525646993722559";
//const char* ssid = "AndroidAP5ad0";
//const char* password = "gbuy4505";
//const char* ssid = "TIM-31734817";
//const char* password = "7dZ7sh43mfBiibn5";
//const char* ssid = "WiLMA-Lab";
//const char* password = "wilmalab-wifi!";
String protocolServer = "http";
String protocolSocket = "ws";
String urlServer;
String urlSocket;
int portServer = 80;
int portSocket = 81;

String thingName = "counter";
String td;

// properties:
int properties_number = 1;
String property1_name = "count";
String property1_type = "integer";
int property1_value = 0;

// actions:
int actions_number = 1;
String action1_name = "increment";

// events:
int events_number = 1;
String event1_name = "change";
String events_list[1] = {event1_name};

DynamicJsonDocument doc(1024);
DeserializationError err;

//requests:
String req1 = "/";
String req2 = "/" + thingName;
String req3 = "/" + thingName + "/properties/" + property1_name;
String req4 = "/" + thingName + "/actions/" + action1_name;
String req5 = "/" + thingName + "/all/properties";

String header;
String json;

IPAddress ipS;

ESP8266WebServer server(portServer);

#define HTTP_MAX_SEND_WAIT 60000

WebSocketsServer webSocket = WebSocketsServer(portSocket);

int i, j;

int count = 0;

void setup() {
  
  Serial.begin(115200);
  Serial.println();
  
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
      "},"
      "\"forms\" : ["
        "{"
          "\"href\": \"" + urlServer + "/all/properties\","
          "\"contentType\": \"application/json\","
          "\"op\": ["
            "\"readallproperties\","
            "\"readmultipleproperties\""
          "]"
        "}"
      "]"            
  "}";

  // richieste server  
  // con on si specifica come viene gestita (funzione passata econdo parametro) la richiesta da parte di un 
  // client all'indirizzo specificato come primo parametro
  server.on(req1, HTTP_GET, handleReq1);
  server.on(req2, HTTP_GET, handleReq2);
  server.on(req3, HTTP_GET, handleReq3);
  server.on(req4, HTTP_POST, handleReq4);
  server.on(req5, HTTP_GET, handleReq5);

  // il server si deve far partire una volta terminato di gestire tutte le richieste
  server.begin();
  webSocket.begin();
  
  webSocket.onEvent(webSocketEvent);
  
  Serial.println("Server started");
  Serial.println(urlServer);
}


void loop() {
  // gestione delle richieste dei client
  server.handleClient();
  webSocket.loop();
}


void connection(const char* ssid, const char* password) {
  
  WiFi.begin(ssid, password);
    
  Serial.print("Connecting to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("."); 
  }

  delay(10000);
  
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
  Serial.print(" value -> ");
  Serial.println(++count);

  DynamicJsonDocument tmp(1024);
  JsonObject obj = tmp.createNestedObject();
  obj[property1_name] = property1_value;
  obj["ip"] = server.client().remoteIP().toString();

  WiFiClient c = server.client();
  Serial.print("a");
  c.setTimeout(HTTP_MAX_SEND_WAIT);
  Serial.print("b");
  
  json = "";
  serializeJson(obj, json);
  
  //json = "{\"" + property1_name + "\":" + property1_value + "}";

  if(count == 20) {
    server.send(200, "application/ld+json", json);
    count = 0;
  }
 
}

void handleReq4() {

  Serial.print("\nPOST invokeaction ");
  Serial.println(action1_name);

  String body = server.arg("plain");
  Serial.printf("Body received: %s", body.c_str());

  /*err = deserializeJson(doc, body);
  
  if(err) {
    Serial.print("deserializeJson() failed with code ");
    Serial.println(err.c_str());
  }
  else {
    int i1 = doc["pippo"];
    String i2 = doc["pluto"];
    Serial.printf("\ni1:%d, i2: %s", i1, i2.c_str()); 
  }*/
  
  // controllo se il body della richiesta è stato ricevuto
  // se si vuole controllare la presenza di un determinato argomento del body, lo si deve
  // eguagliare a false invece che true
  // plain sta ad indicare l'assenza di argomenti nel body della richiesta (body vuoto)
  // qualsiasi sia il content-type della richiesta, il server lo vedrà come una stringa
  /*if (server.hasArg("plain") == false) { 
    
    server.send(400, "text/plain", "Bad Request: Wrong Body");
    return;
 
  }*/

  int property1_lastValue = property1_value; 
  
  increment();
  Serial.print("\nNew value: ");
  Serial.println(property1_value);
  
  json = "{\"" + property1_name + "\":" + property1_value + "}";
  server.send(200, "application/ld+json", json);

  // notifica client sottoscritti all'evento change
  json = "{\"" + property1_name + "\":"
    "{\"lastValue\":" + property1_lastValue + ","
    "\"currentValue\": " + property1_value + "}}";
  
  for(i=0; i<doc.size(); i++) {
    JsonArray ae = doc[(String) i];
    for(j=0; j<ae.size(); j++) {
      if(ae[j]["change"]) {
        webSocket.sendTXT((unsigned char) i, json);
        break;
      }
    }
  }
 
}


void handleReq5() {
  
  Serial.print("\nGET all properties");
  
  json = "{\"" + property1_name + "\":" + property1_value + "}";
  server.send(200, "application/ld+json", json);
 
}


// se un WebSocketClient manda al WebSocketServer una richiesta di connessione e il server è attivo,
// allora quest'ultimo invia al client un messaggio di notifica dell'avvenuta connessione di tipo WStype_CONNECTED
// dopo un tot di tempo che il client e il server non si scambiano messaggi, la connessione viene persa
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {

  IPAddress ip;
  String nums;

  Serial.println();
   
  switch(type) {
    case WStype_DISCONNECTED: 
    {
      // quando si disconnette un client, la webSocket non riesce a risalire al suo IP
      // per cui webSocket.remoteIP(num) non restituisce nessun risultato
      Serial.print(num);
      Serial.println(" -> Disconnected");

      // rimozione di tutte le sottoscrizioni del client
      nums = (String) num;
      doc.remove(nums);
      serializeJson(doc, Serial);
      Serial.println();
    }
    break;

    case WStype_CONNECTED:
    {
      ip = webSocket.remoteIP(num);
      Serial.print(num);
      Serial.print(" -> Connection request from ");
      Serial.println(ip);
      Serial.print("Payload: ");
      // il payload di un messaggio di tipo CONNECTED corrisponde all'url al quale il client si vuole connettere,
      // per cui se il client si vuole sottoscrivere all'evento change deve connettersi all'url count/events/change
      // così è possibile sapere a quale evento, il client, si sta cercando di sottoscrivere parsando il payload  
      Serial.println((char *) payload);
    
      // invia la risposta al client
      // prima di inviare il messaggio al client, il server gli manda un ping per verificare che sia connesso
      webSocket.sendTXT(num, "Connection confirmed");

      // se l'IP del client non è presente nel json lo aggiungo
      nums = (String) num;
      if(doc[nums].isNull()) {
        doc.createNestedArray(nums);
        serializeJson(doc, Serial);
        Serial.println();
      }
    }
    break;

    case WStype_TEXT:
    { 
      ip = webSocket.remoteIP(num);
      Serial.print(num);  
      Serial.print(" -> Get Text from ");
      Serial.println(ip);
      Serial.print("Payload: ");
      Serial.println((char *) payload);

      String message;
      int el_size = sizeof(events_list) / sizeof(String);
      
      nums = (String) num;
      for(i=0; i<el_size; i++) {
        message = "Subscribe " + events_list[i];
        if(message.equals((char *) payload)) {
          webSocket.sendTXT(num, "Subscription confirmed");
          if(doc[nums].isNull()) {
            JsonObject obj = doc.createNestedArray(nums).createNestedObject();
            obj[events_list[i]] = true;
          }
          else {
            // prima di aggiungere l'evento, si verifica se è già presente (count != doc[nums].size())
            int count = 0;
            for(j=0; j<doc[nums].size(); j++) {
              if(!doc[nums][j][events_list[i]]) {
                count++;
              }
            }
            if(count == doc[nums].size()) {
              JsonObject obj = doc[nums].createNestedObject();
              obj[events_list[i]] = true;
            }
          }
          serializeJson(doc, Serial);
          Serial.println();
          break;
        }
      }
      
      // invia la risposta a tutti i client connessi
      // webSocket.broadcastTXT("message here");
    }
    break;

    /*case WStype_PONG:
    {
      ip = webSocket.remoteIP(num);
      Serial.print(num);  
      Serial.print(" -> Get PONG from ");
      Serial.println(ip);
      Serial.print("Payload: ");
      Serial.println((char *) payload);
    }
    break;*/
  }
}
