#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

//192.168.1.105:12000

#define COAP_PORT 5683
#define LED_pin D7

// the UDP client/server
WiFiUDP udp; 

Coap coap(udp);

// LED STATE
bool LEDSTATE = false;

// CoAP server endpoint URL
void callback_light(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Light] ON/OFF");
  
  // send response
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  
  String message(p);

  Serial.print("Message: ");
  Serial.println(message);

  if (message.equals("ledOff"))
    LEDSTATE = false;
  else if(message.equals("ledOn"))
    LEDSTATE = true;
      
  if (LEDSTATE) {
    digitalWrite(LED_pin, HIGH) ; 
    //coap.sendResponse(ip, port, packet.messageid, "ledOn"); //utilizzata per la risposta dal client
  } else { 
    digitalWrite(LED_pin, LOW) ; 
    //coap.sendResponse(ip, port, packet.messageid, "ledOff"); //utilizzata per la risposta dal client
  }
}

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Coap Response got]");
  
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  
  Serial.println(p);
}

void connection() {
  WiFi.begin("raspAP", "raspAPuino");
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); 
}

void setup() {
  // put your setup code here, to run once:
  //pinMode(LED_pin, OUTPUT); 
  //digitalWrite(LED_pin, LOW);
  Serial.begin(115200);
  Serial.println("Sono qui");
  connection();

  udp.begin(COAP_PORT);

  Serial.println("Setup Callback Light");
  coap.server(callback_light, "ledOn");
  coap.server(callback_light, "ledOff");

  coap.start();
}

void loop() {
  // put your main code here, to run repeatedly:
  coap.loop();
}
