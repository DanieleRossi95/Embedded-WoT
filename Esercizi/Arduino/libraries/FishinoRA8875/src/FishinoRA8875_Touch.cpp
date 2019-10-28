#include "FishinoRA8875SPI.h"

#if !defined(USE_EXTERNALTOUCH)

/**************************************************************************/
/*!   Initialize support for on-chip resistive Touch Screen controller
	  It also enable the Touch Screen
	  Parameters:
	  intPin:pin connected to RA8875 INT
*/
/**************************************************************************/
void FishinoRA8875SPIClass::touchBegin(uint8_t intPin)
{
	_touchPin = intPin;
	pinMode(_touchPin, INPUT);
	digitalWrite(_touchPin, HIGH);
	//auto mode + debounce on
	writeReg(RA8875_TPCR1, RA8875_TPCR1_AUTO | RA8875_TPCR1_DEBOUNCE);
	touchEnable(true);
}

/**************************************************************************/
/*!
      Enables or disables the on-chip touch screen controller
	  Parameters:
	  enabled:true(enable),false(disable)
*/
/**************************************************************************/
void FishinoRA8875SPIClass::touchEnable(boolean enabled)
{
	if (_touchPin < 255)
	{
		if (!_touchEnabled && enabled)
		{
			_INTC1Reg |= (1 << 2); //bit set 2
			writeReg(RA8875_INTC1, _INTC1Reg);
			_TPCR0Reg |= (1 << 7); //bit set 7
			writeReg(RA8875_TPCR0, _TPCR0Reg);
			_touchEnabled = true;
		}
		else
			if (_touchEnabled && !enabled)
			{
				_INTC1Reg &= ~(1 << 2); //clear bit 2
				writeReg(RA8875_INTC1, _INTC1Reg);
				_TPCR0Reg &= ~(1 << 7); //clear bit 7
				writeReg(RA8875_TPCR0, _TPCR0Reg);
				_touchEnabled = false;
			}
	}
}

/**************************************************************************/
/*!   Detect a touch and return true, otherwise false.
	  It also correctly resets INT register to avoid false detections.
	  Will not work at all (return false) if touchBegin/touchEnable not set.
	  Using autoclear=true:
	  This is useful to detect any press without get coordinates!
	  Note that if you are not using autoclear you will need a readTouchADC or readTouchPixel
	  just after touchDetect or the INT register will not clear and you will get a loopback!
	  In contrast, using autoclear and readTouchADC/readTouchPixel will result in wrong readings.
	  Parameters:
	  Autoclear:(true/false) set true when you want to use this function standalone and
	  not followed by a coordinate reading with readTouchADC/readTouchPixel
*/
/**************************************************************************/
boolean FishinoRA8875SPIClass::touchDetect(boolean autoclear)
{
	if (_touchEnabled)
	{
		if (!digitalRead(_touchPin))
		{
			_clearTInt = true;
			if (touched())
			{
				if (autoclear)
					clearTouchInt();
				return true;
			}
			else
			{
				return false;
			}
		}
		if (_clearTInt)
		{
			_clearTInt = false;
			clearTouchInt();
			delay(1);
		}
		return false;
	}
	else
	{
		return false;
	}
}


/**************************************************************************/
/*! (from adafruit)
      Checks if a touch event has occured

      @returns  True is a touch event has occured (reading it via
                touchRead() will clear the interrupt in memory)
*/
/**************************************************************************/
boolean FishinoRA8875SPIClass::touched(void)
{
	if (readReg(RA8875_INTC2) & RA8875_INTCx_TP)
		return true;
	return false;
}

