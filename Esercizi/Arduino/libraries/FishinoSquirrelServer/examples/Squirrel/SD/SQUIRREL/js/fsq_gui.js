"use strict";

//////////////////////////////////////////////////////////////////////
//                      TABS HANDLING FUNCTIONS                     //
//////////////////////////////////////////////////////////////////////

// fills a tabbar with array of path items
function sq_fillTabBar(tabId, pathItems, closeCross = true) {
	var tabItems = [];
	$.each(pathItems, function(i, pathItem) {
		var item = sq_getFileName(pathItem);
		var tabItem =
			'<li data-path="' + pathItem + '">' +
			'<a class="tab" onclick="sq_tabClickCb(\'' + tabId + '\', \'' + pathItem + '\');false;">' + item + '</a>';
		if(closeCross)
				tabItem += '<a class="close" onclick="sq_tabCloseCb(\'' + tabId + '\', \'' + pathItem + '\');false;">x</a>';
		tabItem += '</li>';
		tabItems.push(tabItem);
	});  // close each()
	$("#" + tabId + " li").remove();
	$("#" + tabId).append(tabItems);
}

// check if a tab is present
function sq_findTab(tabId, pathItem) {
	var found = false;
	$("#" + tabId).children("li").each(function(i) {
		if($(this).attr('data-path') == pathItem) {
			found = true;
			return false;
		}
	});
	return found;
}

// get selected tab
function sq_getSelectedTab(tabId) {
	var res = "";
	$("#" + tabId).children("li").each(function(i) {
		if($(this).hasClass('current')) {
			res = $(this).attr('data-path');
			return false;
		}
	});
	return res;
}

// append a tab to a tabbar
function sq_appendTab(tabId, pathItem, closeCross = true) {
	if(sq_findTab(tabId, pathItem))
		return;
	var item = sq_getFileName(pathItem);
	var tabItem =
		'<li data-path="' + pathItem + '">' +
		'<a class="tab" onclick="sq_tabClickCb(\'' + tabId + '\', \'' + pathItem + '\');false;">' + item + '</a>';
	if(closeCross)
			tabItem += '<a class="close" onclick="sq_tabCloseCb(\'' + tabId + '\', \'' + pathItem + '\');false;">x</a>';
	tabItem += '</li>';
	$("#" + tabId).append(tabItem);
}

// remove a tab from tabbar - return nearest tab
// useful to select another when closing
function sq_removeTab(tabId, pathItem) {
	var nextPath = "";
	$("#" + tabId).children("li").each(function(i) {
		var curPath = $(this).attr('data-path');
		if(curPath == pathItem) {
			$(this).remove();
			return false;
		}
		else
			nextPath = curPath;
	});
	
	// if we removed an un-selected tab, just leave it alone
	var curPath = sq_getSelectedTab(tabId);
	if(curPath != "")
		return curPath;
	
	// if we removed first tab, next path is empty
	// so we shall get first available tab
	if(nextPath == "") {
		var children = $("#" + tabId).children("li");
		if(children.length) {
			nextPath = children.first().attr('data-path');
		}
	}
	
	return nextPath;
}

// list all paths in tabs
function sq_listTabs(tabId) {
	var res = [];
	$("#" + tabId).children("li").each(function(i) {
		var curPath = $(this).attr('data-path');
		res.push(curPath);
	});
	return res;
}

// select a tab
function sq_selectTab(tabId, pathItem) {
	if(!sq_findTab(tabId, pathItem))
		sq_appendTab(tabId, pathItem);
		
	$("#" + tabId).children("li").each(function(i) {
		if($(this).attr('data-path') == pathItem)
			$(this).addClass("current");
		else
			$(this).removeClass("current");
	});
}

// deselect any tab
function sq_deselectTab(tabId) {
	$("#" + tabId).children("li").each(function(i) {
		$(this).removeClass("current");
	});
}

// fill projects tabbar
function sq_fillProjectTabs(items) {
	sq_fillTabBar("projectsTab", items);
}

// fill files tabbar
function sq_fillFilesTabs(items) {
	sq_fillTabBar("filesTab", items);
}

// fill messages tabbar
function sq_fillMessageTabs() {
	sq_fillTabBar("messageTab", ['Messages', 'Terminal'], false);
}

//////////////////////////////////////////////////////////////////////
//                      LISTS HANDLING FUNCTIONS                    //
//////////////////////////////////////////////////////////////////////

