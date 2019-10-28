WoT.fetch("http://192.168.1.143/").then( async (td) => {

	let thing = WoT.consume(td);
    console.info("=== TD ===");
    //console.info(td);
    console.info("==========");
    
    try {
        let read1 = await thing.properties.count.read();
		console.info("\ncount value is", read1, "\n");
    } 
    catch(err) {
		console.error("Script error:", err);
	}

}).catch( (err) => { console.error("Fetch error:", err); });
