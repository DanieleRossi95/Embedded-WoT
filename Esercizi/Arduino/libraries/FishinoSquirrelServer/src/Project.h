#ifndef __FISHINOSQUIRREL_PROJECT_H
#define __FISHINOSQUIRREL_PROJECT_H

#include <FishinoStl.h>

class Project
{
	private:
	
		// project name
		String _name;
	
		// project libraries
		std::vector<String> _libraries;

		// project files
		std::vector<String> _files;

		// error flag (if couldn't find library
		bool _error;
		
		// parser state
		int _state;
		
	protected:

	public:

		// constructor
		Project(const char *name) ;

		// destructor
		~Project() ;

		// copy constructor
		Project(Project const &p) ;
		
		// assignement
		Project &operator=(Project const &p) ;

		// parser internal callback
		void parserCb(uint8_t filter, uint8_t level, const char *name, const char *value) ;
		
		// error check
		 operator bool() const { return !_error; }
		bool  operator !() const { return _error; }
		bool  isError(void) const { return _error; }
		
		// testing -- dump project data
		void Dump(void) ;
};

#endif
