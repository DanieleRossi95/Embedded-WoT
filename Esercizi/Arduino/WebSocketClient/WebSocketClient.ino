#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WebSocketsClient.h>

const char* ssid = "Infostrada-2.4GHz-9454A5";
const char* password = "0525646993722559";

WebSocketsClient webSocket;
HTTPClient http; 

int count = 1;
bool subscribed = false;

void setup() {
  
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion
    
    delay(1000);
    Serial.print(".");
 
  }

  Serial.println("\nConnected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // indirizzo del server, ports della websocket e URL
  // con begin si invia al WebSocketServer un messaggio di tipo (WStype_t) WStype_CONNECTED
  webSocket.begin("192.168.1.9", 81, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);

  // use HTTP Basic Authorization this is optional remove if not needed
  // webSocket.setAuthorization("user", "Password");

  // se la connessione con il WebSocketServer è persa, si riprova a connettersi
  // dopo 5 secondi
  webSocket.setReconnectInterval(5000);
  
  // start heartbeat (optional)
  // ping server every 15000 ms
  // expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times
  // webSocket.enableHeartbeat(15000, 3000, 2);
  
}


void loop() {
  webSocket.loop();
  
  if(subscribed) {
    if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
      
       http.begin("http://192.168.1.9:80/counter/actions/increment");   //Specify request destination
       http.addHeader("Content-Type", "text/plain");  //Specify content-type header
       
       int httpCode = http.POST("pippo");   //Send the request
    
       if(httpCode > 0) {
          
         String payload = http.getString();   //Get the response payload
         Serial.print("\nRequest n: ");
         Serial.println(count);
         Serial.println("httpCode:");
         Serial.println(httpCode); 
         Serial.println("payload:"); 
         Serial.println(payload);   //Print the response payload (content received from the POST request)
       
       }
       else {
        
        Serial.print("Error: ");
        Serial.println(http.errorToString(httpCode).c_str());
        
       }
     
       http.end();  //Close connection
     
     }
     else {
     
        Serial.println("Error in WiFi connection");   
     
     }

     count++;
     delay(10000);
  }
  
}


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: {
      Serial.printf("[WSc] Connected to url: %s\n", payload);

      // send message to server when Connected
      webSocket.sendTXT("Connected");
    }
      break;
    case WStype_TEXT: {
      Serial.printf("[WSc] get text: %s\n", payload);

      String message = "Connection confirmed";
      if(message.equals((char *) payload)) {
        webSocket.sendTXT("Subscribe change");
        subscribed = true;
      }

      // send message to server
      // webSocket.sendTXT("message here");
    }  
      break;
     case WStype_PING:
       // pong will be send automatically
       Serial.printf("[WSc] get ping\n");
       break;
      case WStype_PONG:
        // answer to a ping we send
        Serial.printf("[WSc] get pong\n");
        break;
    }
}

