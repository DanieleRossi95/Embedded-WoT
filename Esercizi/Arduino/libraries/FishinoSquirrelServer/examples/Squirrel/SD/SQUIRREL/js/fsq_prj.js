"use strict";

// save current project
// @@ later we shall put it in a timed function
// to avoid too many uploads
function sq_saveCurrentProject(doneFunc) {
	// if no data, don't save
	if(sq_currentProject == "") {
		doneFunc(true);
		return;
	}

	// save current file, first
	sq_saveCurrentFile(function(status) {

		// save project file
		var prjPath = sq_projectsRoot + sq_currentProject + '/' + sq_currentProject + '.prj';
		sq_saveFile(prjPath, sq_stringify(sq_currentProjectData), function(status) {
			doneFunc(status);
		});
	});
}

// activate a project (called by click handler, for example)
function sq_activateProject(name, doneFunc) {
	
	// don't do anything if project didn't change
	if(name == sq_currentProject) {
		doneFunc(true);
		return;
	}

	// save current project before switching
	sq_saveCurrentProject(function(status) {
		
		if(!status) {
			doneFunc(false);
			return;
		}

		sq_currentFile = "";
		sq_currentProject = "";
		sq_currentProjectData = {};
		
		if(name == "") {
			doneFunc(true);
			return;
		}
		
		// load project file
		sq_loadFile(sq_projectsRoot + name + "/" + name + '.prj', function(status, data) {
			if(!status) {
				doneFunc(false);
				return;
			}

			data = JSON.parse(data);
			if(data) {
				// store to current project descriptor
				sq_currentProject = name;
				sq_currentProjectData = data;
				
				// load project libraries
				sq_loadLibraries(data.libraries, function(status) {
					
					if(!status) {
						doneFunc(false);
						return;
					}
					
					// if there are opened files show the files pane
					// only if they're still inside project or its libraries
					// as they can be wiped by another project
					// and fill its data, otherwise hide it
					var opened = Object.keys(data.openedFiles);
					var openedFiles = [];
					opened.forEach(function(item) {
						var name = sq_getFileName(item);
						if(item.startsWith(sq_projectsRoot + sq_currentProject + "/")) {
							if(sq_currentProjectData.files.indexOf(name) >= 0)
								openedFiles.push(item);
						}
						else if(item.startsWith(sq_libRoot)) {
							var lib = item.substring(sq_libRoot.length).split("/")[0];
							if(sq_currentProjectData.libraries.indexOf(lib) >= 0) {
								var libFiles = sq_projectLibs[lib].files;
								if(libFiles.indexOf(name) >= 0)
									openedFiles.push(item);
							}
						}
					});
					sq_fillFilesTabs(openedFiles);
					
					// check if last opened file still belongs to project
					// otherwise just take first project's file
					sq_currentFile = data.currentFile;
					if(sq_currentFile.startsWith(sq_projectsRoot + sq_currentProject + "/")) {
						if(sq_currentProjectData.files.indexOf(sq_getFileName(sq_currentFile)) < 0)
							sq_currentFile = "";
					}
					else if(sq_currentFile.startsWith(sq_libRoot)) {
						var lib = sq_currentFile.substring(sq_libRoot.length).split("/")[0];
						if(sq_currentProjectData.libraries.indexOf(lib) < 0)
							sq_currentFile = "";
						else {
							var libFiles = sq_projectLibs[lib].files;
							if(libFiles.indexOf(sq_getFileName(sq_currentFile)) < 0)
								sq_currentFile = "";
						}
					}
					else
						sq_currentFile = "";

					// if no current file, take project's one
					if(sq_currentFile == "")
						sq_currentFile = sq_projectsRoot + sq_currentProject + "/" + sq_currentProjectData.files[0];
						
					// activate current file when done
					sq_activateFile(sq_currentFile, function(status) {
						if(!status) {
							doneFunc(false);
							return;
						}
						sq_saveSession(function(status) {
							if(!status) {
								doneFunc(false);
								return;
							}
							doneFunc(true);
						});
					});
				});
			}
			else {
				doneFunc(false);
				return;
			}
		});
	
	});
}

// opens a project
function sq_openProject(name, doneFunc) {
	
	// activate the project
	sq_activateProject(name, doneFunc);
}

