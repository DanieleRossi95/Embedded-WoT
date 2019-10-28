function pippo()
{
	print("Entering pippo\n");
	suspend(0);
	print("I'm pippo\n");
	suspend(1);
	print("I'm pippo resumed\n");
	suspend(2);
	print("Bye bye pippo\n");
}

th <- newthread(pippo);

th.call();
do
{
	th.wakeup();
}
while(th.getstatus() == "suspended");
