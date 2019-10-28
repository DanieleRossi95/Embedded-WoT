#ifndef __FISHINOSQUIRRELSERVER_SESSION_H
#define __FISHINOSQUIRRELSERVER_SESSION_H

#include <FishinoStl.h>

class Session
{
	private:

		// opened projects
		std::vector<String> _projects;
		
		// current project
		String  _activeProject;

		// error flag (if couldn't find library
		bool _error;
		
		// parser state
		int _state;
		
	protected:

	public:

		// parser internal callback
		void parserCb(uint8_t filter, uint8_t level, const char *name, const char *value) ;
		
		// constructor
		Session();

		// destructor
		~Session();
		
		// get the opened projects
		std::vector<String> const &getProjects(void) const { return _projects; }
		
		// get active project
		String const &getActiveProject(void) const { return _activeProject; }

		// error check
		operator bool() const { return !_error; }
		bool  operator !() const { return _error; }
		bool  isError(void) const { return _error; }
};

#endif
