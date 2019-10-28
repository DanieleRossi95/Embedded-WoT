// Library : 'FishinoSquirrel' -- File : 'Project.cpp'
// Created by FishIDE application 
#include "Project.h"

#include "Library.h"

#include "FishinoSquirrelServer.h"
#include "utils.h"
#include <JSONStreamingParser.h>

#define DEBUG_LEVEL_ERROR
#include <FishinoDebug.h>


static void  _parserCb(uint8_t filter, uint8_t level, const char *name, const char *value, void *cbObj)
{
	((Project *)cbObj)->parserCb(filter, level, name, value);
}

// constructor
Project::Project(const char *s)
{
	_error = true;
	_name = s;
	String root = appendPath(PROJECTS_ROOT, s);
	String prj = appendPathExt(root, s, ".prj");
	SdFile f;
	if(!f.open(prj.c_str(), O_READ))
	{
		DEBUG_ERROR("Error opening file '%s'\n", prj.c_str());
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
		DEBUG_ERROR("Error parsing project file : %x\n", res);
		return;
	}

	// append root path to project files
	for(size_t i = 0; i < _files.size(); i++)
		_files[i] = appendPath(root, _files[i]);
	
	// now it's time to load libraries and gather their files
	std::vector<Library> libs;
	for(size_t iLib = 0; iLib < _libraries.size(); iLib++)
	{
		libs.push_back(Library(_libraries[iLib].c_str()));
		
		// if added library is bad, return with error state
		if(!libs.back())
		{
			DEBUG_ERROR("Library '%s' error\n", _libraries[iLib].c_str());
			return;
		}
	}
	
	// now resolve dependencies
	std::set<String> deps;
	DEBUG_ERROR("Found %lu libraries\n", _libraries.size());
	for(size_t iLib = 0; iLib < _libraries.size(); iLib++)
		deps.insert(_libraries[iLib]);
	while(true)
	{
		int missing = 0;
		for(size_t iLib = 0; iLib < libs.size(); iLib++)
		{
			Library const &lib = libs[iLib];
			std::vector<String> const &reqs = lib.getRequires();
			for(size_t iDep = 0; iDep < reqs.size(); iDep++)
			{
				String req = reqs[iDep];
				if(!deps.count(req))
				{
					missing++;
					_libraries.push_back(req);
					deps.insert(req);
				}
			}
		}
		if(!missing)
			break;
		DEBUG_ERROR("Still missing %d libs\n", missing);
		for(size_t iMiss = _libraries.size() - missing; iMiss < _libraries.size(); iMiss++)
		{
			String newLib = _libraries[iMiss];
			deps.insert(newLib);
			newLib = _libraries[iMiss];
			DEBUG_ERROR("Adding dependent lib '%s'\n", newLib.c_str());
			libs.push_back(Library(newLib.c_str()));
			if(!libs.back())
			{
				DEBUG_ERROR("Library '%s' error\n", newLib.c_str());
				return;
			}
		}
	}
	
	// ok, we've now all files and libraries loaded
	// we shall put all of them into project files
	// starting with libraries and ending with project files
	std::vector<String>allFiles;
	for(size_t iLib = 0; iLib < libs.size(); iLib++)
	{
		Library const &lib = libs[iLib];
		std::vector<String> const &files = lib.getFiles();
		for(size_t iFile = 0; iFile < files.size(); iFile++)
			allFiles.push_back(files[iFile]);
	}
	for(size_t iFile = 0; iFile < _files.size(); iFile++)
		allFiles.push_back(_files[iFile]);
	
	// replace project files with all files
	_files = allFiles;
	
	_error = false;
}

// destructor
Project::~Project()
{
}

// copy constructor
Project::Project(Project const &p)
{
	// library name
	_name = p._name;

	// library files
	_files = p._files;
	
	// library dependencies
	_libraries = p._libraries;
	
	// error flag (if couldn't find library
	_error = p._error;
		
}

// assignement
Project &Project::operator=(Project const &p)
{
	// library name
	_name = p._name;

	// library files
	_files = p._files;
	
	// library dependencies
	_libraries = p._libraries;
	
	// error flag (if couldn't find library
	_error = p._error;
		
	return *this;
}

// parser internal callback
//	{
//		"files": [
//			"ackermann.nut"
//		],
//		"libraries": [
//			"LIB1"
//		],
//		"openedFiles": {
//			"LIBRARIES/LIB1/gigi.html": {
//				"scrollInfo": {
//					"left": 0,
//					"top": 0,
//					"height": 259,
//					"width": 787,
//					"clientHeight": 259,
//					"clientWidth": 787
//				},
//				"cursor": {
//					"line": 0,
//					"ch": 0,
//					"sticky": null
//				}
//			},
//			"PROJECTS/ackermann/ackermann.nut": {
//				"scrollInfo": {
//					"left": 0,
//					"top": 0,
//					"height": 488,
//					"width": 1574,
//					"clientHeight": 375,
//					"clientWidth": 1574
//				},
//				"cursor": {
//					"line": 0,
//					"ch": 0,
//					"sticky": null
//				}
//			},
//			"/SQUIRREL/PROJECTS/ackermann/ackermann.nut": {
//				"scrollInfo": {
//					"left": 0,
//					"top": 0,
//					"height": 488,
//					"width": 1574,
//					"clientHeight": 454,
//					"clientWidth": 1574
//				},
//				"cursor": {
//					"line": 0,
//					"ch": 0,
//					"sticky": null
//				}
//			}
//		},
//		"currentFile": "/SQUIRREL/PROJECTS/ackermann/ackermann.nut"
//	}
void Project::parserCb(uint8_t filter, uint8_t level, const char *name, const char *val)
{
	String value = val;
	if(value.startsWith("\""))
		value = value.substring(1, value.length() - 1);
	// idle
	if(_state == 0)
	{
		if(level == 0 && !strcmp(name, "libraries"))
			_state = 1;
		else if(level == 0 && !strcmp(name, "files"))
			_state = 2;
	}
	// reading libs
	else if(_state == 1)
	{
		if(level == 1)
		{
			if(value == "]")
				_state = 0;
			else
				_libraries.push_back(value);
		}
	}
	// reading files
	else if(_state == 2)
	{
		if(level == 1)
		{
			if(value == "]")
				_state = 0;
			else
				_files.push_back(value);
		}
	}
}

// testing -- dump project data
void Project::Dump(void)
{
	DEBUG_PRINT("---- PROJECT DUMP -----\n");
	DEBUG_PRINT("LIBRARIES:\n");
	for(size_t iLib = 0; iLib < _libraries.size(); iLib++)
		DEBUG_PRINT("  %s\n", _libraries[iLib].c_str());
	DEBUG_PRINT("FILES:\n");
	for(size_t iFile = 0; iFile < _files.size(); iFile++)
		DEBUG_PRINT("  %s\n", _files[iFile].c_str());
}
