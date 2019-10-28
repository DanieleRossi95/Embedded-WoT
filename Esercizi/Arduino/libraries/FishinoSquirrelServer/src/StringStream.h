#ifndef __STRINGSTREAM_H
#define __STRINGSTREAM_H

#include <Arduino.h>

class StringStream : public Stream
{
	private:
	
		String _s;

	protected:

	public:

		// constructor
		StringStream() ;

		// destructor
		~StringStream() ;
		
		// get current string
		String get() ;
		
		// add another string
		void  add(const char *s2) { _s += s2; }
		void  add(String const &s2) { _s += s2; }
		
		// get current length
		int  getCount() const { return _s.length(); }
		
		// limited streaming support
		size_t write(uint8_t *buf, size_t len) ;
		size_t write(uint8_t b) ;
		int  available(void) { return 0; }
		int  read(void) { return -1; }
		int  peek(void) { return -1; }
		void  flush(void) {};
};

#endif
