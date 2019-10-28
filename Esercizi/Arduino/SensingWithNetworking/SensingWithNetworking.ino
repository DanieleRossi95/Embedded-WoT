/*
credenziali rete:
raspAP
raspAPuino

Fishino.begin(nome_rete, password)

Fishino.staStartDHCP()

si cicla con un while dopo la chiamata al DHCP finchè Fishino.status diventa uguale a STATION_GOT_IP

Fishino.localIP() restituisce l'IP

192.168.1.105

*/

#include <Fishino.h>

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

unsigned int localPort = 12000;

// the UDP client/server
FishinoUDP Udp;

// buffer to hold incoming packet
char packetBuffer[255];

// a string to send
char ReplyBuffer[] = "CIAO";

int inputPinPIR = 7;
int inputPinLED = 6;
int val = 0;

void setup() {
  // put your setup code here, to run once:
  //pinMode(inputPinPIR, INPUT);
  //pinMode(inputPinLED, OUTPUT); 
  Serial.begin(115200);
  connection();
  
  // starts listening on local port
  Udp.begin(localPort);
}

void loop() {
  // put your main code here, to run repeatedly:
  // ricezione messaggio di ritorno
  // parsePacket prende i valori dal buffer
  int packetSize = Udp.parsePacket();
  if(packetSize) {
    Udp.read(packetBuffer, 255);
    Serial.println(packetBuffer);
    val = HIGH;
    digitalWrite(inputPinLED, val);
  }
  else {
    val = LOW;
    digitalWrite(inputPinLED, val);    
  }

  val = digitalRead(inputPinPIR);
  if(val == HIGH) {
    Serial.println("Motion detected!");
    Udp.beginPacket("192.168.1.105", localPort);
    Udp.write(ReplyBuffer);
    Udp.endPacket();    
  }
  
  delay(3000);
}
