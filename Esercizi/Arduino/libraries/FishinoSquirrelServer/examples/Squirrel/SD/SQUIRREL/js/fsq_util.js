"use strict";

//////////////////////////////////////////////////////////////////////
//                              UTILITiES                           //
//////////////////////////////////////////////////////////////////////

function sq_stringify(obj) {
	var seen = []; 

	var replacer = function(key, value) {
	  if (value != null && typeof value == "object") {
		if (seen.indexOf(value) >= 0) {
		  return;
		}
		seen.push(value);
	  }
	  return value;
	};

	return JSON.stringify(obj, replacer, '\t');
}

// find an icon depending on file extension
function sq_icon(name) {
	var n = name.toLowerCase();
	
	if(n.endsWith(".prj"))
		return sq_imgRoot + "sq.png";
	else if(n.endsWith(".prl"))
		return sq_imgRoot + "sqlib.png";
	else if(n.endsWith(".nut"))
		return sq_imgRoot + "nut.png";
	else if(n.endsWith(".htm") || n.endsWith(".html") || n.endsWith(".sqp"))
		return sq_imgRoot + "html.png";
	else if(n.endsWith(".png") || n.endsWith(".jpg") || n.endsWith(".tif") || n.endsWith(".gif") || n.endsWith(".ico"))
		return sq_imgRoot + "img.png";
	else
		return sq_imgRoot + "doc.png";
}

// some path utilities
function sq_getFileName(path) {
	return path.split('\\').pop().split('/').pop();
}

function sq_getFilePath(path) {
	path = path.split('\\');
	if(path.length > 1)
		path.pop();
	else {
		path = path.pop().split('/');
		path.pop();
	}
	return path.join('/') + '/';
}

function sq_getFileTitle(path) {
	path = sq_getFileName(path).split('.');
	if(path.length > 1)
		path.pop();
	return path.join('.');
}

function sq_getFileExt(path) {
	return '.' + sq_getFileName(path).split('.').pop();
}

//////////////////////////////////////////////////////////////////////
//                   FILE AND PATH REMOTE FUNCTIONS                 //
//////////////////////////////////////////////////////////////////////

// load a file -- loads from remote storage
function sq_loadFile(path, doneFunc) {
	
	// first try to get it from cached data
	var data = sq_filePool[path];
	if(data)
		doneFunc(true, data.content);
	else {
		$.ajax({
			type: 'POST',
			dataType: 'text',
			url: sq_root + 'sqfish.php',
			data: {'PATH': path, 'COMMAND':'LOAD'},
			success: function(data, status, xhr) {
				var lines = data.split('\n');
				var stat = JSON.parse(lines[0]);
				data = lines.slice(1).join('\n');
				if(stat.status == 'ok') {
					// store file to local cache
					sq_filePool[path] = {'content':data, 'dirty':false};

					// call handler
					doneFunc(true, data)
				}
				else {
					doneFunc(false);
				}
			},
			error: function(xhr, status, err) {
				doneFunc(false);
			}		  
		});
	}
}

// saves a file -- saves a file to storage
function sq_saveFile(path, data, doneFunc) {

	// check if file is in cache
	var cached = sq_filePool[path];
	
	// if file is unchanged, just do nothing
	if(cached && cached.content == data) {
		doneFunc(true);
		return;
	}
	
	// insert (or replace) file in local cache
	sq_filePool[path] = {'content':data, 'dirty':false};
	
	// save the file
	// we could try to defer saving, maybe
	$.ajax({
		type: 'POST',
		dataType: 'text',
		url: sq_root + 'sqfish.php',
		data: {'PATH': path, 'COMMAND':'STORE', 'CONTENT':data},
		success: function(data, status, xhr) {
			var lines = data.split('\n');
			var stat = JSON.parse(lines[0]);
			if(stat.status == 'ok')
				doneFunc(true)
			else
				doneFunc(false);
		},
		error: function(xhr, status, err) {
			doneFunc(false);
		}		  
	});
}

function sq_listFolder(path, folders, doneFunc) {
	$.ajax({
		type: 'POST',
		dataType: 'text',
		url: sq_root + 'sqfish.php',
		data: {'PATH': path, 'COMMAND':'LIST', 'FOLDERS':folders},
		success: function(data, status, xhr) {
			var lines = data.split('\n');
			var stat = JSON.parse(lines[0]);
			if(stat.status == 'ok')
				doneFunc(true, path, JSON.parse(lines[1]).elements);
			else
				doneFunc(false, path);
		},
		error: function(xhr, status, err) {
			doneFunc(false, path);
		}		  
	});
}

// remote delete file
function sq_deleteFile(path, doneFunc) {
	
	// wipe cached file, if any
	delete sq_filePool[path];
	
	// wipe remote file
	$.ajax({
		type: 'POST',
		dataType: 'text',
		url: sq_root + 'sqfish.php',
		data: {'PATH': path, 'COMMAND':'RMFILE'},
		success: function(data, status, xhr) {
			var lines = data.split('\n');
			var stat = JSON.parse(lines[0]);
			if(stat.status == 'ok') {
				doneFunc(true);
			}
			else {
				doneFunc(false);
			}
		},
		error: function(xhr, status, err) {
			doneFunc(false);
		}		  
	});
}

