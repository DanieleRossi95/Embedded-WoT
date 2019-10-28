#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
 
const char* ssid = "Infostrada-2.4GHz-9454A5";
const char* password = "0525646993722559";

int count = 1;
 
void setup () {
 
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("\nConnecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    
    delay(1000);
    Serial.print(".");
 
  }

   Serial.println("\nConnected to the WiFi network");
   Serial.print("IP address: ");
   Serial.println(WiFi.localIP());
  
}
 
void loop() {
 
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
 
    HTTPClient http;  //Declare an object of class HTTPClient

    //Insert as URL the ip address not the word localhost
    http.begin("http://192.168.1.10:8080/counter/all/properties");  //Specify request destination
    
    int httpCode = http.GET();  //Send the request
 
    if (httpCode > 0) { //Check the returning code
      
      String payload = http.getString();   //Get the request response payload
      Serial.print("\nRequest n: ");
      Serial.println(count);
      Serial.println("httpCode:");
      Serial.println(httpCode); 
      Serial.println("payload:"); 
      Serial.println(payload);   //Print the response payload (content received from GET request)
    }
    else {
      
      Serial.println("Error on HTTP request");
    }
 
    http.end();   //Close connection
 
  }

  count++;
  delay(15000);  //Send a request every 30 seconds
 
}
