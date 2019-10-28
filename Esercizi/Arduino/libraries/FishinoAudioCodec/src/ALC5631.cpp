//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								ALC5631.cpp											//
//						ALC5631 Audio Codec driver									//
//					Copyright (c) 2017 Massimo Del Fedele							//
//																					//
//	Redistribution and use in source and binary forms, with or without				//
//	modification, are permitted provided that the following conditions are met:		//
//																					//
//	- Redistributions of source code must retain the above copyright notice,		//
//	  this list of conditions and the following disclaimer.							//
//	- Redistributions in binary form must reproduce the above copyright notice,		//
//	  this list of conditions and the following disclaimer in the documentation		//
//	  and/or other materials provided with the distribution.						//
//																					//	
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"		//
//	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE		//
//	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE		//
//	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE		//
//	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR				//
//	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF			//
//	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS		//
//	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN			//
//	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)			//
//	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE		//
//	POSSIBILITY OF SUCH DAMAGE.														//
//																					//
//	VERSION 1.0.0	2016		INITIAL VERSION										//
//  Version 6.0.2	June 2017	Some small fixes									//
//																					//
//////////////////////////////////////////////////////////////////////////////////////

#define OPT_BOARD_INTERNAL
#include "FishinoAudioCodec.h"

#include <Wire.h>

#include "ALC5631Defs.h"

#define DEBUG_LEVEL_ERROR
#include <FishinoDebug.h>

#ifndef ARDUINO_STREAMING
#define ARDUINO_STREAMING
template<class T> inline Print &operator <<(Print &stream, T arg)
{
	stream.print(arg);
	return stream;
}
#endif

// codec ports
#ifndef _CODEC_CHIP
#error "No codec on board"
#elif _CODEC_CHIP != _ALC5631_CODEC
#error "Unsupported codec"
#endif

// ALC I2C address
#define ALC5631_ADDR	0x1A

static const uint32_t pwrElements[] = {
	AudioCodec::LEFTSPEAKER, AudioCodec::RIGHTSPEAKER, AudioCodec::MONOSPEAKER,
	AudioCodec::LEFTHEADPHONE, AudioCodec::RIGHTHEADPHONE,
	AudioCodec::LEFTAUXOUT, AudioCodec::RIGHTAUXOUT, AudioCodec::MONOAUXOUT,
	AudioCodec::LEFTMIC, AudioCodec::RIGHTMIC,
	AudioCodec::LEFTAUXIN, AudioCodec::RIGHTAUXIN, AudioCodec::MONOAUXIN
};
static const uint8_t numPwrElements = sizeof(pwrElements) / sizeof(uint32_t);

static const uint32_t pwr1[] = {
	/* LEFTSPEAKER    */ RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_CLASS_D | RT5631_PWR_DAC_L_CLK | RT5631_PWR_DAC_REF | RT5631_PWR_DAC_L_TO_MIXER,
	/* RIGHTSPEAKER   */ RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_CLASS_D | RT5631_PWR_DAC_R_CLK | RT5631_PWR_DAC_REF | RT5631_PWR_DAC_R_TO_MIXER,
	/* MONOSPEAKER    */ RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_DAC_L_CLK | RT5631_PWR_DAC_R_CLK | RT5631_PWR_DAC_REF | RT5631_PWR_DAC_L_TO_MIXER | RT5631_PWR_DAC_R_TO_MIXER,
	/* LEFTHEADPHONE  */ RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_DAC_L_CLK | RT5631_PWR_DAC_REF | RT5631_PWR_DAC_L_TO_MIXER,
	/* RIGHTHEADPHONE */ RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_DAC_R_CLK | RT5631_PWR_DAC_REF | RT5631_PWR_DAC_R_TO_MIXER,
	/* LEFTAUXOUT     */ RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_DAC_L_CLK | RT5631_PWR_DAC_REF | RT5631_PWR_DAC_L_TO_MIXER,
	/* RIGHTAUXOUT    */ RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_DAC_R_CLK | RT5631_PWR_DAC_REF | RT5631_PWR_DAC_R_TO_MIXER,
	/* MONOAUXOUT     */ RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_DAC_L_CLK | RT5631_PWR_DAC_REF | RT5631_PWR_DAC_L_TO_MIXER | RT5631_PWR_DAC_R_TO_MIXER,
	/* LEFTMIC        */ RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_ADC_L_CLK,
	/* RIGHTMIC       */ RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_ADC_R_CLK,
	/* LEFTAUXIN      */ RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_ADC_L_CLK,
	/* RIGHTAUXIN     */ RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_ADC_R_CLK,
	/* MONOAUXIN      */ RT5631_PWR_MAIN_I2S_EN | RT5631_PWR_ADC_L_CLK | RT5631_PWR_ADC_R_CLK
};

