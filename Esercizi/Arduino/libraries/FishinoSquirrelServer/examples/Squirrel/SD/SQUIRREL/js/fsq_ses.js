"use strict";

//////////////////////////////////////////////////////////////////////
//                         SESSION MANAGEMENT                       //
//////////////////////////////////////////////////////////////////////

// save current session
// @@ later we shall put it in a timed function
// to avoid too many uploads
function sq_saveSession(doneFunc) {
	
	var projects = [];
	$("#projectsTab").children("li").each(function(i) {
		var path = $(this).attr('data-path');
		projects.push(path);
	});

	var current = sq_currentProject;
	if(!current)
		current = "";
//	var current = $("#projectsTab .current");
//	if(current)
//		current = current.attr('data-path');
//	else
//		current = "";

	var session = { 'projects':projects, 'activeProject':current }
	sq_saveFile(sq_root + 'last.session', sq_stringify(session), function(status) {
		doneFunc(status);
	});
}
