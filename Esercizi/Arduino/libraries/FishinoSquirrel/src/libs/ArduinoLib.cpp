#include "ArduinoLib.h"

using namespace sqfish;

static SQInteger  _arduino_millis()
{
	return (SQInteger)(millis() & 0x7fffffff);
}

SQInteger registerArduinoLib(HSQUIRRELVM v)
{
	RootTable(v)
		.Func("millis"_LIT			, _arduino_millis)
		.Func("pinMode"_LIT			, pinMode)
		.Func("digitalRead"_LIT		, digitalRead)
		.Func("digitalWrite"_LIT	, digitalWrite)
		.Func("analogRead"_LIT		, analogRead)
		.Func("analogWrite"_LIT		, analogWrite)
	;

	ConstTable(v)
		.Value("OUTPUT"_LIT			, OUTPUT		)
		.Value("INPUT"_LIT			, INPUT			)
		.Value("INPUT_PULLUP"_LIT	, INPUT_PULLUP	)
		.Value("HIGH"_LIT			, HIGH			)
		.Value("LOW"_LIT			, LOW			)
	;
	return 1;
}
