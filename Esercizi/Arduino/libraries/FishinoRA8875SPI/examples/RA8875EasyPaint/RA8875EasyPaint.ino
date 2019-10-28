/* Touch screen super-easy paint!
  This version use the new touch screen functions, much easier!
  Did you have already calibrated your screen? Better do as soon you can
  Open TouchScreenCalibration example and follow instructions.
 Tested and worked with:
 Teensy3,Teensy3.1,Arduino UNO,Arduino YUN,Arduino Leonardo,Stellaris
 Works with Arduino 1.0.6 IDE, Arduino 1.5.8 IDE, Energia 0013 IDE
  ---------------> http://www.buydisplay.com
*/


#include <Wire.h>
#include <FishinoRA8875SPI.h>
#include <FishinoFT5x06.h>

// Fishino UNO
#define RA8875_CS 2
#define TOUCH_INTR TOUCH_IRQ

#define tft FishinoRA8875SPI
#define touch FishinoFT5x06

uint16_t tx, ty;


void interface()
{
	tft.fillRect(10,10,40,40,GFX_WHITE);
	tft.fillRect(10+(40*1)+(10*1),10,40,40,GFX_BLUE);
	tft.fillRect(10+(40*2)+(10*2),10,40,40,GFX_RED);
	tft.fillRect(10+(40*3)+(10*3),10,40,40,GFX_GREEN);
	tft.fillRect(10+(40*4)+(10*4),10,40,40,GFX_CYAN);
	tft.fillRect(10+(40*5)+(10*5),10,40,40,GFX_MAGENTA);
	tft.fillRect(10+(40*6)+(10*6),10,40,40,GFX_YELLOW);
	tft.drawRect(10+(40*7)+(10*7),10,40,40,GFX_WHITE);
}

void setup()
{
	Serial.begin(9600);
	//while (!Serial) {;}
	Serial.println("RA8875 start");

	tft.begin(RA8875_CS);//initialize library

	touch.connect(TOUCH_INTR);//enable Touch support!
	interface();
}

uint16_t choosenColor = 0;

void loop()
{
	tft.changeMode(TEXT);
	tft.setTextColor(GFX_WHITE);
	tft.setCursor(600, 0);
	tft.print("www.buydisplay.com");
	tft.changeMode(GRAPHIC);

	if (touch.touching())
	{
		//easy!
		touch.read(tx, ty);//read directly in pixel!
		tx=800-tx;
		ty=480-ty;
		if (ty >= 0 && ty <= 55)
		{
			//interface area
			if ((tx > 10 && tx < (10+40)))
			{
				choosenColor = GFX_WHITE;
				interface();
				tft.fillRect(10,10,40,40,GFX_BLACK);
				tft.fillCircle(tft.width()-10,10,5,choosenColor);
			}
			else if ((tx > 10+(40*1)+(10*1) && tx < 10+(40*2)+(10*1)))
			{
				choosenColor = GFX_BLUE;
				interface();
				tft.fillRect(10+(40*1)+(10*1),10,40,40,GFX_BLACK);
				tft.fillCircle(tft.width()-10,10,5,choosenColor);
			}
			else if ((tx > 10+(40*2)+(10*2) && tx < 10+(40*3)+(10*2)))
			{
				choosenColor = GFX_RED;
				interface();
				tft.fillRect(10+(40*2)+(10*2),10,40,40,GFX_BLACK);
				tft.fillCircle(tft.width()-10,10,5,choosenColor);
			}
			else if ((tx > 10+(40*3)+(10*3) && tx < 10+(40*4)+(10*3)))
			{
				choosenColor = GFX_GREEN;
				interface();
				tft.fillRect(10+(40*3)+(10*3),10,40,40,GFX_BLACK);
				tft.fillCircle(tft.width()-10,10,5,choosenColor);
			}
			else if ((tx > 10+(40*4)+(10*4) && tx < 10+(40*5)+(10*4)))
			{
				choosenColor = GFX_CYAN;
				interface();
				tft.fillRect(10+(40*4)+(10*4),10,40,40,GFX_BLACK);
				tft.fillCircle(tft.width()-10,10,5,choosenColor);
			}
			else if ((tx > 10+(40*5)+(10*5) && tx < 10+(40*6)+(10*5)))
			{
				choosenColor = GFX_MAGENTA;
				interface();
				tft.fillRect(10+(40*5)+(10*5),10,40,40,GFX_BLACK);
				tft.fillCircle(tft.width()-10,10,5,choosenColor);
			}
			else if ((tx > 10+(40*6)+(10*6) && tx < 10+(40*7)+(10*6)))
			{
				choosenColor = GFX_YELLOW;
				interface();
				tft.fillRect(10+(40*6)+(10*6),10,40,40,GFX_BLACK);
				tft.fillCircle(tft.width()-10,10,5,choosenColor);
			}
			else if ((tx > 10+(40*7)+(10*7) && tx < 10+(40*8)+(10*7)))
			{
				choosenColor = 0;
				interface();
				tft.fillRect(0,52,tft.width()-1,tft.height()-53,GFX_BLACK);
				tft.fillCircle(tft.width()-10,10,5,GFX_BLACK);
			}
		}
		else
		{
			//paint
			//if (choosenColor != 0) tft.fillCircle(tx,ty,1,choosenColor);
			if (choosenColor != 0)
				tft.drawPixel(tx,ty,choosenColor);
		}
	}
}
