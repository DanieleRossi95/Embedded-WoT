// fact.nut

function fact(x)
{
	if(x <= 1)
		return 1;
	else
		return x*fact(x-1);
}

//for(local i = 0.0; i < 150.0; i++)
//	print("fact(" + i + ") = " + fact(i) + "\n");

for(local i = 0.0; i < 35.0; i++)
	Serial.println("fact(" + i + ") = " + fact(i));