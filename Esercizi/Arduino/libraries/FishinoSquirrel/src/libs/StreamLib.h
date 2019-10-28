#ifndef __FISHINOSQUIRREL_STREAMLIB_H
#define __FISHINOSQUIRREL_STREAMLIB_H

//#include <Arduino.h>

#include "../Squirrel.h"

class SqStream : public Stream
{
	private:
		
	protected:

	public:

		// destructor
		virtual  ~SqStream() {};

		virtual void  flush(void) = 0;
		virtual size_t  write(char c) = 0;
		virtual size_t  write(const char *buf, size_t len) = 0;
		virtual int  read(void) = 0;
		virtual size_t  read(char *buf, size_t maxLen) = 0;
		virtual int  available(void) = 0;
		virtual int  peek(void) = 0;
		virtual size_t  tell(void) = 0;
		virtual size_t  length(void) = 0;
		virtual bool  eof(void) = 0;
		virtual bool  opened(void) { return false; }

		virtual size_t  write(uint8_t b) { return write((char)b); }

/*
		virtual bool print(const char *s)
			{ return write(s, strlen(s)) == strlen(s); }
		virtual bool println(const char *s)
			{ size_t res = write(s, strlen(s)); res += write("\n", 1); return res == strlen(s) + 1; }
*/
	    size_t  print(const char* s)
	    	{ return Stream::print(s); }
	    size_t  print(char c)
	    	{ return Stream::print(c); }
	    size_t  print(int i, int mode)
	    	{ return Stream::print(i, mode); }
	    size_t  print(int i)
	    	{ return Stream::print(i, DEC); }
	    size_t  print(double d, int prec)
	    	{ return Stream::print(d, prec); }
	    size_t  print(double d)
	    	{ return Stream::print(d, 2); }
	
	    size_t  println(const char* s)
	    	{ return Stream::println(s); }
	    size_t  println(char c)
	    	{ return Stream::println(c); }
	    size_t  println(int i, int mode)
	    	{ return Stream::println(i, mode); }
	    size_t  println(int i)
	    	{ return Stream::println(i, DEC); }
	    size_t  println(double d, int prec)
	    	{ return Stream::println(d, prec); }
	    size_t  println(double d)
	    	{ return Stream::println(d, 2); }
	    size_t  println(void)
	    	{ return Stream::print("\n"); }

		virtual std::string readString(size_t maxLen);
		virtual std::string readLine(size_t maxLen);
		virtual std::string read(size_t maxLen) { return readString(maxLen); }
};

SQInteger registerStreamLib(HSQUIRRELVM v) ;

#endif