static const uint32_t pwr2[] = {
	/* LEFTSPEAKER    */ RT5631_PWR_OUTMIXER_L | RT5631_PWR_SPKMIXER_L,
	/* RIGHTSPEAKER   */ RT5631_PWR_OUTMIXER_R | RT5631_PWR_SPKMIXER_R,
	/* MONOSPEAKER    */ RT5631_PWR_OUTMIXER_L | RT5631_PWR_OUTMIXER_R,
	/* LEFTHEADPHONE  */ RT5631_PWR_OUTMIXER_L,
	/* RIGHTHEADPHONE */ RT5631_PWR_OUTMIXER_R,
	/* LEFTAUXOUT     */ RT5631_PWR_OUTMIXER_L,
	/* RIGHTAUXOUT    */ RT5631_PWR_OUTMIXER_R,
	/* MONOAUXOUT     */ RT5631_PWR_OUTMIXER_L | RT5631_PWR_OUTMIXER_R,
	/* LEFTMIC        */ RT5631_PWR_RECMIXER_L | RT5631_PWR_MIC1_BOOT_GAIN | RT5631_PWR_MICBIAS1_VOL,
	/* RIGHTMIC       */ RT5631_PWR_RECMIXER_R | RT5631_PWR_MIC2_BOOT_GAIN | RT5631_PWR_MICBIAS2_VOL,
	/* LEFTAUXIN      */ RT5631_PWR_RECMIXER_L,
	/* RIGHTAUXIN     */ RT5631_PWR_RECMIXER_R,
	/* MONOAUXIN      */ RT5631_PWR_RECMIXER_L | RT5631_PWR_RECMIXER_R
};

static const uint32_t pwr3[] = {
	/* LEFTSPEAKER    */ 0,
	/* RIGHTSPEAKER   */ 0,
	/* MONOSPEAKER    */ RT5631_PWR_MONOMIXER | RT5631_PWR_MONO_DEPOP_DIS | RT5631_PWR_MONO_AMP_EN,
	/* LEFTHEADPHONE  */ RT5631_PWR_CHARGE_PUMP | RT5631_PWR_HP_L_AMP | RT5631_PWR_HP_DEPOP_DIS | RT5631_PWR_HP_AMP_DRIVING,
	/* RIGHTHEADPHONE */ RT5631_PWR_CHARGE_PUMP | RT5631_PWR_HP_R_AMP | RT5631_PWR_HP_DEPOP_DIS | RT5631_PWR_HP_AMP_DRIVING,
	/* LEFTAUXOUT     */ RT5631_PWR_AXO1MIXER | RT5631_PWR_AXO2MIXER,
	/* RIGHTAUXOUT    */ RT5631_PWR_AXO1MIXER | RT5631_PWR_AXO2MIXER,
	/* MONOAUXOUT     */ RT5631_PWR_AXO1MIXER | RT5631_PWR_AXO2MIXER,
	/* LEFTMIC        */ 0,
	/* RIGHTMIC       */ 0,
	/* LEFTAUXIN      */ 0,
	/* RIGHTAUXIN     */ 0,
	/* MONOAUXIN      */ 0
};

