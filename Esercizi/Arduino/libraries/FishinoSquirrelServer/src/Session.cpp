#include "Session.h"

#include "FishinoSquirrelServer.h"
#include "utils.h"
#include <JSONStreamingParser.h>

#define DEBUG_LEVEL_ERROR
#include <FishinoDebug.h>


static void  _parserCb(uint8_t filter, uint8_t level, const char *name, const char *value, void *cbObj)
{
	((Session *)cbObj)->parserCb(filter, level, name, value);
}


/*
{
	"projects": [
		"fact",
		"generators",
		"coroutines",
		"testthread",
		"erathostenes",
		"blink",
		"threadblink",
		"Client",
		"Server",
		"class"
	],
	"activeProject": "Client"
}
*/

// parser internal callback
void Session::parserCb(uint8_t filter, uint8_t level, const char *name, const char *value)
{
	// idle
	if(_state == 0)
	{
		if(level == 0 && !strcmp(name, "projects"))
			_state = 1;
		else if(level == 0 && !strcmp(name, "activeProject"))
			_activeProject = stripQuotes(value);
	}
	// reading libs
	else if(_state == 1)
	{
		if(level == 1)
		{
			if(!strcmp(value, "]"))
				_state = 0;
			else
				_projects.push_back(stripQuotes(value));
		}
	}
}

// constructor
Session::Session()
{
	_error = true;
	String sesPath = appendPath(SQUIRREL_ROOT, "last.session");
	SdFile f;
	if(!f.open(sesPath.c_str(), O_READ))
	{
		DEBUG_ERROR("Error opening session file '%s'\n", sesPath.c_str());
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
		DEBUG_ERROR("Error parsing session file : %x\n", res);
		return;
	}
	_error = false;
}

// destructor
Session::~Session()
{
}
