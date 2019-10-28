#include <Arduino.h>

#include <FishinoStl.h>
/*
 * Test std::vector
 */

struct TestVector
{

	static void RunTest()
	{

		std::ohserialstream serial(Serial);
		std::vector<int> vec;
		std::vector<int>::const_iterator it;
		int i;

		vec.reserve(50);
		for (i = 0;i < 50;i++)
			vec.push_back(i);

		for (it = vec.begin();it != vec.end();it++)
			serial << *it << " ";

		serial << "OK" << std::endl;
	}

};

TestVector v;

void setup()
{
	Serial.begin(115200);
	while (!Serial)
		;
}

void loop()
{
	delay(1000);
	v.RunTest();
}