static const uint32_t pwr4[] = {
	/* LEFTSPEAKER    */ RT5631_PWR_SPK_L_VOL | RT5631_PWR_LOUT_VOL,
	/* RIGHTSPEAKER   */ RT5631_PWR_SPK_R_VOL | RT5631_PWR_ROUT_VOL,
	/* MONOSPEAKER    */ RT5631_PWR_LOUT_VOL | RT5631_PWR_ROUT_VOL,
	/* LEFTHEADPHONE  */ RT5631_PWR_HP_L_OUT_VOL | RT5631_PWR_LOUT_VOL,
	/* RIGHTHEADPHONE */ RT5631_PWR_HP_R_OUT_VOL | RT5631_PWR_ROUT_VOL,
	/* LEFTAUXOUT     */ RT5631_PWR_LOUT_VOL,
	/* RIGHTAUXOUT    */ RT5631_PWR_ROUT_VOL,
	/* MONOAUXOUT     */ RT5631_PWR_LOUT_VOL | RT5631_PWR_ROUT_VOL,
	/* LEFTMIC        */ 0,
	/* RIGHTMIC       */ 0,
	/* LEFTAUXIN      */ RT5631_PWR_AXIL_IN_VOL,
	/* RIGHTAUXIN     */ RT5631_PWR_AXIR_IN_VOL,
	/* MONOAUXIN      */ RT5631_PWR_MONO_IN_N_VOL | RT5631_PWR_MONO_IN_P_VOL
};

/*
// registers used for mixer paths
static const uint16_t pathRegs[] = {
	RT5631_SPK_OUT_VOL, 
}
RT5631_HP_OUT_VOL
RT5631_MONO_AXO_1_2_VOL
RT5631_STEREO_DAC_VOL_1
RT5631_MIC_CTRL_1
RT5631_STEREO_DAC_VOL_2
RT5631_ADC_CTRL_1
RT5631_ADC_REC_MIXER
RT5631_ADC_CTRL_2
RT5631_VDAC_DIG_VOL
RT5631_OUTMIXER_L_CTRL
RT5631_OUTMIXER_R_CTRL
RT5631_AXO1MIXER_CTRL
RT5631_AXO2MIXER_CTRL
RT5631_MIC_CTRL_2
RT5631_MONO_INPUT_VOL
RT5631_SPK_MIXER_CTRL
RT5631_SPK_MONO_OUT_CTRL
RT5631_SPK_MONO_HP_OUT_CTRL
RT5631_SDP_CTRL
RT5631_MONO_SDP_CTRL
RT5631_STEREO_AD_DA_CLK_CTRL
*/

// write an ALC register
bool ALC5631Class::writeReg(uint8_t reg, uint16_t mask, uint16_t val)
{
	uint8_t res;

	val &= mask;
	val |= (readReg(reg) & ~mask);
	ALC_I2C.beginTransmission(ALC5631_ADDR);
	ALC_I2C.write(reg);
	ALC_I2C.write((uint8_t)(val >> 8));
	ALC_I2C.write((uint8_t)val);
	
	res = ALC_I2C.endTransmission();
	if(res)
		DEBUG_ERROR("writeReg transmission error : %d\n", res);
	return !res;
}

// read an ALC register
uint16_t ALC5631Class::readReg(uint8_t reg)
{
	uint8_t res;

	ALC_I2C.beginTransmission(ALC5631_ADDR);
	ALC_I2C.write(reg);
	res = ALC_I2C.endTransmission();
	if(res)
	{
		DEBUG_ERROR("readReg transmission error : %d\n", res);
		return 0;
	}
	res = ALC_I2C.requestFrom(ALC5631_ADDR, 2);
	if(res != 2)
	{
		DEBUG_ERROR("readReg requestFrom returned : %d\n", res);
		return 0;
	}
	uint16_t resVal;
	resVal = ALC_I2C.read();
	resVal <<= 8;
	resVal |= ALC_I2C.read();

	return resVal;
}

// write 2nd level register
bool ALC5631Class::write2Reg(uint8_t reg, uint16_t mask, uint16_t val)
{
	writeReg(RT5631_INDEX_ADD, 0xffff, reg);
	return writeReg(RT5631_INDEX_DATA, mask, val);
}

// read 2nd level regiser
uint16_t ALC5631Class::read2Reg(uint8_t reg)
{
	writeReg(RT5631_INDEX_ADD, 0xffff, reg);
	return readReg(RT5631_INDEX_DATA);
}

/////////////////////////////////////////////////////////////////////////////

