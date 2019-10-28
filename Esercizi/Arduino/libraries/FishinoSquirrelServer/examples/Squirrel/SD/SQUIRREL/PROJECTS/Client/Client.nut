// Client.nut

client <- FishinoClient();
//addr <- IPAddress(213,239,208,176);
addr <- "www.fishino.it";
while(1) {
	print("Connectint to " + addr + "\n");
	client.connect(addr, 80);
	client.println("GET / HTTP/1.1");
	client.println("Host: www.veneto.com");
	client.println("");
	for(local i = 0; i < 30000; i++)
		;
	while(client.available()) {
		local s = client.read(100);
		print(s);
		local k = 0;
		while(!client.available() && k < 200)
			k++;
	}
	print("Stopping client\n");
	client.stop();
}
