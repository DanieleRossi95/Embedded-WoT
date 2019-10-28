void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

int X=0;
int frequenza = 10000;
long stopTime;
bool stopIncrement = false;

void loop() {
  // put your main code here, to run repeatedly:
  X = X+1;
  if(X <= 50) {
    Serial.print("Hello world, iterazione no: ");
    Serial.println(X);
    if(frequenza > 100) {
      delay(frequenza);
      frequenza = frequenza / 2;
    }
    else {
      if(!stopIncrement) {
        stopTime = millis();
        stopIncrement = true;
        delay(frequenza);
      }
      else {
        if(millis() - stopTime >= 5000) {
          frequenza = 10000;
          stopIncrement = false;
        }
        delay(frequenza);  
      }
    }
  }
  else {
    X = 0;
  }
  
}