// clear all mixer paths
// used internally before setting new one(s)
void ALC5631Class::clearPaths(void)
{
	writeReg(RT5631_SPK_OUT_VOL,			0b1100000011000000, 0b1000000010000000);
	writeReg(RT5631_HP_OUT_VOL,				0b1100000011000000, 0b1000000010000000);
	writeReg(RT5631_MONO_AXO_1_2_VOL,		0b1110000011000000, 0b1010000010000000);
	writeReg(RT5631_STEREO_DAC_VOL_1,		0b1000000010000000, 0b1000000010000000);
	writeReg(RT5631_ADC_CTRL_1,				0b1000000010000000, 0b1000000010000000);
	writeReg(RT5631_ADC_REC_MIXER,			0b1111000011110000, 0b1111000011110000);
	writeReg(RT5631_OUTMIXER_L_CTRL,		0b1111111100000000, 0b1111111100000000);
	writeReg(RT5631_OUTMIXER_R_CTRL,		0b1111111100000000, 0b1111111100000000);
	writeReg(RT5631_AXO1MIXER_CTRL,			0b1000100011000000, 0b1000100011000000);
	writeReg(RT5631_AXO2MIXER_CTRL,			0b1000100011000000, 0b1000100011000000);
	writeReg(RT5631_SPK_MIXER_CTRL,			0b1111000011110000, 0b1111000011110000);
	writeReg(RT5631_SPK_MONO_OUT_CTRL,		0b1111110000000000, 0b1111110000000000);
	writeReg(RT5631_SPK_MONO_HP_OUT_CTRL,	0b1100110011001100, 0b0000000000000000);
}

/////////////////////////////////////////////////////////////////////////////

// reset the controller
bool ALC5631Class::reset(void)
{
	// reset the controller and setup default audio formats
	if(	!writeReg(RT5631_RESET, 0xffff, 0x00))
		return false;
		
	writeReg(RT5631_SDP_CTRL,				0b1000111111111111, 0b1000000000001000);
	writeReg(RT5631_STEREO_AD_DA_CLK_CTRL,	0b1111111110000000, 0b0000010100000000);
//	writeReg(RT5631_STEREO_AD_DA_CLK_CTRL,	0b1111111110000000, 0b0000000000000000);

	// enable protections
	writeReg(RT5631_MISC_CTRL, 0b1110110011110000, 0b1110110010000000);

	_power = 0;

	return true;
}

