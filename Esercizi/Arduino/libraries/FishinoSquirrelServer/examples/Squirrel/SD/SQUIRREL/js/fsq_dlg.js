"use strict";

// start a select/create folder dialog
// the callback has 2 parameters :
//		status  = true for OK, false for CANCEL
//		name    = the folder name (without rootFolder path)
function sq_folderDialog(kind, title, rootFolder, iconImage, excludes, doneFunc) {
	
	// depending on dialog kind, put a titl on list
	var listFunc;
	switch(kind) {
		case 'newProject':
			$('#dialogListHeader').html("Available projects:");
			$('#folderDialogNameBlock').show();
			$('#folderNameLabel').html("Enter a project name:");
			listFunc = function(name) {};
			break;
			
		case 'openProject':
			$('#dialogListHeader').html("Select project:");
			$('#folderDialogNameBlock').hide();
			listFunc = function(name) {
				$('#folderDialog').dialog('close');
				doneFunc(true, name);
			};

			break;
			
		case 'newLib':
			$('#dialogListHeader').html("Available libraries:");
			$('#folderDialogNameBlock').show();
			$('#folderNameLabel').html("Enter a library name:");
			listFunc = function(name) {};
			break;
			
		case 'openLib':
			$('#dialogListHeader').html("Select a library:");
			$('#folderDialogNameBlock').hide();
			listFunc = function(name) {
				$('#folderDialog').dialog('close');
				doneFunc(true, name);
			};
			break;
			
		default:
			alert("Unknown folder dialog type");
			return;
	}

	// get a list of available folders under root folder
	sq_listFolder(rootFolder, true, function(status, path, elements) {
		if(status) {
			// fill dialog list with folder names
			var items = [];
			elements.forEach(function(element) {
				if(typeof(excludes) == 'undefined' || excludes.indexOf(element) < 0)
					items.push({'name':element, 'icon': sq_imgRoot + iconImage});
			});
			sq_fillList('folderDialogList', items, listFunc);
			
			// get list width (max item's width) adjusting with image and margin
			var listWidth = getListWidth(elements, '16px Arial, Helvetica') + 42;
			
			// limit width to dialog size and force all list elements
			// to be of that width, so theyre spaced evenly
			var dialogWidth = window.innerWidth * 2 / 3 - 20;
			if(listWidth > dialogWidth)
				listWidth = dialogWidth;
			var listItems = $('#folderDialogList').children();
			listItems.css("maxWidth", listWidth + 'px');
			listItems.css("flex-basis", listWidth + 'px');
			
			// create project dialog and run it
			var buttons = [];
			if(kind == 'newProject' || kind == 'newLib')
				buttons.push({
					text:'Ok',
					icon: "ui-icon-ok",
					click:function() {
						var name = $("#folderName").val();
						var valid = /^(?!\.)(?!com[0-9]$)(?!con$)(?!lpt[0-9]$)(?!nul$)(?!prn$)[^\|\*\?\\:<>/$"]*[^\.\|\*\?\\:<>/$"]+$/i.test(name);
						if(!valid)
							alert("Invalid project name!");
						else if($.inArray(name, elements) >= 0)
							alert("Project '" + name + "' already exists!");
						else {
							$(this).dialog('close');
							doneFunc(true, name);
						}
					}
				});
			buttons.push({
				text:'Cancel',
				icon: "ui-icon-cancel",
				click:function() {
					$(this).dialog('close');
					doneFunc(false);
				}
			});
			$('#folderDialog').dialog({
				autoOpen:true,
				width: window.innerWidth * 2 / 3,
				height: window.innerHeight * 2 / 3,
				buttons: buttons,
				classes: {
					"ui-dialog": "ui-corner-all ui-widget-shadow",
				},
				modal:true,
				title:title,
				resizable:false
			});
		}
		else
			doneFunc(false);
	});
}


