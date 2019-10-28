#include "utils.h"

// appends a path to another
String appendPath(String const &p1, String const &p2)
{
	String res = p1;
	if(!res.endsWith("/"))
		res += "/";
	return res + p2;
}

// appends an extension to a path
// returns a dynamic buffer that MUST be freed
String appendExt(String const &path, String const ext)
{
	return path + ext;
}

// appends a path and an extension to a path
// returns a dynamic buffer that MUST be freed
String appendPathExt(String const &p1, String const &p2, String const ext)
{
	return appendExt(appendPath(p1, p2), ext);
}

// get file folder from full path
String getFileFolder(String const &path)
{
	const char *s = strrchr(path.c_str(), '/');
	if(s)
		return path.substring(0, s - path.c_str() + 1);
	else
		return path + "/";
}

// get file name from full path
String getFileName(String const &path)
{
	const char *s = strrchr(path.c_str(), '/');
	if(s)
		return path.substring(s - path.c_str() + 1);
	else
		return path;
}

// get file title from full path
String getFileTitle(String const &path)
{
	String name = getFileName(path);
	const char *s = strrchr(name.c_str(), '.');
	if(s)
		return name.substring(0, s - name.c_str());
	else
		return name;
}

// get file extension from full path
String getFileExt(String const &path)
{
	const char *s = strrchr(path.c_str(), '.');
	if(s)
		return path.substring(s - path.c_str());
	else
		return "";
}

// create a folder with its parent folders from a full path
bool createFileFolder(String const &filePath)
{
	String folder = getFileFolder(filePath);
	if(!folder.endsWith("/"))
		return true;

	SdFile root;
	root.open("/", O_WRITE);
	SdFile f;
	f.mkdir(&root, folder.c_str(), true);
	f.close();
	root.close();
	return true;
}

// strip quotes from string
String stripQuotes(String s)
{
	if(s.startsWith("\""))
		s = s.substring(1);
	if(s.endsWith("\""))
		s = s.substring(0, s.length() - 1);
	return s;
}


// escape a string to be sent to javascript
// all non-alphanumeric chars are escaped
String escapeString(const char *s)
{
	String res;
	while(*s)
	{
		if(isalnum(*s))
			res += *s++;
		else
		{
			res += "#";
			uint8_t b;
			b = (*s >> 4) & 0x0f;
			if(b <= 9)
				res += (char)(b + '0');
			else
				res += (char)(b - 10 + 'a');
			b = *s & 0x0f;
			if(b <= 9)
				res += (char)(b + '0');
			else
				res += (char)(b - 10 + 'a');
			s++;
		}
	}
	return res;
}