// codec power management
bool ALC5631Class::power(uint32_t control)
{
	if(_debugStream)
		*_debugStream << "Setting power to " << control << "\n";
	uint16_t mask1, mask2, mask3, mask4;
	uint16_t val1, val2, val3, val4;

	mask1 =
		RT5631_PWR_MAIN_I2S_EN		|
		RT5631_PWR_CLASS_D			|
		RT5631_PWR_ADC_L_CLK		|
		RT5631_PWR_ADC_R_CLK		|
		RT5631_PWR_DAC_L_CLK		|
		RT5631_PWR_DAC_R_CLK		|
		RT5631_PWR_DAC_REF			|
		RT5631_PWR_DAC_L_TO_MIXER	|
		RT5631_PWR_DAC_R_TO_MIXER
	;

	mask2 =
		RT5631_PWR_OUTMIXER_L		|
		RT5631_PWR_OUTMIXER_R		|
		RT5631_PWR_SPKMIXER_L		|
		RT5631_PWR_SPKMIXER_R		|
		RT5631_PWR_RECMIXER_L		|
		RT5631_PWR_RECMIXER_R		|
		RT5631_PWR_MIC1_BOOT_GAIN	|
		RT5631_PWR_MIC2_BOOT_GAIN	|
		RT5631_PWR_MICBIAS1_VOL		|
		RT5631_PWR_MICBIAS2_VOL		|
		RT5631_PWR_PLL1				|
		RT5631_PWR_PLL2
	;

	mask3 =
		RT5631_PWR_VREF				|
		RT5631_PWR_FAST_VREF_CTRL	|
		RT5631_PWR_MAIN_BIAS		|
		RT5631_PWR_AXO1MIXER		|
		RT5631_PWR_AXO2MIXER		|
		RT5631_PWR_MONOMIXER		|
		RT5631_PWR_MONO_DEPOP_DIS	|
		RT5631_PWR_MONO_AMP_EN		|
		RT5631_PWR_CHARGE_PUMP		|
		RT5631_PWR_HP_L_AMP			|
		RT5631_PWR_HP_R_AMP			|
		RT5631_PWR_HP_DEPOP_DIS		|
		RT5631_PWR_HP_AMP_DRIVING
	;

	mask4 =
		RT5631_PWR_SPK_L_VOL		|
		RT5631_PWR_SPK_R_VOL		|
		RT5631_PWR_LOUT_VOL			|
		RT5631_PWR_ROUT_VOL			|
		RT5631_PWR_HP_L_OUT_VOL		|
		RT5631_PWR_HP_R_OUT_VOL		|
		RT5631_PWR_AXIL_IN_VOL		|
		RT5631_PWR_AXIR_IN_VOL		|
		RT5631_PWR_MONO_IN_P_VOL	|
		RT5631_PWR_MONO_IN_N_VOL
	;

	// if we want to power it off do it
	if(!control)
	{
		writeReg(RT5631_PWR_MANAG_ADD1, mask1, 0);
		writeReg(RT5631_PWR_MANAG_ADD2, mask2, 0);
		writeReg(RT5631_PWR_MANAG_ADD3, mask3, 0);
		writeReg(RT5631_PWR_MANAG_ADD4, mask4, 0);
		
		// remember power state
		_power = 0;
		
		return true;
	}
	
	// if nothing was powered on and now something is
	// we must at first power on VREF and MAIN BIAS
	// (those are on on every thing)
	// we shall also be careful, as turning them on together
	// can hang the codec. So, a small delay between
	if(!_power)
	{
		// turn on VREF and MAIN_BIAS at first
		writeReg(RT5631_PWR_MANAG_ADD3, RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS, RT5631_PWR_VREF | RT5631_PWR_MAIN_BIAS);
		
		// wait some 100 milliseconds to give codec to settle
		delay(100);
		
		// now enable fast VREF too
		writeReg(RT5631_PWR_MANAG_ADD3, RT5631_PWR_FAST_VREF_CTRL, RT5631_PWR_FAST_VREF_CTRL);
	}
	
	// remove VREF stuffs from masks, we're done with it
	mask3 &= ~(RT5631_PWR_VREF | RT5631_PWR_FAST_VREF_CTRL | RT5631_PWR_MAIN_BIAS);

	// start with no elements powered on
	val1 = val2 = val3 = val4 = 0;
	
	// now select stuffs that shall be turned on basing on control parameter
	for(uint8_t iElem = 0; iElem < numPwrElements; iElem++)
	{
		if(control & pwrElements[iElem])
		{
			val1 |= pwr1[iElem];
			val2 |= pwr2[iElem];
			val3 |= pwr3[iElem];
			val4 |= pwr4[iElem];
		}
	}
	writeReg(RT5631_PWR_MANAG_ADD1, mask1, val1);
	writeReg(RT5631_PWR_MANAG_ADD2, mask2, val2);
	writeReg(RT5631_PWR_MANAG_ADD3, mask3, val3);
	writeReg(RT5631_PWR_MANAG_ADD4, mask4, val4);
	
	if(_debugStream)
	{
		*_debugStream << "RT5631_PWR_MANAG_ADD1 : ";
		_debugStream->println(readReg(RT5631_PWR_MANAG_ADD1), HEX);
		*_debugStream << "RT5631_PWR_MANAG_ADD2 : ";
		_debugStream->println(readReg(RT5631_PWR_MANAG_ADD2), HEX);
		*_debugStream << "RT5631_PWR_MANAG_ADD3 : ";
		_debugStream->println(readReg(RT5631_PWR_MANAG_ADD3), HEX);
		*_debugStream << "RT5631_PWR_MANAG_ADD4 : ";
		_debugStream->println(readReg(RT5631_PWR_MANAG_ADD4), HEX);
	}
	
	_power = control;
	
	return true;
}

