// Library : 'FishinoSquirrel' -- File : 'Library.h'
// Created by FishIDE application 
#ifndef __FISHINOSQUIRREL_LIBRARY_H
#define __FISHINOSQUIRREL_LIBRARY_H

#include <FishinoStl.h>

class Library
{
	private:
	
		// library name
		String _name;
	
		// library files
		std::vector<String> _files;
		
		// library dependencies
		std::vector<String> _requires;
		
		// error flag (if couldn't find library
		bool _error;
		
		// parser state
		int _state;
		
	protected:

	public:

		// constructor
		Library(const char *name) ;
		
		// copy constructor
		Library(Library const &l) ;
		
		// assignement
		Library &operator=(Library const &l) ;

		// destructor
		~Library() ;
		
		// parser internal callback
		void parserCb(uint8_t filter, uint8_t level, const char *name, const char *value) ;
		
		// error check
		 operator bool() const { return !_error; }
		bool  operator !() const { return _error; }
		bool  isError(void) const { return _error; }
		
		std::vector<String> const  &getRequires() const { return _requires; }
		std::vector<String> const  &getFiles() const { return _files; }
};

#endif
