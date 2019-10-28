// Library : 'FishinoSquirrel' -- File : 'StreamLib.cpp'
// Created by FishIDE application
#include "StreamLib.h"

using namespace sqfish;

std::string SqStream::readString(size_t maxLen)
{
	if (!maxLen)
		return std::string();
	char *buf = (char *)malloc(maxLen + 1);
	size_t len = read(buf, maxLen);
	buf[len] = 0;
	std::string res = buf;
	free(buf);
	return res;
}

std::string SqStream::readLine(size_t maxLen)
{
	std::string res;
	if (!maxLen)
		return res;
	while (maxLen--)
	{
		char c = read();
		if (c == '\n' || c <= 0)
			break;
		res += c;
	}
	return res;
}

SQInteger registerStreamLib(HSQUIRRELVM v)
{
	RootTable(v)
		.Class<SqStream>("Stream"_LIT)
			.NoCreate()
			.Func								("flush"_LIT		, &SqStream::flush)
			.Func<size_t, char>					("write"_LIT		, &SqStream::write)
			.Func<size_t, const char*, size_t>	("write"_LIT		, &SqStream::write)
			.Func<int>							("read"_LIT			, &SqStream::read)
			.Func<std::string, size_t>			("read"_LIT			, &SqStream::read)
			.Func								("available"_LIT	, &SqStream::available)
			.Func								("tell"_LIT			, &SqStream::tell)
			.Func								("length"_LIT		, &SqStream::length)
			.Func								("eof"_LIT			, &SqStream::eof)

			.Func<size_t, const char *>			("print"_LIT		, &SqStream::print)
			.Func<size_t, char>					("print"_LIT		, &SqStream::print)
			.Func<size_t, char>					("print"_LIT		, &SqStream::print)
			.Func<size_t, int, int>				("print"_LIT		, &SqStream::print)
			.Func<size_t, int>					("print"_LIT		, &SqStream::print)
			.Func<size_t, double, int>			("print"_LIT		, &SqStream::print)
			.Func<size_t, double>				("print"_LIT		, &SqStream::print)

			.Func<size_t, const char *>			("println"_LIT		, &SqStream::println)
			.Func<size_t, char>					("println"_LIT		, &SqStream::println)
			.Func<size_t, char>					("println"_LIT		, &SqStream::println)
			.Func<size_t, int, int>				("println"_LIT		, &SqStream::println)
			.Func<size_t, int>					("println"_LIT		, &SqStream::println)
			.Func<size_t, double, int>			("println"_LIT		, &SqStream::println)
			.Func<size_t, double>				("println"_LIT		, &SqStream::println)

			.Func								("readString"_LIT	, &SqStream::readString)
			.Func								("readLine"_LIT		, &SqStream::readLine)
	;
	return 0;
}
