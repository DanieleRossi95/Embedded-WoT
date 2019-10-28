"use strict";

// save current file in editor
// @@ later we shall put it in a timed function
// to avoid too many uploads
function sq_saveCurrentFile(doneFunc) {

	// if no data, don't save
	if(sq_currentFile == "") {
		doneFunc(true);
		return;
	}
		
	// save current file position
	var cursor = myCodeMirror.getCursor();
	var scrollInfo = myCodeMirror.getScrollInfo();

	sq_currentProjectData.openedFiles[sq_currentFile] = {'scrollInfo':scrollInfo, 'cursor':cursor};

	// check if editor is dirty
	if(myCodeMirror.isClean()) {
		doneFunc(true);
		return;
	}
		
	var data = myCodeMirror.getValue();
	sq_saveFile(sq_currentFile, data, function(status) {
		myCodeMirror.markClean();
		doneFunc(true);
	});
}

// activate a file (called by click handler, for example)
function sq_activateFile(path, doneFunc) {
	
	var mixedMode = {
		name: "htmlmixed",
		scriptTypes: [{matches: /\/x-handlebars-template|\/x-mustache/i,
		mode: null},
		{matches: /(text|application)\/(x-)?vb(a|script)/i,
		mode: "javascript"}]
	};


	// save current file
	sq_saveCurrentFile(function(status) {
		
		if(!status) {
			sq_currentFile = "";
			doneFunc(false);
			return;
		}
		
		if(sq_currentProject == "" || path == "") {
			sq_currentFile = "";
			doneFunc(true);
			return;
		}
	
		// change current file record in current project
		sq_currentProjectData.currentFile = path;
		
		sq_loadFile(path, function(status, data) {
			if(!status) {
				doneFunc(false);
				return;
			}

			// store new file
			sq_currentFile = path;
			
			// append a new file tab, if needed, and select it
			sq_appendTab("filesTab", path);
			sq_selectTab("filesTab", path);
			
			myCodeMirror.setValue(data);
			switch(sq_getFileExt(path)) {
				case ".nut":
					myCodeMirror.setOption("mode", "javascript");
					break;
				case ".htm":
				case ".html":
					myCodeMirror.setOption("mode", mixedMode);
					break;
				default:
					myCodeMirror.setOption("mode", "javascript");
			}
			myCodeMirror.markClean();
			
			// reset cursor position
			var info = sq_currentProjectData.openedFiles[path];
			if(typeof(info) != 'undefined') {
				myCodeMirror.scrollTo(info.scrollInfo);
				myCodeMirror.setCursor(info.cursor);
			}
			myCodeMirror.focus();
			
			doneFunc(true);
		});

	})
}

// close an opened file
function sq_closeFile(path, doneFunc) {
	
	// just a small recutsion to get rid of current file problem
	if(sq_currentFile == path) {
		sq_saveCurrentFile(function(status) {
			if(!status) {
				doneFunc(false);
				return;
			}
			sq_currentFile = "";
			sq_closeFile(path, doneFunc);
			return;
		})
	}
	else {
		// if file is not opened, just do nothing
		if(!sq_findTab("filesTab", path)) {
			doneFunc(true);
			return;
		}
		var nextFile = sq_removeTab("filesTab", path);
		delete sq_currentProjectData.openedFiles[path];
		if(sq_currentFile == path || sq_currentFile == "")
			sq_activateFile(nextFile, doneFunc);
		else
			doneFunc(true);
	}
}


// create content for new file based on file extension
function sq_createContent(path) {

	var ext = sq_getFileExt(path);
	var name = sq_getFileName(path);
	var title = sq_getFileTitle(path);
	var data;
	switch(ext) {
		case ".nut":
			data = "// " + name + "\n\n";
			break;
		case ".html":
		case ".htm":
			data = 
				"<!DOCTYPE html>\n" +
				"<html>\n" +
				"	<head>\n" +
				"		<title>" + title + "</title>\n" +
				"		<meta charset=\"utf-8\"/>\n" +
				"	</head>\n" +
				"	<body>\n" +
				"	</body>\n" +
				"<\html>\n"
			;
			break;
		default:
			data = "";
	};
	return data;
}
