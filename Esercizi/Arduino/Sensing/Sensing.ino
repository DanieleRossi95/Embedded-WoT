// choose the input pin (for PIR sensor)
int inputPinPIR = 7;
int inputPinLED = 6;
// variable for reading the pin status
int val = 0;
// accensione/spegnimento LED
// int as = HIGH;

void setup() {
  // put your setup code here, to run once:
  // declare sensor as input 
  pinMode(inputPinPIR, INPUT);
  pinMode(inputPinLED, OUTPUT); 
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  // read input value
  // il valore letto è relativo alla tensione
  val = digitalRead(inputPinPIR);
  if(val == HIGH) {
    Serial.println("Motion detected!");
    digitalWrite(inputPinLED, val);
  }
  else {
    val = LOW;
    digitalWrite(inputPinLED, val);
  }
  delay(2000);
}
