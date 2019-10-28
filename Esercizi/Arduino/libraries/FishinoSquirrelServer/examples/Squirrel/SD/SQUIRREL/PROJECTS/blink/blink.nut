pinMode(26, 1);
local delayTime = 500;

function delay(tim)
{
	local t = millis() + tim;
	while(millis() < t)
		;
}

while(1)
{
	digitalWrite(26,1);
	delay(delayTime);
	digitalWrite(26,0);
	delay(delayTime);
}