// remote delete folder with its subfolders
function sq_deleteFolderDeep(path, doneFunc) {

	// wipe any file in folder from cache
	var paths = Object.keys(sq_filePool);
	paths.forEach(function(item) {
		if(item.startsWith(path)) {
			delete sq_filePool[item];
		}
	});

	// wipe remote folder
	$.ajax({
		type: 'POST',
		dataType: 'text',
		url: sq_root + 'sqfish.php',
		data: {'PATH': path, 'COMMAND':'RMFOLDER'},
		success: function(data, status, xhr) {
			var lines = data.split('\n');
			var stat = JSON.parse(lines[0]);
			if(stat.status == 'ok') {
				doneFunc(true);
			}
			else {
				doneFunc(false);
			}
		},
		error: function(xhr, status, err) {
			doneFunc(false);
		}		  
	});
}

// sends a terminal command to server
function sq_sendTerm(cmd, doneFunc) {

	$.ajax({
		type: 'POST',
		dataType: 'text',
		url: sq_root + 'sqfish.php',
		data: {'COMMAND':'TERMSEND', 'CONTENT':cmd},
		success: function(data, status, xhr) {
			var lines = data.split('\n');
			var stat = JSON.parse(lines[0]);
			if(stat.status == 'ok') {
				doneFunc(true);
			}
			else {
				doneFunc(false);
			}
		},
		error: function(xhr, status, err) {
			doneFunc(false);
		},
		complete: function(xhr, status) {
			sq_getStatus();
		}
	});
}

// runs a project
function sq_runProject(project, doneFunc) {
	
	// save first
	sq_saveCurrentProject(function(status) {
		
		if(!status) {
			doneFunc(false);
			return;
		}

		$.ajax({
			type: 'POST',
			dataType: 'text',
			url: sq_root + 'sqfish.php',
			data: {'COMMAND':'RUN', 'PROJECT':project},
			success: function(data, status, xhr) {
				var lines = data.split('\n');
				var stat = JSON.parse(lines[0]);
				if(stat.status == 'ok') {
					doneFunc(true);
				}
				else {
					doneFunc(false);
				}
			},
			error: function(xhr, status, err) {
				doneFunc(false);
			},
			complete: function(xhr, status) {
			}
		});
	});
}

function jsonEscape(str)  {
	return str.replace(/\n/g, "\\\\n").replace(/\r/g, "\\\\r").replace(/\t/g, "\\\\t");
}

function unescapeString(str) {
	var res = "";
	var i = 0;
	while(i < str.length) {
		var c = str.substr(i, 1);
		if(c == '#') {
			i++;
			res += String.fromCharCode(parseInt(str.substr(i, 2), 16));
			i++;
		}
		else
			res += c;
		i++;
	}
	return res;
}

// request status from server; re-spawn itself to query every 400 mSec
var inStatus = false;
function sq_getStatus() {
	
	// avoid re-entering if server takes too much time to answer
	if(inStatus)
		return;
	inStatus = true;
	
	$.ajax({
		type: 'POST',
		dataType: 'text',
		url: sq_root + 'sqfish.php',
		timeout:1000,
		data: {'COMMAND':'STATUS'},
		success: function(data, status, xhr) {
			var lines = data.split('\n');
			var stat = JSON.parse(lines[0]);
			if(stat.status == 'ok') {
				// parse the status answer
				lines.splice(0, 1);
				if(lines.length) {
					lines = lines.join("\n");
					if(lines != "") {
//						lines = jsonEscape(lines);
						try {
							stat = JSON.parse(lines);
							showStatus(stat);
						}
						catch(err) {
						}
					}
				}
			}
		},
		error: function(xhr, status, err) {
		},
		complete: function(xhr, status) {
			// prepare for next update
			window.setTimeout(function() {
				sq_getStatus();}, 400
			);
			inStatus = false;
		}
	});
}

$.fn.textWidth = function(text, font) {
    if (!$.fn.textWidth.fakeEl) $.fn.textWidth.fakeEl = $('<span>').hide().appendTo(document.body);
    $.fn.textWidth.fakeEl.text(text || this.val() || this.text()).css('font', font || this.css('font'));
    return $.fn.textWidth.fakeEl.width();
};

// get width of an array of list elements
function getListWidth(items, font) {
	var maxWidth = 0;
	items.forEach(function(item) {
		var itemWidth = $.fn.textWidth(item, font);
		if(itemWidth > maxWidth)
			maxWidth = itemWidth;
	});
	return maxWidth;
}

// check if a variable is undefined
function sq_checkdef(item) {
	if(typeof(item) == 'undefined')
		alert('undefined!');
}

// temp -- show status (by now just terminal output)
function showStatus(stat) {

	var content = stat['term'];
	if(typeof(content) != 'undefined') {
		var term = $('#terminal').terminal();
		content = unescapeString(content);
		content = content.split("\n");
		var h = term.height();
		content.forEach(function(item) {
			term.echo(item);
		});
		term.height(h);
	}
	var ram = stat['ram'];
	if(typeof(ram) == 'undefined')
		ram = 0;
	else
		ram = parseInt(ram);
	var ramItem = $('#ramValue');
	if(ram < 5000) {
		ramItem.css("background-color", "red");
		ramItem.css("color", "white");
	}
	else {
		ramItem.css("background-color", "white");
		ramItem.css("color", "black");
	}
	if(ram >= 1000)
		ram = (ram / 1000) + " kb";
	else
		ram = ram + " bytes";
	ramItem.text(ram);
}
