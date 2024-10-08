/* A 3d rotating cube
 Tested and worked with:
 Teensy3,Teensy3.1,Arduino UNO,Arduino YUN,Arduino Leonardo,Stellaris
 Works with Arduino 1.0.6 IDE, Arduino 1.5.8 IDE, Energia 0013 IDE
  ---------------> http://www.buydisplay.com
*/

#include <SPI.h>
#include <FishinoRA8875SPI.h>

// Fishino UNO
#define RA8875_CS 2

#define tft FishinoRA8875SPI

float sin_d[] =
{
	0,0.17,0.34,0.5,0.64,0.77,0.87,0.94,0.98,1,0.98,0.94,
	0.87,0.77,0.64,0.5,0.34,0.17,0,-0.17,-0.34,-0.5,-0.64,
	-0.77,-0.87,-0.94,-0.98,-1,-0.98,-0.94,-0.87,-0.77,
	-0.64,-0.5,-0.34,-0.17
};

float cos_d[] =
{
	1,0.98,0.94,0.87,0.77,0.64,0.5,0.34,0.17,0,-0.17,-0.34,
	-0.5,-0.64,-0.77,-0.87,-0.94,-0.98,-1,-0.98,-0.94,-0.87,
	-0.77,-0.64,-0.5,-0.34,-0.17,0,0.17,0.34,0.5,0.64,0.77,
	0.87,0.94,0.98
};

float d = 30;

float px[] =
{
	-d,  d,  d, -d, -d,  d,  d, -d
};

float py[] =
{
	-d, -d,  d,  d, -d, -d,  d,  d
};

float pz[] =
{
	-d, -d, -d, -d,  d,  d,  d,  d
};

float p2x[] =
{
	0,0,0,0,0,0,0,0
};
float p2y[] =
{
	0,0,0,0,0,0,0,0
};

int r[] =
{
	0,0,0
};

void setup()
{
	Serial.begin(9600);
	Serial.println("RA8875 start");

	tft.begin(RA8875_CS);

}

uint16_t ccolor = GFX_GREEN;
uint8_t ch = 0;

void loop()
{
	tft.fillScreen(GFX_BLACK);
	r[0]++;
	r[1]++;
	if (r[0] == 36)
		r[0] = 0;
	if (r[1] == 36)
		r[1] = 0;
	if (r[2] == 36)
		r[2] = 0;
	
	uint16_t ww = tft.width();
	uint16_t hh = tft.height();
	for (uint8_t i = 0; i < 8; i++)
	{
		float px2 = px[i];
		float py2 = cos_d[r[0]] * py[i] - sin_d[r[0]] * pz[i];
		float pz2 = sin_d[r[0]] * py[i] + cos_d[r[0]] * pz[i];

		float px3 = cos_d[r[1]] * px2 + sin_d[r[1]] * pz2;
		float py3 = py2;
		float pz3 = -sin_d[r[1]] * px2 + cos_d[r[1]] * pz2;

		float ax = cos_d[r[2]] * px3 - sin_d[r[2]] * py3;
		float ay = sin_d[r[2]] * px3 + cos_d[r[2]] * py3;
		float az = pz3 - 190;

		p2x[i] = ww / 2 + ax * 500 / az;
		p2y[i] = hh / 2 + ay * 500 / az;
	}
	
	for (uint8_t i = 0; i < 3; i++)
	{
		tft.drawLine(p2x[i], p2y[i], p2x[i+1], p2y[i+1], ccolor);
		tft.drawLine(p2x[i+4], p2y[i+4], p2x[i+5], p2y[i+5], ccolor);
		tft.drawLine(p2x[i], p2y[i], p2x[i+4], p2y[i+4], ccolor);
	}
	
	tft.drawLine(p2x[3], p2y[3], p2x[0], p2y[0], ccolor);
	tft.drawLine(p2x[7], p2y[7], p2x[4], p2y[4], ccolor);
	tft.drawLine(p2x[3], p2y[3], p2x[7], p2y[7], ccolor);
	
#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__SAM3X8E__)
	delay(40);
#else
	delay(10);
#endif

	if (ch >=20)
	{
		ch = 0;
		ccolor = random(GFX_BLUE, GFX_WHITE);
	}
	else
	{
		ch++;
	}

}