/**************************************************************************/
/*!   Read 10bit internal ADC of RA8875 registers and perform corrections
	  It will return always RAW data
	  Parameters:
	  x:out 0...1024
	  Y:out 0...1024
*/
/**************************************************************************/
void FishinoRA8875SPIClass::readTouchADC(uint16_t *x, uint16_t *y)
{
	uint16_t tx =  readReg(RA8875_TPXH);
	uint16_t ty =  readReg(RA8875_TPYH);
	uint8_t temp = readReg(RA8875_TPXYL);
	tx <<= 2;
	ty <<= 2;
	tx |= temp & 0x03;        // get the bottom x bits
	ty |= (temp >> 2) & 0x03; // get the bottom y bits
#if defined (INVERTETOUCH_X)
	tx = 1024 - tx;
#endif

#if defined (INVERTETOUCH_Y)
	ty = 1024 - ty;
#endif
	//calibrate???
#if defined(TOUCSRCAL_XLOW) && (TOUCSRCAL_XLOW != 0)
	_tsAdcMinX = TOUCSRCAL_XLOW;
	if (tx < TOUCSRCAL_XLOW)
		tx = TOUCSRCAL_XLOW;
#endif

#if defined(TOUCSRCAL_YLOW) && (TOUCSRCAL_YLOW != 0)
	_tsAdcMinY = TOUCSRCAL_YLOW;
	if (ty < TOUCSRCAL_YLOW)
		ty = TOUCSRCAL_YLOW;
#endif

#if defined(TOUCSRCAL_XHIGH) && (TOUCSRCAL_XHIGH != 0)
	_tsAdcMaxX = TOUCSRCAL_XHIGH;
	if (tx > TOUCSRCAL_XHIGH)
		tx = TOUCSRCAL_XHIGH;
#endif

#if defined(TOUCSRCAL_YHIGH) && (TOUCSRCAL_YHIGH != 0)
	_tsAdcMaxY = TOUCSRCAL_YHIGH;
	if (ty > TOUCSRCAL_YHIGH)
		ty = TOUCSRCAL_YHIGH;
#endif
	*x = tx;
	*y = ty;
}

/**************************************************************************/
/*!   Returns 10bit x,y data with TRUE scale (0...1024)
	  Parameters:
	  x:out 0...1024
	  Y:out 0...1024
*/
/**************************************************************************/
void FishinoRA8875SPIClass::touchReadRaw(uint16_t *x, uint16_t *y)
{
	uint16_t tx,ty;
	readTouchADC(&tx,&ty);
#if (defined(TOUCSRCAL_XLOW) && (TOUCSRCAL_XLOW != 0)) || (defined(TOUCSRCAL_XHIGH) && (TOUCSRCAL_XHIGH != 0))
	*x = map(tx,_tsAdcMinX,_tsAdcMaxX,0,1024);
#else
	*x = tx;
#endif
#if (defined(TOUCSRCAL_YLOW) && (TOUCSRCAL_YLOW != 0)) || (defined(TOUCSRCAL_YHIGH) && (TOUCSRCAL_YHIGH != 0))
	*y = map(ty,_tsAdcMinY,_tsAdcMaxY,0,1024);
#else
	*y = ty;
#endif
	clearTouchInt();
}

/**************************************************************************/
/*!   Returns pixel x,y data with SCREEN scale (screen width, screen Height)
	  Parameters:
	  x:out 0...screen width  (pixels)
	  Y:out 0...screen Height (pixels)
*/
/**************************************************************************/
void FishinoRA8875SPIClass::touchReadPixel(uint16_t *x, uint16_t *y)
{
	uint16_t tx,ty;
	readTouchADC(&tx,&ty);
	*x = map(tx,_tsAdcMinX,_tsAdcMaxX,0,_width-1);
	*y = map(ty,_tsAdcMinY,_tsAdcMaxY,0,_height-1);
	clearTouchInt();
}

/**************************************************************************/
/*!   A service utility that detects if system has been calibrated in the past
	  Return true if an old calibration exists
*/
/**************************************************************************/
boolean FishinoRA8875SPIClass::touchCalibrated(void)
{
	uint8_t uncaltetection = 4;
#if defined(TOUCSRCAL_XLOW) && (TOUCSRCAL_XLOW != 0)
	uncaltetection--;
#endif
#if defined(TOUCSRCAL_YLOW) && (TOUCSRCAL_YLOW != 0)
	uncaltetection--;
#endif
#if defined(TOUCSRCAL_XHIGH) && (TOUCSRCAL_XHIGH != 0)
	uncaltetection--;
#endif
#if defined(TOUCSRCAL_YHIGH) && (TOUCSRCAL_YHIGH != 0)
	uncaltetection--;
#endif
	if (uncaltetection < 1)
		return true;
	return false;
}


void FishinoRA8875SPIClass::clearTouchInt(void)
{
	writeReg(RA8875_INTC2, RA8875_INTCx_TP);
}

#endif