function sq_newProject(name, doneFunc) {
	var path = sq_projectsRoot + name + '/';
	var prjPath = path + name + '.prj';
	var nut = '// ' + name + '.nut\n\n';
	var nutPath = path + name + '.nut';
	var prj =
	'{' +
	'	"files":["' + name + '.nut"],' +
	'	"libraries":[],' +
	'	"openedFiles":{},' +
	'	"currentFile":"' + nutPath + '"' +
	'}';
	
	sq_saveFile(prjPath, prj, function(status) {
		if(!status) {
			doneFunc(false);
			return;
		}
		sq_saveFile(nutPath, nut, function(status) {
			if(!status) {
				doneFunc(false);
				return;
			}
			sq_openProject(name, function(status) {
				if(!status) {
					doneFunc(false);
					return;
				}
				sq_activateFile(nutPath, doneFunc);
			});
		})
	})
}

function sq_deleteProject(name, doneFunc) {
	
	// get current project
	var project = sq_currentProject;

	// reset current project, otherwise it would be saved
	// when activating next one
	sq_currentProject = "";
	
	// close project tab
	var nextItem = sq_removeTab("projectsTab", project);
	
	sq_activateProject(nextItem, function(status) {
		if(!status) {
			alert("Error activating project");
		}

		// remove the project
		sq_deleteFolderDeep(sq_projectsRoot + project, function(status) {
			if(!status) {
				doneFunc(false);
				return;
			}
			doneFunc(true);
		});
	});
}

// add a library to current project
function sq_addLibrary(libName, doneFunc) {

	// if already there, do nothing
	if(sq_currentProjectData.libraries.indexOf(libName) >= 0) {
		doneFunc(true);
		return;
	}

	// add the library
	sq_currentProjectData.libraries.push(libName);
	
	// reload libraries for project
	sq_loadLibraries(sq_currentProjectData.libraries, function(status) {
		if(!status) {
			doneFunc(false);
			return;
		}
		
		// save current project
		sq_saveCurrentProject(function(status) {
			if(!status) {
				doneFunc(false);
				return;
			}
			
			doneFunc(true);
		})
	})

}

// remove a library from current project
function sq_removeLibrary(libName, doneFunc) {

	// if not there, do nothing
	var idx = sq_currentProjectData.libraries.indexOf(libName);
	if(idx < 0) {
		doneFunc(true);
		return;
	}

	// close all opened library files first
	sq_closeLibraryFiles(libName, function(status) {

		// remove the library
		sq_currentProjectData.libraries.splice(idx, 1);
		
		// reload libraries for project
		sq_loadLibraries(sq_currentProjectData.libraries, function(status) {
			if(!status) {
				doneFunc(false);
				return;
			}
			// save current project
			sq_saveCurrentProject(function(status) {
				if(!status) {
					doneFunc(false);
					return;
				}
				
				doneFunc(true);
			})
		})
	});
}

// remove a library from current project and delete it
function sq_deleteLibrary(libName, doneFunc) {
	// first remove it
	sq_removeLibrary(libName, function(status) {
		if(!status) {
			doneFunc(false);
			return;
		}
		// remove the library
		sq_deleteFolderDeep(sq_libRoot + libName, doneFunc);
	})
}

// add a .nut file to current project
function sq_addProjectFile(name, doneFunc) {
	// strip any path
	name = sq_getFileName(name);
	var path = sq_projectsRoot + sq_currentProject + '/' + name;
	
	// don't add if already there
	if(sq_currentProjectData.files.indexOf(name) < 0)
		// add to current project files
		sq_currentProjectData.files.push(name); 

	// activate it
	sq_activateFile(path, doneFunc);
}

// add a NEW .nut file to current project
function sq_addNewProjectFile(name, doneFunc) {
	// strip any path
	name = sq_getFileName(name);
	var path = sq_projectsRoot + sq_currentProject + '/' + name;
	
	// create the file content
	var data = sq_createContent(name);
	
	// save it
	sq_saveFile(path, data, function(status) {
		if(!status) {
			doneFunc(false);
			return;
		}
		sq_addProjectFile(name, doneFunc);
	});
}

// remove a file from current project
function sq_removeProjectFile(path, doneFunc) {

	// membership check
	if(!path.startsWith(sq_projectsRoot + sq_currentProject + "/")) {
		doneFunc(false);
		return;
	}
	sq_closeFile(path, function(status) {
		if(!status) {
			doneFunc(false);
			return;
		}
		path = sq_getFileName(path);
		var idx = sq_currentProjectData.files.indexOf(path);
		if(idx >= 0)
			sq_currentProjectData.files.splice(idx, 1);
		sq_saveCurrentProject(doneFunc);
	});
}

// remove a file from current project and delete it
function sq_deleteProjectFile(path, doneFunc) {
	sq_removeProjectFile(path, function(status) {
		if(!status) {
			doneFunc(false);
			return;
		}
		sq_deleteFile(path, doneFunc);
	});
}
