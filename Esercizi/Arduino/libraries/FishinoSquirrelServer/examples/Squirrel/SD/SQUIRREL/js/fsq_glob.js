"use strict";

//////////////////////////////////////////////////////////////////////
//                         GLOBAL VARIABLES                         //
//////////////////////////////////////////////////////////////////////

var sq_currentProject = "";
var sq_currentProjectData = {};

// current file displayed in editor
var sq_currentFile = "";

// libraries for current project
var sq_projectLibs = {};

// file pool -- files already loaded
// files are stored as 'path':['content': content, 'dirty':boolean]
var sq_filePool = {};

// squirrel root
var sq_root = "/SQUIRREL/";

// images root folder
var sq_imgRoot = sq_root + "img/";

// libraries root folder
var sq_libRoot = sq_root + "LIBRARIES/";

// projects root folder
var sq_projectsRoot = sq_root + "PROJECTS/";
