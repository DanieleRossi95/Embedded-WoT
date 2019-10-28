function blink(pin, tim) {
	print("Starting blink(" + pin + ", " + tim + ")\n");
	pinMode(pin, OUTPUT);
	while(true) {
		digitalWrite(pin, HIGH);
		sleep(tim);
		digitalWrite(pin, LOW);
		sleep(tim);
	}
}

schedule(@() blink(25, 200));
run();
