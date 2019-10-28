// Server.nut

server <- FishinoServer(80);
server.begin();
print("Server started!\n");
while(true) {
	
	if(server.hasClients()) {
		local client  = server.available();
		print("New client\n");
		local currentLineIsBlank = true;
		while(client.connected()) {
			if (client.available())
			{
				local c = client.read();
				print(c.tochar());
				if (c == '\n' && currentLineIsBlank)
				{
					// send a standard http response header
					// invia uno header standard http
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: text/html");
					client.println("Connection: close");  // the connection will be closed after completion of the response
					client.println("Refresh: 0.5");  // refresh the page automatically every 5 sec
					client.println("");
					client.println("<!DOCTYPE HTML>");
					client.println("<html>");
					
					// output the value of each analog input pin
					// invia il valore di tutti i pins analogici
					for (local analogChannel = 0; analogChannel < 6; analogChannel++)
					{
						local sensorReading = analogRead(analogChannel);
						client.print("analog input ");
						client.print(analogChannel);
						client.print(" is ");
						client.print(sensorReading);
						client.println("<br />");
					}
					client.println("</html>");
					break;
				}
				if (c == '\n')
				{
					// you're starting a new line
					// inizio di una nuova linea
					currentLineIsBlank = true;
				}
				else if (c != '\r')
				{
					// you've gotten a character on the current line
					// sono stati ricevuti dei caratteri nella linea corrente
					currentLineIsBlank = false;
				}
			}
		} // while connected

		// give the web browser time to receive the data
		// lascia tempo al browser per ricevere i dati
//		delay(1);

		// close the connection:
		// chiudi la connessione
		client.stop();
		print("client disonnected\n");
	}
}

				

