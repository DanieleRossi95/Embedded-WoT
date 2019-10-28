function __runLoop()
{
	::suspend();
	while(true) {
		if(__fin)
			break;
		loop();
		::suspend();
	}
	
	print("DONE\n");
	__fin <- false;
}

function start()
{
	__fin <- false;
	__threads <- [
		{ "thread" : ::newthread(__runLoop), "time" : 0 }
	];
	
	setup();
	__threads[0].thread.call();
	while(__threads[0].thread.getstatus() == "suspended")
		__threads[0].thread.wakeup();
}

function setup()
{
	count <- 0;
}

function loop()
{
	if(count > 200)
		__fin = true;
	else
		print("Count:" + count++ + " Time:" + millis() + "\n");
}

start();

