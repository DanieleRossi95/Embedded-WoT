"use strict";

//////////////////////////////////////////////////////////////////////
//                    LIBRARIES HANDLING FUNCTIONS                  //
//////////////////////////////////////////////////////////////////////

// save library (must belong to current project
function sq_saveLibrary(libName, doneFunc) {
	
	var libData = sq_projectLibs[libName];
	if(!libData) {
		doneFunc(false);
		return;
	}
	
	var prl = sq_stringify(libData);
	var path = sq_libRoot + libName + "/" + libName + ".prl";
	sq_saveFile(path, prl, doneFunc);
}

// close all opened library files
function sq_closeLibraryFiles(libName, doneFunc) {

	// just a small recursion to get rid of current file problem
	if(sq_currentFile.startsWith(sq_libRoot + libName)) {
		sq_saveCurrentFile(function(status) {
			if(!status) {
				doneFunc(false);
				return;
			}
			sq_currentFile = "";
			sq_closeLibraryFiles(libName, doneFunc);
			return;
		})
	}
	else {

		var paths = sq_listTabs("filesTab");
		var nextTab = "";
		paths.forEach(function(item) {
			if(item.startsWith(sq_libRoot + libName)) {
				nextTab = sq_removeTab("filesTab", item);
				delete sq_currentProjectData.openedFiles[item];
			}
		});
		// if a closed file was active activate another one
		if(sq_currentFile == "" || sq_currentFile.startsWith(sq_libRoot + libName))
			sq_activateFile(nextTab, function(status) {
				sq_currentProjectData.currentFile = nextTab;
				doneFunc(status);
			});
		else
			doneFunc(true);
	}
}

// activate a library when clicking on libraries list
function sq_activateLibrary(item) {

	// select corresponding list item
	sq_selectListItem("librariesList", item);
}

// load library descriptor
function sq_loadLibrary(name, doneFunc) {
	sq_loadFile(sq_libRoot + name + "/" + name + '.prl', function(status, data) {
		if(status) {
			data = JSON.parse(data);
			if(data) {
				doneFunc(true, name, data);
			}
			else {
				doneFunc(false, name);
			}
		}
	})
}

// load a group of library descriptor and put their info into
// global sq_libraries data
// @@ still missing handling of "requires" inside libs!!!
function sq_loadLibraries(names, doneFunc) {

	sq_projectLibs = {};
	
	// if project has no libraries, just terminate
	if(!names.length)
		doneFunc(true);
	else {
		var missing = names.length;
		var err = false;
		var done = false;
		for(var i = 0; i < names.length; i++) {
			if(err)
				break;
			sq_loadLibrary(names[i], function(status, n, data) {
				if(!status) {
					err = true;
					if(!done) {
						doneFunc(false);
						done = true;
					}
					return;
				}
				sq_projectLibs[n] = data;
				missing--;
				if(!missing && !err && !done) {
					done = true;
					doneFunc(true);
				}
			});
		}
	}
}

// create a new library
function sq_newLibrary(name, doneFunc) {
	var path = sq_libRoot + name + '/';
	var libPath = path + name + '.prl';
	var nut = '// ' + name + '.nut\n\n';
	var nutPath = path + name + '.nut';
	var prl =
	'{' +
	'	"files":["' + name + '.nut"],' +
	'	"requires":[]' +
	'}';
	
	// save library project file
	sq_saveFile(libPath, prl, function(status) {
		if(!status) {
			doneFunc(false);
			return;
		}
		// then save nut file
		sq_saveFile(nutPath, nut, doneFunc);
	})
}

// add a file to library
function sq_addLibraryFile(lib, name, doneFunc) {
	// strip any path
	name = sq_getFileName(name);
	var path = sq_libRoot + lib + '/' + name;

	// don't add if already there
	if(sq_projectLibs[lib].files.indexOf(name) < 0) {
		// add to current project files
		sq_projectLibs[lib].files.push(name);
		
		sq_saveLibrary(lib, function(status) {
			if(!status) {
				doneFunc(false);
				return;
			}
			// activate it
			sq_activateFile(path, doneFunc);
		});
	}
	else
		// activate it
		sq_activateFile(path, doneFunc);
}

// add a NEW file to library project
function sq_addNewLibraryFile(lib, name, doneFunc) {
	// strip any path
	name = sq_getFileName(name);
	var path = sq_libRoot + lib + '/' + name;
	
	// create the file content
	var data = sq_createContent(name);
	
	// save it
	sq_saveFile(path, data, function(status) {
		if(!status) {
			doneFunc(false);
			return;
		}
		sq_addLibraryFile(lib, name, doneFunc);
	});
}

// remove a file from library
function sq_removeLibraryFile(lib, path, doneFunc) {

	// membership check
	if(!path.startsWith(sq_libRoot + lib + "/")) {
		doneFunc(false);
		return;
	}
	sq_closeFile(path, function(status) {
		if(!status) {
			doneFunc(false);
			return;
		}
		path = sq_getFileName(path);
		var idx = sq_projectLibs[lib].files.indexOf(path);
		if(idx >= 0)
			sq_projectLibs[lib].files.splice(idx, 1);
		sq_saveLibrary(lib, doneFunc);
	});
}

// remove a file from library and delete it
function sq_deleteLibraryFile(lib, path, doneFunc) {
	sq_removeLibraryFile(lib, path, function(status) {
		if(!status) {
			doneFunc(false);
			return;
		}
		sq_deleteFile(path, doneFunc);
	});
}

// get library dependencies
function sq_getLibraryDeps(name) {
	var libData = sq_projectLibs[name];
	if(typeof(libData) == 'undefined')
		return [];
	return libData.requires;
}

// set library dependencies
function sq_setLibraryDeps(name, deps, doneFunc) {
	if(typeof(sq_projectLibs[name]) == 'undefined') {
		doneFunc(false);
		return;
	}
	sq_projectLibs[name].requires = deps;
	sq_saveLibrary(name, doneFunc);
}
