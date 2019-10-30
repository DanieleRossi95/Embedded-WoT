#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Infostrada-2.4GHz-9454A5";
const char* password = "0525646993722559";

int count = 1;

void setup() {
 
  Serial.begin(115200);   //Serial connection
  WiFi.begin(ssid, password);   //WiFi connection

  Serial.print("\nConnecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion
    
    delay(1000);
    Serial.print(".");
 
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
}
 
void loop() {
 
 if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
 
   HTTPClient http;    //Declare object of class HTTPClient
 
   http.begin("http://192.168.1.10:8080/counter/actions/increment");   //Specify request destination
   http.addHeader("Content-Type", "application/json");  //Specify content-type header
   
   int httpCode = http.POST("invokeaction");   //Send the request
   
   String payload = http.getString();   //Get the response payload
   Serial.print("\nRequest n: ");
   Serial.println(count);
   Serial.println("httpCode:");
   Serial.println(httpCode); 
   Serial.println("payload:"); 
   Serial.println(payload);   //Print the response payload (content received from GET request)
 
   http.end();  //Close connection
 
 }
 else {
 
    Serial.println("Error in WiFi connection");   
 
 }

  count++;
  delay(10000);  //Send a request every 30 seconds
 
}
