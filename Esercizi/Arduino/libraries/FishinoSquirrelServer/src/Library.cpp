// Library : 'FishinoSquirrel' -- File : 'Library.cpp'
// Created by FishIDE application 
#include "Library.h"

#include "FishinoSquirrelServer.h"
#include "utils.h"
#include <JSONStreamingParser.h>

#define DEBUG_LEVEL_ERROR
#include <FishinoDebug.h>

static void  _parserCb(uint8_t filter, uint8_t level, const char *name, const char *value, void *cbObj)
{
	((Library *)cbObj)->parserCb(filter, level, name, value);
}

// constructor
Library::Library(const char *s)
{
	_error = true;
	_name = s;
	String root = appendPath(LIBS_ROOT, s);
	String prl = appendPathExt(root, s, ".prl");
	SdFile f;
	if(!f.open(prl.c_str(), O_READ))
	{
		DEBUG_ERROR("Error opening file '%s'\n", prl.c_str());
		return;
	}

	_state = 0;
	JSONStreamingParser parser;
	parser.setCallback(_parserCb, this);
	char c;
	uint8_t res = 1;
	while( (c = f.read()) != -1 && res != JSONStreamingParser::PARSER_FINISHED && res != JSONStreamingParser::PARSER_ERROR)
		res = parser.feed(c);

	f.close();
	if(res != JSONStreamingParser::PARSER_FINISHED)
	{
		DEBUG_ERROR("Error parsing library file\n");
		return;
	}

	_error = false;
	for(size_t i = 0; i < _files.size(); i++)
		_files[i] = appendPath(root, _files[i]);
}

// destructor
Library::~Library()
{
}

// copy constructor
Library::Library(Library const &l)
{
	// library name
	_name = l._name;

	// library files
	_files = l._files;
	
	// library dependencies
	_requires = l._requires;
	
	// error flag (if couldn't find library
	_error = l._error;
		
}

// assignement
Library &Library::operator=(Library const &l)
{
	// library name
	_name = l._name;

	// library files
	_files = l._files;
	
	// library dependencies
	_requires = l._requires;
	
	// error flag (if couldn't find library
	_error = l._error;
	
	return *this;
}


// parser internal callback
//	{
//		"requires": [
//			"MYLIB"
//		],
//		"files": [
//			"LIB1.nut",
//			"list.nut",
//			"loops.nut"
//		]
//	}
void Library::parserCb(uint8_t filter, uint8_t level, const char *name, const char *val)
{
	String value = val;
	if(value.startsWith("\""))
		value = value.substring(1, value.length() - 1);

	if(_state == 0 && level == 0 && !strcmp(name, "requires"))
		_state = 1;
	else if(_state == 0 && level == 0 && !strcmp(name, "files"))
		_state = 2;
	else if(_state == 1 && level == 1)
	{
		if(value == "]")
			_state = 0;
		else
			_requires.push_back(value);
	}
	else if(_state == 2 && level == 1)
	{
		if(value == "]")
			_state = 0;
		else
			_files.push_back(value);
	}
}
