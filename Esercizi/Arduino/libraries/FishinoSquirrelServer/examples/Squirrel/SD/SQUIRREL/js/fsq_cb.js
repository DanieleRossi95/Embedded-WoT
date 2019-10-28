"use strict";

//////// TOP TOOLBAR HANDLERS

// toolbar handlers
function sq_newProjectCb() {
	sq_folderDialog("newProject", "New project", sq_projectsRoot, "sq.png", [], function(status, name) {
		if(!status)
			return;
		sq_newProject(name, function(status) {
			if(!status)
				alert("Error creating project");
				
			// resync gui
			sq_syncIDE(true);
		});
	});
	return false;
}

function sq_selectProjectCb() {
	sq_folderDialog("openProject", "Open project", sq_projectsRoot, "sq.png", [], function(status, name) {
		if(!status)
			return;
		sq_openProject(name, function(status) {
			if(!status)
				alert("Error opening project");

			// resync gui
			sq_syncIDE(true);
		});
	});
	return false;
}

function sq_saveProjectCb() {
	sq_saveCurrentProject(function(status) {
		if(!status)
			alert("Error saving project");
	});
	return false;
}

function sq_undoCb() {
	if(sq_currentFile != "")
		myCodeMirror.undo();
	return false;
}


function sq_redoCb() {
	if(sq_currentFile != "")
		myCodeMirror.redo();
	return false;
}


function sq_buildCb() {
	alert("Build");
	return false;
}


function sq_uploadCb() {
	sq_runProject(sq_currentProject, function(status) {
		if(!status)
			alert("Error running project");
	});
	return false;
}

//////// TABS HANDLERS

// click on project or file tabs
function sq_tabClickCb(tab, item) {

	// if tab is a project tab, activate project
	if(tab == "projectsTab") {
		sq_activateProject(item, function(status) {
			if(!status)
				alert("Error activating project");

			// resync gui
			sq_syncIDE(false);
		});
	}
	else if(tab == "filesTab") {
		sq_activateFile(item, function(status) {
			if(!status)
				alert("Error activating file");

			// resync gui
			sq_syncIDE(true);
		});
	}
}

// click on close cross on project or file tabs
function sq_tabCloseCb(tab, item) {

	var nextItem = sq_removeTab(tab, item);
	if(tab == "projectsTab") {
		sq_activateProject(nextItem, function(status) {
			if(!status)
				alert("Error activating project");

			// resync gui
			sq_syncIDE(true);
		});
	}
	else {
		sq_activateFile(nextItem, function(status) {
			if(!status)
				alert("Error activating file");

			delete sq_currentProjectData.openedFiles[item];
			sq_saveCurrentProject(function(status) {
				if(!status)
					alert("Error saving current project");

				// resync gui
				sq_syncIDE(true);
			});
		});
	}

}

//////// SIDE LISTS HANDLERS

// click on a side library or files list
function sq_listClickCb(list, item) {

	// if tab is a project tab, activate project
	if(list == "librariesList") {
		sq_activateLibrary(item);
		sq_saveSession(function(status) {
			if(!status)
				alert("Error saving session");

			// resync gui
			sq_syncIDE();
		});
	}
	else if(list == "filesList") {
		sq_appendTab("filesTab", item);
		sq_activateFile(item, function(status) {
			
			if(!status)
				alert("Error activating file");

			// resync gui
			sq_syncIDE();
		});
	}
}

//////// CONTEXT MENUS HANDLERS


// lists handlers
function sq_deleteProjectCb() {
	
	// delete required project
	sq_deleteProject(sq_currentProject, function(status) {
		if(!status) {
			alert("Error deleting project");
		}
		// resync gui
		sq_syncIDE();
	})
}

function sq_addLibraryCb() {
	var exclude = Object.keys(sq_currentProjectData.libraries);
	sq_folderDialog("openLib", "Select library", sq_libRoot, "sqlib.png", exclude, function(status, name) {
		if(!status)
			return;
		sq_addLibrary(name, function(status) {
			if(!status)
				alert("Error adding library");

			// resync gui
			sq_syncIDE();
		});
	});
}

