/*  MCP23017 library for Arduino
    Copyright (C) 2009 David Pye    <davidmpye@gmail.com  
	Copyright (C) 2012 Kasper Skårhøj <kasperskaarhoj@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "IOExpanderMCP23017.h"

// IMPORTANT on byte order:
// Note on Writing words: The MSB is for GPA7-0 and the LSB is for GPB7-0
// Pinnumbers 0-7 = GPB0-7, 8-15 = GPA0-7
// This comes across as slightly un-intuitive when programming

IOExpanderMCP23017::IOExpanderMCP23017() {}

void IOExpanderMCP23017::begin(int i2cAddress) {
	_i2cAddress = (MCP23017_I2C_BASE_ADDRESS >>1) | (i2cAddress & 0x07);

	//Default state is 0 for our pins
	_GPIO = 0x0000;
	_IODIR = 0x0000;
	_GPPU = 0x0000;
	_IOINT = 0x0000;
}

bool IOExpanderMCP23017::init()	{
	// If this value is true (return value of this function), we assume the board actually responded and is "online"
	bool retVal = readRegister(0x00)==65535;
	
	//Set the IOCON.BANK bit to 0 to enable sequential addressing
	//IOCON 'default' address is 0x05, but will
	//change to our definition of IOCON once this write completes.
	writeRegister(0x05, (byte)0x0);
	
	//Our pins default to being outputs by default.
	writeRegister(MCP23017_IODIR, (word)_IODIR);
	//Disable INTA and INTB by default.
	writeRegister(MCP23017_INTEN, (word)0x0000);
	//Reset DEFVAL register
	writeRegister(MCP23017_DEFVAL, (word)0x0000);
	//Reset INTCON register (int generated on any change)
	writeRegister(MCP23017_INTCON, (word)0x0000);	
	
	return retVal;
}

void IOExpanderMCP23017::pinMode(int pin, int mode) {
	//Arduino defines OUTPUT as 1, but
	//MCP23017 uses OUTPUT as 0. (input is 0x1)
	mode = !mode;
	if (mode) _IODIR |= 1 << pin;
	else _IODIR &= ~(1 << pin);
	writeRegister(MCP23017_IODIR, (word)_IODIR);
}

int IOExpanderMCP23017::digitalRead(int pin) {
	_GPIO = readRegister(MCP23017_GPIO);
	if ( _GPIO & (1 << pin)) return HIGH;
	else return LOW;
}

void IOExpanderMCP23017::digitalWrite(int pin, int val) {
	//If this pin is an INPUT pin, a write here will
	//enable the internal pullup
	//otherwise, it will set the OUTPUT voltage
	//as appropriate.
	bool isOutput = !(_IODIR & 1<<pin);

	if (isOutput) {
		//This is an output pin so just write the value
		if (val) _GPIO |= 1 << pin;
		else _GPIO &= ~(1 << pin);
		writeRegister(MCP23017_GPIO, (word)_GPIO);
	}
	else {
		//This is an input pin, so we need to enable the pullup
		if (val) _GPPU |= 1 << pin;
		else _GPPU &= ~(1 << pin);
		writeRegister(MCP23017_GPPU, (word)_GPPU);
	}
}

word IOExpanderMCP23017::digitalWordRead() {
	_GPIO = readRegister(MCP23017_GPIO);
	return _GPIO;
}
void IOExpanderMCP23017::digitalWordWrite(word w) {
	_GPIO = w;
	writeRegister(MCP23017_GPIO, (word)_GPIO);
}

void IOExpanderMCP23017::inputPolarityMask(word mask) {
	writeRegister(MCP23017_IPOL, mask);
}

void IOExpanderMCP23017::inputOutputMask(word mask) {
	_IODIR = mask;
	writeRegister(MCP23017_IODIR, (word)_IODIR);
}

void IOExpanderMCP23017::internalPullupMask(word mask) {
	_GPPU = mask;
	writeRegister(MCP23017_GPPU, (word)_GPPU);
}

void IOExpanderMCP23017::pinEnableINT(int pin) {
	//Set selected bit to 1 to enable int
	_IOINT |= 1 << pin;
	writeRegister(MCP23017_INTEN, (word)_IOINT);
}

void IOExpanderMCP23017::pinDisableINT(int pin) {
	//Set selected bit to 0 to disable int
	_IOINT &= ~(1 << pin);
	writeRegister(MCP23017_INTEN, (word)_IOINT);
}

word IOExpanderMCP23017::pinINTWordRead() {
	return readRegister(MCP23017_INTF);
}

//PRIVATE
void IOExpanderMCP23017::writeRegister(int regAddress, byte data) {
	Wire.beginTransmission(_i2cAddress);
	Wire.write(regAddress);
	Wire.write(data);
	Wire.endTransmission();
}

void IOExpanderMCP23017::writeRegister(int regAddress, word data) {
	Wire.beginTransmission(_i2cAddress);
	Wire.write(regAddress);
	Wire.write(highByte(data));
	Wire.write(lowByte(data));
	Wire.endTransmission();
}

word IOExpanderMCP23017::readRegister(int regAddress) {
	word returnword = 0x00;
	Wire.beginTransmission(_i2cAddress);
	Wire.write(regAddress);
	Wire.endTransmission();
	Wire.requestFrom((int)_i2cAddress, 2);
    
	int c=0;
	//Wait for our 2 bytes to become available
	while (Wire.available()) {
	  //high byte
	  if (c==0)   { returnword = Wire.read() << 8; }
	  //low byte
	  if (c==1)   { returnword |= Wire.read(); }
	  c++;
	}
    
	return returnword;
}