// fills a list, setting names, icons and paths
// elements must be an array of objects {name:name, icon:icon, path:path}
// all of them can be missing
function sq_fillList(listId, elements, clickFunction) {
	var list = $("#" + listId);
	list.children().remove();
	$.each(elements, function(i, element) {
		var name = element.name;
		var icon = element.icon;
		var path = element.path;
		if(!name && path)
			name = sq_getFileName(path);
		var listItem = '<li';
		if(path)
			listItem += ' data-path="' + path + '"';
		listItem += '>';
		listItem += '<a><div>';
		if(icon) {
			listItem += '<div><img src="' + icon + '"></div>';
		}
		listItem += "<div>" + name + "</div>";
		listItem += '</div></a></li>';
		list.append(listItem);
		var last = list.children("li").last().children("a");
		last.click(function() { clickFunction(name, path); false; });
	});  // close each()
}

// check if a tab is present
function sq_findListItem(listId, pathItem) {
	var found = false;
	$("#" + listId).children("li").each(function(i) {
		if($(this).attr('data-path') == pathItem)
			found = true;
	});
	return found;
}

// get selected listId
function sq_getSelectedListItem(listId) {
	var res = "";
	$("#" + listId).children("li").each(function(i) {
		if($(this).hasClass('current')) {
			res = $(this).attr('data-path');
			return false;
		}
	});
	return res;
}

// append an item to a list
function sq_appendListItem(listId, pathItem) {
	if(sq_findListItem(listId, pathItem))
		return;
	var item = sq_getFileName(pathItem);
	var listItem =
		'<li data-path="' + pathItem + '">' +
		'<a class="tab" onclick="sq_listClickCb(\'' + tabId + '\', \'' + pathItem + '\');false;">' + item + '</a>';
	listItem += '</li>';
	$("#" + listId).append(listItem);
}

// remove an item from list
function sq_removeListItem(listId, pathItem) {
	$("#" + listId).children("li").each(function(i) {
		if($(this).attr('data-path') == pathItem)
			$("#" + listId).remove(this);
	});
}

// select a list item
function sq_selectListItem(listId, pathItem) {
	$("#" + listId).children("li").each(function(i) {
		if($(this).attr('data-path') == pathItem)
			$(this).addClass("current");
		else
			$(this).removeClass("current");
	});
}

//////////////////////////////////////////////////////////////////////
//                         CONTEXT MENUS                            //
//////////////////////////////////////////////////////////////////////

// build libraries context menu
function buildLibrariesContextMenu() {
	
	$(function(){
		$.contextMenu({
			selector: '#librariesList li',
			build:function($trigger, e) {
				var libName = $trigger.attr('data-path');
				var items = {};
				if(libName == '<Project>') {
					items["addFile"]			= {
						name: "Add file to project",
						callback: function() { sq_addProjectFileCb(); }
					};
					items["addNewFile"]			= {
						name: "Add new file to project",
						callback: function() { sq_addProjectNewFileCb(); }
					};
					items["sep1"]		= "---------";
					items["addLib"]			= {
						name: "Add library to project",
						callback: function() { sq_addLibraryCb(); }
					};
					items["addNewLib"]		= {
						name: "Add new library to project",
						callback: function() { sq_addNewLibraryCb(); }
					};
					items["sep2"]		= "---------";
					items["delProject"]		= {
						name: "Delete project",
						callback: function() { sq_deleteProjectCb(); }
					};
				}
				else {
					items["addFile"]			= {
						name: "Add .file to '" + libName + "'",
						callback: function() { sq_addLibraryFileCb(libName); }
					};
					items["addNewFile"]			= {
						name: "Add new file to '" + libName + "'",
						callback: function() { sq_addLibraryNewFileCb(libName); }
					};
					items["sep1"]		= "---------";
					items["remLib"]		= {
						name: "Remove library from project",
						callback: function() { sq_removeLibraryCb(libName); }
					};
					items["delLib"]		= {
						name: "Delete library",
						callback: function() { sq_deleteLibraryCb(libName); }
					};
					items["sep2"]		= "---------";
					items["libDeps"]	= {
						name: "Manage library dependencies",
						callback: function() { sq_libraryDepsCb(libName); }
					};
				}
				if(Object.keys(items).length == 0)
					return false;
				return {
					zIndex:10,
					items: items
				};
			}
		});
	});
}

// build files context menu
function buildFilesContextMenu() {
	
	$(function(){
		$.contextMenu({
			selector: '#files li',
			build:function($trigger, e) {
				var filePath = $trigger.attr('data-path');
				var fileName = sq_getFileName(filePath);
				var libName = sq_getSelectedListItem("librariesList");
				var items = {};
				if(libName == '<Project>') {
					if(sq_getFileTitle(fileName) != sq_currentProject || sq_getFileExt(fileName) != '.nut') {
						items["remFile"]	= {
							name: "Remove '" + fileName + "' from project",
							callback: function() { sq_removeProjectFileCb(filePath); }
						};
						items["delFile"]	= {
							name: "Remove '" + fileName + "' from project and delete it",
							callback: function() { sq_deleteProjectFileCb(filePath); }
						};
					}
				}
				else {
					if(sq_getFileTitle(fileName) != libName || sq_getFileExt(fileName) != '.nut') {
						items["remFile"]	= {
							name: "Remove '" + fileName + "' from '" + libName + "'",
							callback: function() { sq_removeLibraryFileCb(libName, filePath); }
						};
						items["delFile"]	= {
							name: "Remove '" + fileName + "' from '" + libName + "' and delete it",
							callback: function() { sq_deleteLibraryFileCb(libName, filePath); }
						}
					}
				}
				if(Object.keys(items).length == 0)
					return false;
				return {
					zIndex:10,
					items: items
				};
			}
		});
	});
}

