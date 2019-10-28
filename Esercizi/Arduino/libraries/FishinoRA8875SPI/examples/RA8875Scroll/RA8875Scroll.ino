/*
Basic Scroll example
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


void setup()
{
	Serial.begin(9600);
	//while (!Serial) {;}
	Serial.println("RA8875 start");
	//initialization routine
	tft.begin(RA8875_CS);

	//following it's already by begin function but
	//if you like another background color....
	tft.fillScreen(GFX_BLACK);//fill screen black

	//RA8875 it's capable to draw graphic but also Text
	//here we switch to TEXT mode
	tft.changeMode(TEXT);
	//now set a text color, background transparent
	tft.setTextColor(GFX_WHITE);
	//use the classic print an println command
	tft.print("www.buydisplay.com");
	//by default the text location is set to 0,0
	//now set it at 50,20 pixels and different color
	tft.setCursor(50,20);//set cursor work in pixel!!!
	//this time we not using transparent background
	tft.setTextColor(GFX_RED,GFX_GREEN);
	tft.print("www.buydisplay.com");
	//by default we using the internal font
	//so some manipulation it's possible
	tft.setFontScale(1);//font x2
	tft.setTextColor(GFX_RED);
	tft.print("Hello World");
	//You notice that font location has been
	//automatically increased by chip, unfortunatly not
	//tracked by library but we can use a command for that...
	int16_t currentX,currentY;
	tft.getCursor(currentX,currentY);
	//now we have the location, lets draw a white pixel
	tft.changeMode(GRAPHIC);//first we swith in graphic mode
	tft.drawPixel(currentX,currentY,GFX_WHITE);
	//did you see the white dot?
	tft.changeMode(TEXT);//go back to text mode
	tft.setFontScale(0);//font x1
	tft.setCursor(0,50);
	tft.setTextColor(GFX_YELLOW);
	tft.println("ABCDEF 1 2 3 4");//this time println!
	tft.setFontSpacing(5);//now give 5 pix spacing
	tft.println("ABCDEF 1 2 3 4");
	tft.setFontSpacing(0);//reset
	tft.setFontRotate(true);
	tft.println("www.buydisplay.com");
	tft.setFontRotate(false);
	tft.setFontScale(2);//font x1
	tft.setTextColor(GFX_BLUE,GFX_BLACK);
}

uint16_t i = 0;

void loop()
{
	tft.setScrollWindow(0,320,0,60);	//Specifies scrolling activity area
	i=0;
	while (i++<60)
	{
		delay(10);
		tft.scroll(i,i);
	} //Note:  scroll offset value must be less than  scroll setting range
	while (i-->0)
	{
		delay(10);
		tft.scroll(i,i);
	}
	while (i++<60)
	{
		delay(10);
		tft.scroll(i,i);
	}
	while (i-->0)
	{
		delay(10);
		tft.scroll(i,i);
	}
}
