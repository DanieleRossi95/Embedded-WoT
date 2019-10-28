// threadblink2.nut

function blink(pin, tim) {
	print("Starting blink(" + pin + ", " + tim + ")\n");
	pinMode(pin, OUTPUT);
	while(true) {
		digitalWrite(pin, HIGH);
		Threads.sleep(tim);
		digitalWrite(pin, LOW);
		Threads.sleep(tim);
	}
}

Threads.schedule(@() blink(25, 50));
Threads.schedule(@() blink(26, 500));
Threads.run();
