// some utilities
#include <Arduino.h>

#include <FishinoSdFat.h>

// appends a path to another
String appendPath(String const &p1, String const &p2) ;

// appends an extension to a path
String appendExt(String const &path, String const ext) ;

// appends a path and an extension to a path
String appendPathExt(String const &p1, String const &p2, String const ext) ;

// get file folder from full path
String getFileFolder(String const &path) ;

// get file name from full path
String getFileName(String const &path) ;

// get file title from full path
String getFileTitle(String const &path) ;

// get file extension from full path
String getFileExt(String const &path) ;

// create a folder with its parent folders from a full path
bool createFileFolder(String const &filePath) ;

// strip quotes from string
String stripQuotes(String s);

// escape a string to be sent to javascript
// chars < 0x20 are escaped as hex codes
String escapeString(const char *s) ;
