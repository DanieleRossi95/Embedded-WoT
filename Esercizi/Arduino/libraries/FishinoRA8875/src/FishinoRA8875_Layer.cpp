#include "FishinoRA8875SPI.h"

/**************************************************************************/
/*!
		Instruct the RA8875 chip to use 2 layers (if it's possible)
		Return false if not possible, true if possible
		Parameters:
		on:enable multiple layers (2)

*/
/**************************************************************************/
bool FishinoRA8875Class::useLayers(bool on)
{
	bool clearBuffer = false;

	if(on && _maxLayers == 1)
		set8BPP();
	if (on)
	{
		_useMultiLayers = true;
		_DPCRReg |= (1 << 7);
		clearBuffer = true;
	}
	else
	{
		_useMultiLayers = false;
		_DPCRReg &= ~(1 << 7);
	}
	writeReg(RA8875_DPCR,_DPCRReg);
	if (clearBuffer)
	{
		//for some reason if you switch to multilayer the layer 2 has garbage
		//better clear
		writeTo(L2);//switch to layer 2
		clearMemory(false);//clear memory of layer 2
		writeTo(L1);//switch to layer 1
	}
	if(!on && _maxLayers == 1)
		set16BPP();
	return true;//it's possible with current conf
}


/**************************************************************************/
/*!


*/
/**************************************************************************/
void FishinoRA8875Class::layerEffect(enum RA8875boolean efx)
{
	uint8_t	reg = 0b00000000;
	//reg &= ~(0x07);//clear bit 2,1,0
	switch (efx)
	{
			//                       bit 2,1,0 of LTPR0
		case LAYER1: //only layer 1 visible  [000]
			//do nothing
			break;
		case LAYER2: //only layer 2 visible  [001]
			reg |= (1 << 0);
			break;
		case TRANSPARENT: //transparent mode [011]
			reg |= (1 << 0);
			reg |= (1 << 1);
			break;
		case LIGHTEN: //lighten-overlay mode [010]
			reg |= (1 << 1);
			break;
		case OR: //boolean OR mode           [100]
			reg |= (1 << 2);
			break;
		case AND: //boolean AND mode         [101]
			reg |= (1 << 0);
			reg |= (1 << 2);
			break;
		case FLOATING: //floating windows    [110]
			reg |= (1 << 1);
			reg |= (1 << 2);
			break;
		default:
			//do nothing
			break;
	}
	if (_useMultiLayers)
		writeReg(RA8875_LTPR0,reg);
}

/**************************************************************************/
/*!


*/
/**************************************************************************/
void FishinoRA8875Class::layerTransparency(uint8_t layer1,uint8_t layer2)
{
	if (layer1 > 8)
		layer1 = 8;
	if (layer2 > 8)
		layer2 = 8;

	uint8_t res = 0b00000000;//RA8875_LTPR1
	//reg &= ~(0x07);//clear bit 2,1,0
	switch (layer1)
	{
		case 0: //disable layer
			res |= (1 << 3);
			break;
		case 1: //1/8
			res |= (1 << 0);
			res |= (1 << 1);
			res |= (1 << 2);
			break;
		case 2: //1/4
			res |= (1 << 1);
			res |= (1 << 2);
			break;
		case 3: //3/8
			res |= (1 << 0);
			res |= (1 << 2);
			break;
		case 4: //1/2
			res |= (1 << 2);
			break;
		case 5: //5/8
			res |= (1 << 0);
			res |= (1 << 1);
			break;
		case 6: //3/4
			res |= (1 << 1);
			break;
		case 7: //7/8
			res |= (1 << 0);
			break;
	}

	switch (layer2)
	{
		case 0: //disable layer
			res |= (1 << 7);
			break;
		case 1: //1/8
			res |= (1 << 4);
			res |= (1 << 5);
			res |= (1 << 6);
			break;
		case 2: //1/4
			res |= (1 << 5);
			res |= (1 << 6);
			break;
		case 3: //3/8
			res |= (1 << 4);
			res |= (1 << 6);
			break;
		case 4: //1/2
			res |= (1 << 6);
			break;
		case 5: //5/8
			res |= (1 << 4);
			res |= (1 << 5);
			break;
		case 6: //3/4
			res |= (1 << 5);
			break;
		case 7: //7/8
			res |= (1 << 4);
			break;
	}
	if (_useMultiLayers)
		writeReg(RA8875_LTPR1,res);
}