function sq_addNewLibraryCb() {
	sq_folderDialog("newLib", "New library", sq_libRoot, "sqlib.png", [], function(status, name) {
		if(!status)
			return;
		sq_newLibrary(name, function(status) {
			if(!status) {
				alert("Error creating library");
				return;
			}

			sq_addLibrary(name, function(status) {
				if(!status)
					alert("Error adding library");
				// resync gui
				sq_syncIDE();
			});
		});
	});
}

function sq_removeLibraryCb(libName) {
	sq_removeLibrary(libName, function(status) {
		if(!status)
			alert("Error removing library");
			
		// resync gui
		sq_syncIDE();
	});
}

function sq_deleteLibraryCb(libName) {
	sq_deleteLibrary(libName, function(status) {
		if(!status)
			alert("Error deleting library");
			
		// resync gui
		sq_syncIDE();
	});
}

// project files management

function sq_addProjectFileCb() {
	sq_fileDialog("openFile", "Select file", sq_projectsRoot + sq_currentProject + '/', undefined, [".prj", ".prl"], function(status, name) {
		if(!status)
			return;
		sq_addProjectFile(name, function(status) {
			if(!status)
				alert("Error adding file");

			// resync gui
			sq_syncIDE();
		});
	});
}

function sq_addProjectNewFileCb() {
	sq_fileDialog("newFile", "New file", sq_projectsRoot + sq_currentProject + '/', undefined, [".prj", ".prl"], function(status, name) {
		if(!status)
			return;
		sq_addNewProjectFile(name, function(status) {
			if(!status)
				alert("Error adding file");

			// resync gui
			sq_syncIDE();
		});
	});
}

function sq_removeProjectFileCb(path) {
	sq_removeProjectFile(path, function(status) {
		if(!status)
			alert("Error removing project file");
			
		sq_syncIDE();
	});
}

function sq_deleteProjectFileCb(path) {
	var name = sq_getFileName(path);
	sq_yesNoDlg("Delete file", "Delete file &#39;<strong>" + name + "</strong>&#39; from project &#39;<strong>" + sq_currentProject + "</strong>&#39; ?<br><strong>This operation can&#39t be undone!</strong>", function(status) {
		if(!status)
			return;
		sq_deleteProjectFile(path, function(status) {
			if(!status)
				alert("Error removing project file");
				
			sq_syncIDE();
		});
	});
}

// library files management

function sq_addLibraryFileCb(libName) {
	sq_fileDialog("openFile", "Select file", sq_libRoot + libName + '/', undefined, [".prj", ".prl"], function(status, name) {
		if(!status)
			return;
		sq_addLibraryFile(libName, name, function(status) {
			if(!status)
				alert("Error adding file");

			// resync gui
			sq_syncIDE();
		});
	});
}

function sq_addLibraryNewFileCb(libName) {
	sq_fileDialog("newFile", "New file", sq_libRoot + libName + '/', undefined, [".prj", ".prl"], function(status, name) {
		if(!status)
			return;
		sq_addNewLibraryFile(libName, name, function(status) {
			if(!status)
				alert("Error adding file");

			// resync gui
			sq_syncIDE();
		});
	});
}

function sq_removeLibraryFileCb(libName, filePath) {
	sq_removeLibraryFile(libName, filePath, function(status) {
		if(!status)
			alert("Error removing library file");
			
		sq_syncIDE();
	});
}

function sq_deleteLibraryFileCb(libName, filePath) {
	var name = sq_getFileName(filePath);
	sq_yesNoDlg("Delete file", "Delete file &#39;<strong>" + name + "</strong>&#39; from library &#39;<strong>" + libName + "</strong>&#39; ?<br><strong>This operation can&#39t be undone!</strong>", function(status) {
		if(!status)
			return;
		sq_deleteLibraryFile(libName, filePath, function(status) {
			if(!status)
				alert("Error removing library file");
				
			sq_syncIDE();
		});
	});
}

function sq_libraryDepsCb(libName) {
	var deps = sq_getLibraryDeps(libName);
	sq_libraryDepsDialog(libName, deps, function(status, newDeps) {
		if(!status)
			return;
		
		// remove itself from deps, if set
		var idx = newDeps.indexOf(name);
		if(idx >= 0)
			newDeps.splice(idx, 1);
		sq_setLibraryDeps(libName, newDeps, function(status) {
			if(!status)
				alert("Error settings library dependencies");
		});
	});
}
