#include "HardwareSerialLib.h"

using namespace sqfish;

class SqSerial : public SqStream
{
	private:
		
	protected:
		
		void *_device;
		bool _opened;
		
		virtual void  flush(void) = 0;
		virtual size_t  write(char c) = 0;
		virtual size_t  write(const char *buf, size_t len) = 0;
		virtual int  read(void) = 0;
		virtual size_t  read(char *buf, size_t maxLen) = 0;
		virtual int  available(void) = 0;
		virtual int  peek(void) = 0;

		virtual size_t  tell(void)
			{ return 0; }
		virtual size_t  length(void)
			{ return available(); }
		virtual bool  eof(void)
			{ return !_opened; }
		virtual bool  opened(void)
			{ return _opened; }
		
		 SqSerial(void *dev) : _device(dev), _opened(false) {};
	public:
		
		// start serial
		virtual void  begin(int speed) = 0;
		
		// terminate serial
		virtual void  end(void) = 0;

		virtual  ~SqSerial() {}

};

#ifdef _USE_USB_FOR_SERIAL_
class SqUSBSerial : public SqSerial
{
	private:
		
		inline USBSerial  *getSerial(void) { return (USBSerial *)_device; }
		
	protected:

	public:

		// constructor
		 SqUSBSerial(USBSerial &s) : SqSerial(&s) {}

		// destructor
		 ~SqUSBSerial() { end(); };
		
		virtual void  flush(void)
			{ getSerial()->flush(); }
		virtual size_t  write(char c)
			{ return getSerial()->write(c); }
		virtual size_t  write(const char *buf, size_t len)
			{ return getSerial()->write(buf, len); }
		virtual int  read(void)
			{ return getSerial()->read(); }
		virtual size_t  read(char *buf, size_t maxLen)
			{ return getSerial()->readBytes(buf, maxLen); }
		virtual int  available(void)
			{ return getSerial()->available(); }
		virtual int  peek(void)
			{ return getSerial()->peek(); }

		// start serial
		void  begin(int speed) { getSerial()->begin(speed); _opened = true; }
		
		// terminate serial
		void  end(void) { getSerial()->end(); _opened = false; }
};
#endif

class SqHardwareSerial : public SqSerial
{
	private:
		
		inline HardwareSerial  *getSerial(void) { return (HardwareSerial *)_device; }
		
	protected:

	public:

		// constructor
		 SqHardwareSerial(HardwareSerial &s) : SqSerial(&s) {}

		// destructor
		 ~SqHardwareSerial() { end(); };
		
		virtual void  flush(void)
			{ getSerial()->flush(); }
		virtual size_t  write(char c)
			{ return getSerial()->write(c); }
		virtual size_t  write(const char *buf, size_t len)
			{ return getSerial()->write(buf, len); }
		virtual int  read(void)
			{ return getSerial()->read(); }
		virtual size_t  read(char *buf, size_t maxLen)
			{ return getSerial()->readBytes(buf, maxLen); }
		virtual int  available(void)
			{ return getSerial()->available(); }
		virtual int  peek(void)
			{ return getSerial()->peek(); }

		// start serial
		void  begin(SQInteger speed) { getSerial()->begin(speed); _opened = true; }
		
		// terminate serial
		void  end(void) { getSerial()->end(); _opened = false; }
};

#ifdef _USE_USB_FOR_SERIAL_
	SqUSBSerial sqSerial(Serial);
#else
	SqHardwareSerial sqSerial(Serial);
#endif
#if (NUM_SERIAL_PORTS > 1)
	SqHardwareSerial sqSerial1(Serial1);
	#if (NUM_SERIAL_PORTS > 2)
		SqHardwareSerial sqSerial2(Serial2);
		#if (NUM_SERIAL_PORTS > 3)
			SqHardwareSerial sqSerial3(Serial3);
			#if (NUM_SERIAL_PORTS > 4)
				SqHardwareSerial sqSerial4(Serial4);
				#if (NUM_SERIAL_PORTS > 5)
					SqHardwareSerial sqSerial5(Serial5);
					#if (NUM_SERIAL_PORTS > 6)
						SqHardwareSerial sqSerial6(Serial6);
						#if (NUM_SERIAL_PORTS > 7)
							SqHardwareSerial sqSerial7(Serial7);
						#endif
					#endif
				#endif
			#endif
		#endif
	#endif
#endif

SQInteger registerSerialLib(HSQUIRRELVM v)
{
	Class<SqSerial>("Stream"_LIT, "SqSerial"_LIT, v)
		.NoCreate()
		.Func("begin"_LIT	, &SqSerial::begin)
		.Func("end"_LIT		, &SqSerial::end)
	;

	RootTable(v)
		.Instance<SqSerial>("Serial"_LIT, "SqSerial"_LIT, sqSerial)
		#if (NUM_SERIAL_PORTS > 1)
			.Instance<SqSerial>("Serial1"_LIT, "SqSerial"_LIT, sqSerial1)
			#if (NUM_SERIAL_PORTS > 2)
				.Instance<SqSerial>("Serial2"_LIT, "SqSerial"_LIT, sqSerial2)
				#if (NUM_SERIAL_PORTS > 3)
					.Instance<SqSerial>("Serial3"_LIT, "SqSerial"_LIT, sqSerial3)
					#if (NUM_SERIAL_PORTS > 4)
						.Instance<SqSerial>("Serial4"_LIT, "SqSerial"_LIT, sqSerial4)
						#if (NUM_SERIAL_PORTS > 5)
							.Instance<SqSerial>("Serial5"_LIT, "SqSerial"_LIT, sqSerial5)
							#if (NUM_SERIAL_PORTS > 6)
								.Instance<SqSerial>("Serial6"_LIT, "SqSerial"_LIT, sqSerial6)
								#if (NUM_SERIAL_PORTS > 7)
									.Instance<SqSerial>("Serial7"_LIT, "SqSerial"_LIT, sqSerial7)
								#endif
							#endif
						#endif
					#endif
				#endif
			#endif
		#endif

	;

	return 1;
}
