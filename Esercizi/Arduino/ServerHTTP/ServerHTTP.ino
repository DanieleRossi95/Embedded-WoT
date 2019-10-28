#include <Fishino.h>
#include <FishinoServer.h>
#include <FishinoClient.h>

//server generico in ascolto sulla porta 80
FishinoServer server(8080);

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
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  // attende nuovi clienti
  FishinoClient client = server.available();

  if (client)
  {
    Serial.println("new client");
    
    // an http request ends with a blank line
    // ogni richiesta http termina con una linea vuota
    //boolean currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);

        if (c == '\n')
        {
          // send a standard http response header
          // invia uno header standard http
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<body>");
          client.println("Fishino UNO");
          client.println("</body>");
          client.println("</html>");
        }
      }
    }
    // give the web browser time to receive the data
    // lascia tempo al browser per ricevere i dati
    delay(1);

    // close the connection:
    // chiudi la connessione
    client.stop();
    Serial.println("client disonnected");
  }
}
