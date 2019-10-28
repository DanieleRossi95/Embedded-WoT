// FileTest.nut

file <- File();
file.open("pippo.txt", O_WRITE | O_CREAT | O_TRUNC);
file.print("Ciao pippo:" + 3.14 + " e' il valore di PIGRECO\n");
file.print("Ma chissenfrega ???\n");
file.print("Trullallero trullalla'\n");
for(local i = 0; i < 100; i++)
	file.print("Numero:" + i + "\n");
file.close();

file.open("pippo.txt", O_READ);
while(file.avail())
	print(file.readLine(100) + "\n");
file.close();
