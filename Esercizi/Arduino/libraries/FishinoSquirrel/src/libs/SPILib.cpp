#include "SPILib.h"

#include <SPI.h>

using namespace sqfish;

class SqSPIClass
{
	private:
		
		// the SPIClass pointer
		SPIClass *_spi;
		bool _enabled;
		
		// current transfer settings
		uint32_t _clock;
		uint8_t _bitOrder;
		uint8_t _mode;
		SPISettings _settings;
		
		// the pin select
		uint8_t _cs;
		
		void doSettings(void)
		{
			_settings = SPISettings(_clock, _bitOrder, _mode);
		}
		
	protected:
		
	public:
		
		// constructor - takes an existing SPIClass object
		SqSPIClass(SPIClass &spi) : _spi(&spi)
		{
			_clock = 25000000;
			_bitOrder = MSBFIRST;
			_mode = SPI_MODE0;
			_settings = SPISettings(_clock, _bitOrder, _mode);
			_enabled = false;
			_cs = SS;
		}
		
		// destructor - just close the interface if needed
		~SqSPIClass()
		{
			if(_enabled)
				end();
		}

		// enables the interface (does nothing on WiFi SPI interface!)
		void begin()
		{
			// do NOT touch at WiFi select pin
			if(_cs != WIFICS)
			{
				digitalWrite(_cs, HIGH);
				pinMode(_cs, OUTPUT);
			}

			// do nothing on wifi SPI!
			if(_spi == &WIFISPI)
				return;
			if(_enabled)
				return;
			_enabled = true;
			_spi->begin();
		}
		
		// terminates the interface (does nothing on WiFi SPI interface)
		void end()
		{
			// do NOT touch at WiFi select pin
			if(_cs != WIFICS)
				pinMode(_cs, INPUT);

			// do nothing on wifi SPI!
			if(_spi == &WIFISPI)
				return;
			if(!_enabled)
				return;
			_enabled = false;
			_spi->end();
		}
		
		// set the interface select pin
		void setCS(uint8_t cs)
		{
			// do NOT touch at WiFi select pin
			if(_cs != WIFICS)
				pinMode(_cs, INPUT);
			_cs = cs;
			if(_cs != WIFICS)
			{
				digitalWrite(_cs, HIGH);
				pinMode(_cs, OUTPUT);
			}
		}
		
		// set SPI parameters
		void setClock(uint32_t clock)
		{
			_clock = clock;
			doSettings();
		}
		
		void setBitOrder(uint8_t order)
		{
			_bitOrder = order;
			doSettings();
		}
		
		void setDataMode(uint8_t mode)
		{
			_mode = mode;
			doSettings();
		}
		
		// transfer a byte
		uint8_t transfer(uint8_t data)
		{
			_spi->beginTransaction(_settings);
			digitalWrite(_cs, LOW);
			uint8_t res = _spi->transfer(data);
			digitalWrite(_cs, HIGH);
			_spi->endTransaction();
			return res;
		}
		
		// transfer a word
		uint16_t transfer16(uint16_t data)
		{
			_spi->beginTransaction(_settings);
			digitalWrite(_cs, LOW);
			uint8_t res = _spi->transfer16(data);
			digitalWrite(_cs, HIGH);
			_spi->endTransaction();
			return res;
		}
		
		// transfer a buffer
		void transfer(void *buf, size_t count)
		{
		}
};

SqSPIClass SqSPI(SPI);
#if (NUM_SPI_PORTS >= 2)
	 SqSPIClass SqSPI0(SPI0);
#endif
#if (NUM_SPI_PORTS >= 3)
	 SqSPIClass SqSPI1(SPI1);
#endif
#if (NUM_SPI_PORTS >= 4)
	 SqSPIClass SqSPI2(SPI2);
#endif

SQInteger registerSPILib(HSQUIRRELVM v)
{
	RootTable(v)
		.Class<SqSPIClass>("SPIClass"_LIT)
			.NoCreate()
			.Func						("begin"_LIT		, &SqSPIClass::begin		)
			.Func						("end"_LIT			, &SqSPIClass::end			)
			.Func						("setCS"_LIT		, &SqSPIClass::setCS		)
			.Func						("setClock"_LIT		, &SqSPIClass::setClock		)
			.Func						("setBitOrder"_LIT	, &SqSPIClass::setBitOrder	)
			.Func						("setDataMode"_LIT	, &SqSPIClass::setDataMode	)
			.Func<uint8_t, uint8_t>		("transfer"_LIT		, &SqSPIClass::transfer		)
			.Func						("transfer16"_LIT	, &SqSPIClass::transfer16	)
//			.Func<>("transfer"_LIT, transfer)
			--
		.Instance<SqSPIClass>("SPI"_LIT, "SPIClass"_LIT, SqSPI)
#if (NUM_SPI_PORTS >= 2)
		.Instance<SqSPIClass>("SPI0"_LIT, "SPIClass"_LIT, SqSPI0)
#endif
#if (NUM_SPI_PORTS >= 3)
		.Instance<SqSPIClass>("SPI1"_LIT,"SPIClass"_LIT,  SqSPI1)
#endif
#if (NUM_SPI_PORTS >= 4)
		.Instance<SqSPIClass>("SPI2"_LIT, "SPIClass"_LIT, SqSPI2)
#endif
		
		.Value("LSBFIRST"_LIT	, LSBFIRST)
		.Value("MSBFIRST"_LIT	, MSBFIRST)
		.Value("SPI_MODE0"_LIT	, SPI_MODE0)
		.Value("SPI_MODE1"_LIT	, SPI_MODE1)
		.Value("SPI_MODE2"_LIT	, SPI_MODE2)
		.Value("SPI_MODE3"_LIT	, SPI_MODE3)
	;

	return 1;
}
