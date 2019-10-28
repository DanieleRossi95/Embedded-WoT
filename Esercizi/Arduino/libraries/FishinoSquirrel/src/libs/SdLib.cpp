#include "SdLib.h"

#include <FishinoSdFat.h>

using namespace sqfish;

class SqSd
{
	private:
		
	protected:
		
	public:
		
};

class SqFile : public SqStream
{
	private:
		
		SdFile _file;
		bool _opened;
		
	protected:
		
	public:

		SqFile();
		SqFile(const char *path);
		SqFile(const char *path, int mode);
		
		virtual ~SqFile();
		
		bool open(const char *path);
		bool open(const char *path, int mode);
		
		void close(void);

		virtual void  flush(void);
		virtual size_t  write(char c);
		virtual size_t  write(const char *buf, size_t len);
		virtual int  read(void);
		virtual size_t  read(char *buf, size_t maxLen);
		virtual int  available(void);
		virtual int  peek(void);
		virtual size_t  tell(void);
		virtual size_t  length(void);
		virtual bool  eof(void);
		virtual bool  opened(void);
		
		int seek(int pos);
		int seek(int pos, int whence);
};

SqFile::SqFile()
{
	_opened = false;
}

SqFile::SqFile(const char *path)
{
	_opened = _file.open(path, O_READ);
}

SqFile::SqFile(const char *path, int mode)
{
	_opened = _file.open(path, mode);
}

SqFile::~SqFile()
{
	if(_opened)
		close();
}

bool SqFile::open(const char *path)
{
	if(_opened)
		close();
	_opened = _file.open(path, O_READ);
	return _opened;
}

bool SqFile::open(const char *path, int mode)
{
	if(_opened)
		close();
	_opened = _file.open(path, mode);
	return _opened;
}

void SqFile::close(void)
{
	if(_opened)
		_file.close();
	_opened = false;
}

void  SqFile::flush(void)
{
	if(!_opened)
		return;
	_file.flush();
}

size_t  SqFile::write(char c)
{
	if(!_opened)
		return 0;
	return _file.write(c);
}

size_t  SqFile::write(const char *buf, size_t len)
{
	if(!_opened)
		return 0;
	return _file.write(buf, len);
}

int  SqFile::read(void)
{
	if(!_opened)
		return -1;
	return _file.read();
}

size_t  SqFile::read(char *buf, size_t maxLen)
{
	if(!_opened)
		return 0;
	return _file.read(buf, maxLen);
}

int  SqFile::available(void)
{
	if(!_opened)
		return 0;
	return _file.available();
}

int  SqFile::peek(void)
{
	if(!_opened)
		return -1;
	return _file.peek();
}

size_t  SqFile::tell(void)
{
	if(!_opened)
		return 0;
	return _file.curPosition();
}

size_t  SqFile::length(void)
{
	if(!_opened)
		return 0;
	return _file.fileSize();
}

bool  SqFile::eof(void)
{
	if(!_opened)
		return true;
	return _file.curPosition() >= _file.fileSize();
}

bool  SqFile::opened(void)
{
	return _opened;
}

int SqFile::seek(int pos)
{
	if(!_opened)
		return 0;
	int curPos = _file.curPosition();
	_file.seekSet(pos);
	return curPos;
}

int SqFile::seek(int pos, int whence)
{
	if(!_opened)
		return 0;
	int curPos = _file.curPosition();
	switch(whence)
	{
		case SEEK_SET:
		default:
			_file.seekSet(pos);
			break;
		case SEEK_CUR:
			_file.seekCur(pos);
			break;
		case SEEK_END:
			_file.seekEnd(pos);
			break;
	}
	return curPos;
}


SQUIRREL_API SQInteger registerSdLib(HSQUIRRELVM v)
{
	RootTable(v)
		.Class<SqSd>("SD"_LIT)
			--
		.Class<SqFile>("Stream"_LIT, "File"_LIT)
			.Constructor<>						()
			.Constructor<const char *>			()
			.Constructor<const char *, int>		()
			.Func<bool, const char *>			("open"_LIT		, &SqFile::open)
			.Func<bool, const char *, int>		("open"_LIT		, &SqFile::open)
			.Func								("close"_LIT	, &SqFile::close)
			.Func<int, int>						("seek"_LIT		, &SqFile::seek)
			.Func<int, int, int>				("seek"_LIT		, &SqFile::seek)
			--
		.Value("O_READ"_LIT		, O_READ)
		.Value("O_WRITE"_LIT	, O_WRITE)
		.Value("O_RDWR"_LIT		, O_RDWR)
		.Value("O_APPEND"_LIT	, O_APPEND)
		.Value("O_CREAT"_LIT	, O_CREAT)
		.Value("O_EXCL"_LIT		, O_EXCL)
		.Value("O_TRUNC"_LIT	, O_TRUNC)
		
		.Value("SEEK_SET"_LIT	, SEEK_SET)
		.Value("SEEK_CUR"_LIT	, SEEK_CUR)
		.Value("SEEK_END"_LIT	, SEEK_END)
	;
	return 0;
}