// start a select/create file dialog
// the callback has 2 parameters :
//		status  = true for OK, false for CANCEL
//		name    = the folder name (without rootFolder path)
function sq_fileDialog(kind, title, rootFolder, includes, excludes, doneFunc) {
	
	// depending on dialog kind, put a title on list
	var listFunc;
	switch(kind) {
		case 'newFile':
			$('#dialogListHeader').html("Available files:");
			$('#fileDialogNameBlock').show();
			$('#fileNameLabel').html("Enter a file name:");
			listFunc = function(name) {};
			break;
			
		case 'openFile':
			$('#dialogListHeader').html("Select file:");
			$('#fileDialogNameBlock').hide();
			listFunc = function(name) {
				$('#fileDialog').dialog('close');
				doneFunc(true, name);
			};
			break;
			
		default:
			alert("Unknown file dialog type");
			return;
	}

	// get a list of available folders under root folder
	sq_listFolder(rootFolder, false, function(status, path, elements) {
		if(status) {
			// fill dialog list with file names
			var items = [];
			elements.forEach(function(element) {
				var ext = sq_getFileExt(element);
				if(
					(typeof(includes) == 'undefined' || includes == "*" || includes.indexOf(ext) >= 0) &&
					(typeof(excludes) == 'undefined' || excludes.indexOf(ext) < 0)
				)
					items.push({'name':element, 'icon': sq_icon(ext)});
			});
			sq_fillList('fileDialogList', items, listFunc);
			
			// get list width (max item's width) adjusting with image and margin
			var listWidth = getListWidth(elements, '16px Arial, Helvetica') + 42;
			
			// limit width to dialog size and force all list elements
			// to be of that width, so theyre spaced evenly
			var dialogWidth = window.innerWidth * 2 / 3 - 20;
			if(listWidth > dialogWidth)
				listWidth = dialogWidth;
			var listItems = $('#fileDialogList').children();
			listItems.css("maxWidth", listWidth + 'px');
			listItems.css("flex-basis", listWidth + 'px');
			
			// create project dialog and run it
			var buttons = [];
			if(kind == 'newFile')
				buttons.push({
					text:'Ok',
					icon: "ui-icon-ok",
					click:function() {
						var name = $("#fileName").val();
						var valid = /^(?!\.)(?!com[0-9]$)(?!con$)(?!lpt[0-9]$)(?!nul$)(?!prn$)[^\|\*\?\\:<>/$"]*[^\.\|\*\?\\:<>/$"]+$/i.test(name);
						if(!valid)
							alert("Invalid file name!");
						else if($.inArray(name, elements) >= 0)
							alert("File '" + name + "' already exists!");
						else {
							$(this).dialog('close');
							doneFunc(true, name);
						}
					}
				});
			buttons.push({
				text:'Cancel',
				icon: "ui-icon-cancel",
				click:function() {
					$(this).dialog('close');
					doneFunc(false);
				}
			});
			$('#fileDialog').dialog({
				autoOpen:true,
				width: window.innerWidth * 2 / 3,
				height: window.innerHeight * 2 / 3,
				buttons: buttons,
				classes: {
					"ui-dialog": "ui-corner-all ui-widget-shadow",
				},
				modal:true,
				title:title,
				resizable:false
			});
		}
		else
			doneFunc(false);
	});
}

// display yes/no dialog
function sq_yesNoDlg(title, htmlText, doneFunc) {

	$("#yesNoText").html(htmlText);
	
	$('#yesNoDialog').dialog({
		autoOpen:true,
		width: "auto",
		buttons: [
			{
				text:'Yes',
				icon: "ui-icon-ok",
				click:function() {
					$(this).dialog('close');
					doneFunc(true);
				}
			},
			{
				text:'No',
				icon: "ui-icon-cancel",
				click:function() {
					$(this).dialog('close');
					doneFunc(false);
				}
			}
		],
		classes: {
			"ui-dialog": "ui-corner-all ui-widget-shadow",
		},
		modal:true,
		title:title,
		resizable:false
	});
}

// display a library dependencies dialog
// doneFunc gets called with 2 parameters:
// - status
// new dependencies, array
function sq_libraryDepsDialog(libName, deps, doneFunc) {
	// get a list of available libraries
	sq_listFolder(sq_libRoot, true, function(status, path, libraries) {
		if(!status) {
			doneFunc(false);
			return;
		}
		
		var content = "";
		libraries.forEach(function(item) {
			if(item != libName) {
				content += '<div>\n';
				content += '<label for="lib_' + item + '"><img src="img/sqlib.png">' + item + '</label>\n';
				content += '<input type="checkbox" name="lib' + item + '" id="lib' + item + '"';
				if(deps.indexOf(item) >= 0)
					content += ' checked';
				content += '>\n';
				content += '</div>\n';
			}
		})
		$("#libraryDepsList").html(content);

		$('#libraryDepsDialog').dialog({
			autoOpen:true,
			width: window.innerWidth * 2 / 3,
			height: window.innerHeight * 2 / 3,
			buttons: [
				{
					text:'Ok',
					icon: "ui-icon-ok",
					click:function() {
						$(this).dialog('close');
						var deps = [];
						$("#libraryDepsList input").each(function() {
							if($(this).is(':checked'))
								deps.push($(this).attr('name').substring(3));
						});
						doneFunc(true, deps);
					}
				},
				{
					text:'Cancel',
					icon: "ui-icon-cancel",
					click:function() {
						$(this).dialog('close');
						doneFunc(false);
					}
				}
			],
			classes: {
				"ui-dialog": "ui-corner-all ui-widget-shadow",
			},
			modal:true,
			title:'Change libraries dependencies',
			resizable:false
		});
	});
}
