"use strict";

//////////////////////////////////////////////////////////////////////
//                     INITIALIZATION AND MAIN                      //
//////////////////////////////////////////////////////////////////////

// initialize ide subsystem
function sq_init() {
	
	// restore last session
	sq_loadFile(sq_root + 'last.session', function(status, data) {
		if(status) {
			data = JSON.parse(data);
			if(data) {
				// get opened project and last active project
				var projects = data.projects;
				var activeProject = data.activeProject;
				if(typeof(activeProject) == 'undefined') {
					if(projects && projects.length)
						activeProject = projects[0];
					else
						activeProject = "";
				}
				
				// fill projects tabs
				sq_fillProjectTabs(projects);
				
				// open last active project
				sq_openProject(activeProject, function(status) {
					if(!status)
						alert("Error opening project");
					sq_syncIDE(true);
				});
			}
			else {
				// hide project area
				$("#projectArea").css("display", "hidden");
				sq_syncIDE(true);
			}
		}
	});

	// fill messages tabs
	sq_fillMessageTabs();
	sq_selectTab('messageTab', 'Terminal');
	
	// side lists context menus
	buildLibrariesContextMenu();
	buildFilesContextMenu();

}

// A $( document ).ready() block.
$(document).ready(function() {

	sq_init();

});
