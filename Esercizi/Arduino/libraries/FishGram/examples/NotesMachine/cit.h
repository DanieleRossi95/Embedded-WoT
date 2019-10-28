#ifndef __CIT_H
#define __CIT_H

///////////////////////////////////////////////////////////////////////////////////
// Citation server handling
//
// server MUST answer to a query with this format:
//
// id&len&msg
// where:
// id		is the citation id number (from 0 to number of available)
// len		is the length of following text
// msg		is the text of the citation
//
// see the .php file(s) inside sketch's folder
//
// if the query string has an '&id=nnn' content, the citation with
// given id is retrieved (useful to print it on serial port without
// having to store it locally); if no id is given, a random one is retrieved
// The id of the last retrieved citation is stored, so we can ask for it again
//
///////////////////////////////////////////////////////////////////////////////////

#include <Fishino.h>

class Cit : public FishinoClient
{
	private:
	
		// citation server host
		const __FlashStringHelper *_host;
	
		// citation server path
		const __FlashStringHelper *_path;
	
		// last citation id
		uint16_t _lastId;
		
		// message length
		uint16_t _msgLen;
		
		// debug helper
		int readChar(void);
		
		// skip http headers
		bool skipHeaders(void);
		
		// read a number from client
		bool readNum(uint16_t &num);
	
	protected:
	
	public:
	
		// constructor
		Cit();
		
		// destructor
		~Cit();
		
		// set citation server host and path

		Cit &setHost(const __FlashStringHelper *host);
		Cit &setPath(const __FlashStringHelper *path);
		
		// query for a new citation, storing message id and length
		// if id is given, use it to fetch the citation, otherwise get a random one
		bool query(uint16_t id = (uint16_t)-1);
		
		uint16_t getId(void) { return _lastId; }
		uint16_t getLen(void) { return _msgLen; }
};

#endif