//////////////////////////////////////////////////////////////////////
//                    IDE SYNC ON CURRENT FILE                      //
//////////////////////////////////////////////////////////////////////
function sq_syncIDE(syncFilesListItem) {

	// if no project selected, hide the whole project area and return
	if(sq_currentProject == "") {
		
		// hide project area
		$("#projectArea").css("visibility", "hidden");
		
		// deselect any project tab
		sq_deselectTab("projectsTab");
		return;
	}
	
	// show project area
	$("#projectArea").css("visibility", "visible");
	
	// check if we changed project tab; in case force file sync
	var currentProjectTab = sq_getSelectedTab("projectsTab");
	if(currentProjectTab != sq_currentProject)
		syncFilesListItem = true;
	
	// activate current project's tab
	sq_selectTab("projectsTab", sq_currentProject);

	// if there's a current file selected, show code area
	// otherwise hide it
	if(sq_currentFile != "") {
		$("#codeArea").show();
	}
	else {
		$("#codeArea").hide();
	}
	
	// store current library list item, re-fill list and re-set
	// selected item, if list didn't change
	var currentLibItem = sq_getSelectedListItem("librariesList");
	var keys = Object.keys(sq_projectLibs);
	var items = [{"name":"&lt;Project&gt;", "path":"<Project>", "icon":sq_imgRoot + "sq.png"}];
	keys.forEach(function(lib) {
		items.push({"path":lib, "icon":sq_imgRoot + "sqlib.png"});
	});
	sq_fillList("librariesList", items, function(item, path) {
		sq_listClickCb("librariesList", path);
	});
	sq_selectListItem("librariesList", currentLibItem);
	
	// if we want to sync library and files lists with current file
	// just do it
	if(syncFilesListItem) {
	
		// if there's a current file selected, find the right library
		// and file list item for it
		var files = [];
		var root = "";
		if(sq_currentFile != "") {
			if(sq_currentFile.startsWith(sq_projectsRoot)) {
				// it's a project file, select first lib item
				sq_selectListItem("librariesList", "<Project>");
				files = sq_currentProjectData.files;
				root = sq_projectsRoot + sq_currentProject + "/";
			}
			else {
				// get library for this file
				var libName = sq_currentFile.substring(sq_libRoot.length);
				libName = libName.split("/")[0];
				files = sq_projectLibs[libName].files;
				sq_selectListItem("librariesList", libName);
				root = sq_libRoot + libName + "/";
			}
			
			// fill files list
			var filesItems = [];
			files.forEach(function(file) {
				filesItems.push({"path":root + file, "icon":sq_icon(file)});
			});
			sq_fillList("filesList", filesItems, function(item, path) {
				sq_listClickCb("filesList", path);
			})
			
			// activate current files list item
			sq_selectListItem("filesList", sq_currentFile);
		}
		else {
			// no current file -- just select project files in libraries list
			// and fill files list
			sq_selectListItem("librariesList", "<Project>");
			files = sq_currentProjectData.files;
			root = sq_projectsRoot + sq_currentProject + "/";
			var filesItems = [];
			files.forEach(function(file) {
				filesItems.push({"path":root + file, "icon":sq_icon(file)});
			});
			sq_fillList("filesList", filesItems, function(item, path) {
				sq_listClickCb("filesList", path);
			})
		}
	}
	
	// otherwise just sync the files pane with current library selected
	// into library pane
	else {
		var lib = sq_getSelectedListItem("librariesList");
		if(lib != "") {
			if(lib == "<Project>") {
				files = sq_currentProjectData.files;
				root = sq_projectsRoot + sq_currentProject + "/";
			}
			else {
				files = sq_projectLibs[lib].files;
				root = sq_libRoot + lib + "/";
			}

			var filesItems = [];
			files.forEach(function(file) {
				filesItems.push({"path":root + file, "icon":sq_icon(file)});
			});
			sq_fillList("filesList", filesItems, function(item, path) {
				sq_listClickCb("filesList", path);
			})

			// activate current files list item if in list
			if(sq_currentFile.startsWith(root))
				sq_selectListItem("filesList", sq_currentFile);
		}
	}
}