// codec path management
bool ALC5631Class::path(uint32_t pth)
{
	uint16_t val;
	
	// clear any previous path
	clearPaths();
	
	// do not mix paths by now, just one of them
	switch(pth)
	{
		case DAC2STEREOSPEAKERS :
			if(_debugStream)
				*_debugStream << "Setting DAC2STEREOSPEAKERS path\n";
			writeReg(RT5631_SPK_OUT_VOL,			0b1100000011000000, 0b0100000001000000);
			writeReg(RT5631_HP_OUT_VOL,				0b1100000011000000, 0b1000000010000000);
			writeReg(RT5631_MONO_AXO_1_2_VOL,		0b1110000011000000, 0b1010000010000000);
			writeReg(RT5631_STEREO_DAC_VOL_1,		0b1000000010000000, 0b0000000000000000);
			writeReg(RT5631_ADC_CTRL_1,				0b1000000010000000, 0b1000000010000000);
			writeReg(RT5631_ADC_REC_MIXER,			0b1111000011110000, 0b1111000011110000);
			writeReg(RT5631_OUTMIXER_L_CTRL,		0b1111111100000000, 0b1111111100000000);
			writeReg(RT5631_OUTMIXER_R_CTRL,		0b1111111100000000, 0b1111111100000000);
			writeReg(RT5631_AXO1MIXER_CTRL,			0b1000100011000000, 0b1000100011000000);
			writeReg(RT5631_AXO2MIXER_CTRL,			0b1000100011000000, 0b1000100011000000);
			writeReg(RT5631_SPK_MIXER_CTRL,			0b1111000011110000, 0b1101000011010000);
			writeReg(RT5631_SPK_MONO_OUT_CTRL,		0b1111110000000000, 0b0110110000000000);
			writeReg(RT5631_SPK_MONO_HP_OUT_CTRL,	0b1100110011000000, 0b0000000000000000);
			break;
			
		case DAC2MONOSPEAKER :
			return false;
			break;
			
		case DAC2STEREOHEADPHONES :
			return false;
			break;
			
		case DAC2STEREOAUXOUT :
			return false;
			break;
			
		case DAC2MONOAUXOUT :
			return false;
			break;
			
		// audio recording paths (can be mixed between them)
		case LEFTMIC2LEFTADC :
		case RIGHTMIC2RIGHTADC :
		case STEREOMICS2ADC :
		{
			writeReg(RT5631_MIC_CTRL_1,			0b1000000010000000, 0b0000000000000000);
			writeReg(RT5631_ADC_CTRL_1,			0b1000000010000000, 0b0000000000000000);
			val = 0b1111111111111111;
			if(pth & LEFTMIC2LEFTADC)
				val &= ~0b0100000000000000;
			if(pth & RIGHTMIC2RIGHTADC)
				val &= ~0b0000000001000000;
			writeReg(RT5631_ADC_REC_MIXER,		0b1111000011110000, val);
		}
			break;

		case STEREOAUXIN2ADC :
			return false;
			break;
			
		case MONOAUXIN2ADC :
			return false;
			break;
			
		default:
			return false;
	}
	return true;
}

