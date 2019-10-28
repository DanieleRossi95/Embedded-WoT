/* see copyright notice in squirrel.h */
#include "../Squirrel.h"
#include <math.h>
#include <stdlib.h>
#include "sqstdmath.h"

using namespace sqfish;


SQRESULT registerMathLib(HSQUIRRELVM v)
{
	RootTable(v)
		.Table("Math"_LIT)
			.Value("PI"_LIT			, M_PI		)
			.Value("RAND_MAX"_LIT	, RAND_MAX	)
			.Func("sqrt"_LIT		, sqrt		)
			.Func("fabs"_LIT		, fabs		)
			.Func("sin"_LIT			, sin		)
			.Func("cos"_LIT			, cos		)
			.Func("asin"_LIT		, asin		)
			.Func("acos"_LIT		, acos		)
			.Func("log"_LIT			, log		)
			.Func("log10"_LIT		, log10		)
			.Func("tan"_LIT			, tan		)
			.Func("atan"_LIT		, atan		)
			.Func("atan2"_LIT		, atan2		)
			.Func("pow"_LIT			, pow		)
			.Func("floor"_LIT		, floor		)
			.Func("ceil"_LIT		, ceil		)
			.Func("exp"_LIT			, exp		)

			.Func("srand"_LIT		, srand		)
			.Func("rand"_LIT		, rand		)
			.Func("fabs"_LIT		, fabs		)
			.Func("abs"_LIT			, abs		)
	;
    return SQ_OK;
}