// volume management
bool ALC5631Class::volume(uint32_t what, double vol, double bal)
{
	// trim volume and balance values
	if(vol < 0)
		vol = 0;
	else if(vol > 1)
		vol = 1;
	if(bal < -1)
		bal = -1;
	else if(bal > 1)
		bal = 1;
	
	// balance is done by lowering the opposite channel
	// (maybe there's a better way....)
	double left = vol;
	double right = vol;
	if(bal < 0)
		right *= 1 + bal;
	if(bal > 0)
		left *= 1 - bal;
	
	switch(what)
	{
		case SPEAKERSVOL :
		{
		
			uint32_t leftVol = 0x1f;
			uint32_t rightVol = 0x1f;
			leftVol *= (1 - left);
			rightVol *= (1 - right);
			writeReg(
				RT5631_SPK_OUT_VOL,
				(RT5631_VOL_MASK	<< RT5631_L_VOL_SHIFT)	| (RT5631_VOL_MASK	<< RT5631_R_VOL_SHIFT),
				(leftVol			<< RT5631_L_VOL_SHIFT)	| (rightVol			<< RT5631_R_VOL_SHIFT)
			);
		}
			break;
			
		case HEADPHONESVOL :
			break;
			
		case OUTPUTVOL :
			break;
			
		case OTPUTTOAXO1VOL :
			break;
			
		case OUTPUTTOAXO2VOL :
		
		case MICSINPUTGAIN :
		{
			uint16_t mic1 = (uint16_t)(8 * left);
			uint16_t mic2 = (uint16_t)(8 * right);
			if(_debugStream)
				*_debugStream << "MIC1:" << mic1 << "  MIC2:" << mic2 << "\n";
			writeReg(RT5631_MIC_CTRL_2, 0b1111111100000000, (mic1 << 12) | (mic2 << 8));
		}
			break;
			
		case AUXINPUTVOL :
			break;
		
		case DACDIGITALPREBOOST :
			break;
			
		case DACDIGITALVOL :
			break;
		
		case ADCDIGITALPREBOOST :
		{
			uint16_t val = (uint16_t)(0x13 * vol);
			writeReg(RT5631_ADC_CTRL_1, 0b0000000000011111, val);
		}
			break;
			
		case ADCDIGITALVOL :
		{
			uint16_t l = (uint16_t)(0xff * (1 - left));
			uint16_t r = (uint16_t)(0xff * (1 - right));
			writeReg(RT5631_ADC_CTRL_2, 0b1111111111111111, (l << 8) | r);
		}
			break;
			
		default:
			return false;
			break;
	}
	return true;
}

// constructor
ALC5631Class::ALC5631Class() : AudioCodec()
{
	// reset the codec
	reset();
	
	// power it off
	power(0);
}

// destructor
ALC5631Class::~ALC5631Class()
{
	// reset the codec
	reset();
	
	// power it off
	power(0);
}

// debug helper -- prints codec registers
void ALC5631Class::printReg(Stream &s, uint8_t reg)
{
	uint32_t bit = 1 << 15;
	uint16_t val = readReg(reg);
	if(reg < 0x10)
		s.print('0');
	s.print(reg, HEX);
	s.print(" : ");
	while(bit)
	{
		s.print((val & bit) ? '1' : '0');
		bit >>= 1;
	}
	s.println();
}

void ALC5631Class::printRegs(Stream &s)
{
	s.println("---------REGISTERS-------------");
	printReg(s, 0x00);
	printReg(s, 0x02);
	printReg(s, 0x04);
	printReg(s, 0x06);
	printReg(s, 0x0A);
	printReg(s, 0x0C);
	printReg(s, 0x0E);
	printReg(s, 0x10);
	printReg(s, 0x12);
	printReg(s, 0x14);
	printReg(s, 0x16);
	printReg(s, 0x18);
	printReg(s, 0x1A);
	printReg(s, 0x1C);
	printReg(s, 0x1E);
	printReg(s, 0x20);
	printReg(s, 0x22);
	printReg(s, 0x24);
	printReg(s, 0x26);
	printReg(s, 0x28);
	printReg(s, 0x2A);
	printReg(s, 0x2C);
	printReg(s, 0x34);
	printReg(s, 0x36);
	printReg(s, 0x38);
	printReg(s, 0x3A);
	printReg(s, 0x3B);
	printReg(s, 0x3C);
	printReg(s, 0x3E);
	printReg(s, 0x40);
	printReg(s, 0x42);
	printReg(s, 0x44);
	printReg(s, 0x48);
	printReg(s, 0x4A);
	printReg(s, 0x4C);
	printReg(s, 0x52);
	printReg(s, 0x54);
	printReg(s, 0x56);
	printReg(s, 0x5A);
	printReg(s, 0x5C);
	printReg(s, 0x64);
	printReg(s, 0x65);
	printReg(s, 0x66);
	printReg(s, 0x68);
	printReg(s, 0x6A);
	printReg(s, 0x6C);
	printReg(s, 0x6E);
	printReg(s, 0x7A);
	printReg(s, 0x7C);
	printReg(s, 0x7E);
	s.println("-------------------------------");
}

ALC5631Class &__ALC5631()
{
	static ALC5631Class alc;
	static bool I2CInitialized = false;
	
	if(!I2CInitialized)
	{
		Wire.begin();
		Wire.setClock(100000);
		I2CInitialized = true;
	}
	return alc;
}

