//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishGram.cpp										//
//				Library to handle Telegram messaging with Fishino					//
//	Copyright (c) 2017 Massimo Del Fedele and Andrea S. Costa. All right reserved	//
//																					//
//	Redistribution and use in source and binary forms, with or without				//
//	modification, are permitted provided that the following conditions are met:		//
//																					//
//	- Redistributions of source code must retain the above copyright notice,		//
//	  this list of conditions and the following disclaimer.							//
//	- Redistributions in binary form must reproduce the above copyright notice,		//
//	  this list of conditions and the following disclaimer in the documentation		//
//	  and/or other materials provided with the distribution.						//
//																					//	
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"		//
//	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE		//
//	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE		//
//	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE		//
//	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR				//
//	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF			//
//	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS		//
//	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN			//
//	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)			//
//	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE		//
//	POSSIBILITY OF SUCH DAMAGE.														//
//																					//
//	VERSION 1.0.0 --	INITIAL VERSION												//
//  Version 6.0.2 --	June 2017													//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#define DEBUG_LEVEL_ERROR
#include <FishinoDebug.h>

#include "FishGram.h"

#include <FishinoFlash.h>

#define TELEGRAM_HOST	"api.telegram.org"
#define TELEGRAM_PORT	443

// external parser callback
// just calls the internal one
void __JSONCallback(uint8_t filter, uint8_t level, const char *name, const char *value, void *cbObj)
{
	// call FishGramClass internal handler
	((FishGramClass *)cbObj)->JSONCallback(filter, level, name, value);
}

// helper to copy a string removing the "" enclosing quotes
static char *strdupNoQuotes(const char *s)
{
	if (!s)
		return NULL;
	if (*s == '"')
		s++;
	if (!*s)
		return NULL;
	size_t len = strlen(s);
	if (s[len - 1] == '"')
		len--;
	char *res = (char *)malloc(len + 1);
	memcpy(res, s, len);
	res[len] = 0;
	return res;
}

// choose picture resolution -- called when receiving new picture format
void FishGramClass::setLastImage(void)
{
	if(_lastFile)
		free(_lastFile);
	_lastFile = _prevFile;
	_prevFile = NULL;
	_lastImgWidth = _prevImgWidth;
	_lastImgHeight = _prevImgHeight;
}

void FishGramClass::chooseResolution(void)
{
	// if this one is the first record, just store it
	if(!_lastFile)
	{
		setLastImage();
		return;
	}
	
	// not the first one. If we gave an optimal resolution, check if this one is better
	// the 'better' resolution is the one that fits best the display,
	// being bigger or equal to its size
	if(_reqImgWidth && _reqImgHeight)
	{
		// adjust orientation
		uint16_t w, h;
		if(
			(_reqImgWidth > _reqImgHeight && _prevImgWidth > _prevImgHeight) ||
			(_reqImgWidth < _reqImgHeight && _prevImgWidth < _prevImgHeight)
		)
		{
			w = _reqImgWidth;
			h = _reqImgHeight;
		}
		else
		{
			w = _reqImgHeight;
			h = _reqImgWidth;
		}
		
		// if previous image was smaller then requested, replace it
		if((_lastImgWidth < w || _lastImgHeight < h) && _prevImgWidth > _lastImgWidth && _prevImgHeight > _lastImgHeight)
			setLastImage();
		// otherwise check if this one fits better
		else if(_prevImgWidth >= w && _prevImgWidth < _lastImgWidth && _prevImgHeight >= h && _prevImgHeight < _lastImgHeight)
			setLastImage();
	}
	// otherwise just select the hightest resolution available
	else if(_prevImgWidth > _lastImgWidth || _prevImgHeight > _lastImgHeight)
		setLastImage();
}

// the internal callback
void FishGramClass::JSONCallback(uint8_t filter, uint8_t level, const char *name, const char *value)
{
	DEBUG_INFO("Level:%u, name:%s, value:%s\n", level, name, value);

	if (!strcmp(name, F("ok")))
	{
		_ok = !strcmp(value, F("true"));
		return;
	}

	// when sending, we don't care about other data
	if (_sending)
		return;
	
	// special processing for 'getFile' query
	if(_gettingFile)
	{
		if (!strcmp(name, F("file_path")))
		{
			if (_filePath)
				free(_filePath);
			_filePath = strdupNoQuotes(value);
		}
		else if (!strcmp(name, F("file_size")))
		{
			sscanf(value, "%lu", &_fileSize);
		}
		return;
	}
	
	// drop vars with level < 2
	if(level < 2)
	{
		setSubState(FishGramOut);
		return;
	}
	
	// on level == 2 accept just 'update_id' and 'message' vars
	if(level == 2)
	{
		// empty value means a level back
		if(!*name && JSONStreamingParser::isSeqEnd(value))
			setSubState(FishGramOut);
		else if (!strcmp(name, F("update_id")))
		{
			// store update id
			sscanf(value, "%lu", &_lastUpdateId);
			_lastUpdateId++;
		}
		else if(!strcmp(name, F("message")))
			setSubState(FishGramInMessage);
		return;
	}
	
	// on level == 3 we're interested in many more things :
	// 'from', 'photo', 'caption', 'text' and 'voice'
	if(level == 3) // we shall be in message level here
	{
		// empty value means a level back
		// the same if in a bad state (shall be FishGramInMessage here)
		if((!*name  && JSONStreamingParser::isSeqEnd(value)) || _subState != FishGramInMessage)
			setSubState(FishGramOut);
		else if(!strcmp(name, F("from")))
		{
			// switch to 'FishGramInFrom' substate
			setSubState(FishGramInFrom);
		}
		else if(!strcmp(name, F("photo")) && _pictureEvent)
		{
			// switch to 'FishGramInPhoto' substate
			setSubState(FishGramInPhoto);
			
			// clear any previous photo data
			if(_prevFile)
				free(_prevFile);
			_prevFile = NULL;
			_prevImgWidth = 0;
			_prevImgHeight = 0;
			if(_lastFile)
				free(_lastFile);
			_lastFile = NULL;
			_lastImgWidth = 0;
			_lastImgHeight = 0;
		
			// signal we've got a picture
			_gotElement |= FishGramPhoto;
		}
		else if(!strcmp(name, F("text")) && _messageEvent)
		{
			if (_lastMessage)
				free(_lastMessage);
			_lastMessage = strdupNoQuotes(value);
			
			// signal we've got a text
			_gotElement |= FishGramText;
		}
		else if(!strcmp(name, F("caption")) && _pictureEvent)
		{
			if (_lastMessage)
				free(_lastMessage);
			_lastMessage = strdupNoQuotes(value);
			
			// don't signal here, we do it in picture field
		}
		else if(!strcmp(name, F("voice")) && _audioEvent)
		{
			// switch to 'FishGramInVoice' substate
			setSubState(FishGramInVoice);

			// signal we've got a voice message
			_gotElement |= FishGramAudio;
		}
		return;
	}
	
	// on level 4 we analyze the various elements depending on our subState
	if(level == 4)
	{
		// empty value means a level back
		if(!*name && JSONStreamingParser::isSeqEnd(value))
			setSubState(FishGramInMessage);

		// we're reading 'from' data ?
		else if(_subState == FishGramInFrom)
		{
			// read sender data
			if (!strcmp(name, F("id")))
				sscanf(value,  "%lu", &_lastSenderId);
			else if (!strcmp(name, F("first_name")))
			{
				if (_lastSenderFirstName)
					free(_lastSenderFirstName);
				_lastSenderFirstName = strdupNoQuotes(value);
			}
			else if (!strcmp(name, F("last_name")))
			{
				if (_lastSenderLastName)
					free(_lastSenderLastName);
				_lastSenderLastName = strdupNoQuotes(value);
			}
		}

		// we're reading 'picture' data ?
		else if(_subState == FishGramInPhoto)
		{
			// do nothing here, we shall collect and select
			// from various picture resolutions
		}

		// we're reading 'voice' data ?
		else if(_subState == FishGramInVoice)
		{
			// for audio files, we assume it's an ogg one
			// so we're interested just on file id
			if (!strcmp(name, F("file_id")))
			{
				if (_lastFile)
					free(_lastFile);
				_lastFile = strdupNoQuotes(value);
			}
		}
		return;
	}
	
	// on level 5 we shall be inside picture formats
	// so we read them and select the best one
	if(level == 5)
	{
		// on level termination, we shall choose picture resolution
		if(!*name && JSONStreamingParser::isSeqEnd(value))
			chooseResolution();
		else if(!strcmp(name, F("file_id")))
		{
			if (_prevFile)
				free(_prevFile);
			_prevFile = strdupNoQuotes(value);
		}
		else if(!strcmp(name, F("width")))
		{
#ifdef _FISHINO_PIC32_
			sscanf(value, "%hu", &_prevImgWidth);
#else
			sscanf(value, "%u", &_prevImgWidth);
#endif
		}
		else if(!strcmp(name, F("height")))
		{
#ifdef _FISHINO_PIC32_
			sscanf(value, "%hu", &_prevImgHeight);
#else
			sscanf(value, "%u", &_prevImgHeight);
#endif
		}
	}
}

// state setter -- for debug purposes
void FishGramClass::setState(States s)
{
	_state = s;
#ifdef DEBUG_LEVEL_INFO
		DEBUG_INFO("\nSetting state to '");
		switch(_state)
		{
			case FishGramOff:
				DEBUG_INFO("FishGramOff");
				break;
			case FishGramRestart:
				DEBUG_INFO("FishGramRestart");
				break;
			case FishGramIdle:
				DEBUG_INFO("FishGramIdle");
				break;
			case FishGramListen:
				DEBUG_INFO("FishGramListen");
				break;
			case FishGramHeadersStartLine:
				DEBUG_INFO("FishGramHeadersStartLine");
				break;
			case FishGramHeaders:
				DEBUG_INFO("FishGramHeaders");
				break;
			case FishGramJSon:
				DEBUG_INFO("FishGramJSon");
				break;
			default:
				DEBUG_INFO("UNKNOWN");
		}
		DEBUG_INFO_N("'\n");
#endif
}

void FishGramClass::setSubState(SubStates s)
{
	_subState = s;
	/*
		Serial << F("\nSetting substate to '");
		switch(_subState)
		{
			case FishGramNone:
				Serial << F("FishGramNone");
				break;
			case FishGramInMessage:
				Serial << F("FishGramInMessage");
				break;
			case FishGramInPhoto:
				Serial << F("FishGramInPhoto");
				break;
			case FishGramInPhotoRes:
				Serial << F("FishGramInPhotoRes");
				break;
			case FishGramInVoice:
				Serial << F("FishGramInVoice");
				break;
			default:
				Serial << F("UNKNOWN");
		}
		Serial << "'\n";
	*/
}

// read char hook -- for debugging purposes
int FishGramClass::readChar(void)
{
	int c = _client.read();
//	Serial.print((char)c);
	return c;
}

// connect to telegram server, with some retry on failuse
bool FishGramClass::connect(void)
{
	for (int i = 0; i < 5; i++)
	{
		if (_client.connect(TELEGRAM_HOST, TELEGRAM_PORT))
		{
			DEBUG_INFO_FUNCTION("Connected\n");
			return true;
		}
		_client.stop();
		delay(200);
	}
	DEBUG_ERROR_FUNCTION("Connection failed\n");
	return false;
}

// run the query
bool FishGramClass::runQuery(int32_t id)
{
	DEBUG_INFO("runQuery()\n");
	if (!connect())
		return false;

	_client << F("GET /bot");
	DEBUG_INFO_N("GET /bot");
	if (_flashToken)
	{
		_client << (const __FlashStringHelper *)_token;
#ifdef DEBUG_LEVEL_INFO
		size_t len = strlen_P(_token);
		char *s = (char *)malloc(len + 1);
		strcpy_P(s, _token);
		DEBUG_INFO_N("%s", s);
		free(s);
#endif
	}
	else
	{
		_client << _token;
		DEBUG_INFO_N("%s", _token);
	}
	_client	<< F("/getUpdates?offset=");
	DEBUG_INFO_N("/getUpdates?offset=");
	_client.print(id, DEC);
	DEBUG_INFO_N("%d", id);
	_client
	<< F("&timeout=4&limit=1&allowed_updates=messages HTTP/1.1\r\n")
	<< F("User-Agent: FishGram 1.0.0\r\n")
	<< F("Host: api.telegram.org\r\n\r\n")
	;
	DEBUG_INFO_N("&timeout=4&limit=1&allowed_updates=messages HTTP/1.1\r\nUser-Agent: FishGram 1.0.0\r\nHost: api.telegram.org\r\n\r\n");
	
	return true;
}

// get file path from persistent ID
// return a dynamically allocated variable
// that must be freed by caller
bool FishGramClass::getFilePath(const char *id, char *&path, uint32_t &size)
{
	// initialize results
	path = NULL;
	size = 0;
	
	// if processing a query, wait till finished
	// (state must be idle)
	while (_state != FishGramIdle)
		loop();

	if (!connect())
		return false;
	_ok = false;

	_client << F("GET /bot");
	if (_flashToken)
		_client << (const __FlashStringHelper *)_token;
	else
		_client << _token;
	_client	<< F("/getFile?file_id=");
	_client.print(id);
	_client
		<< F(" HTTP/1.1\r\n")
		<< F("User-Agent: FishGram 1.0.0\r\n")
		<< F("Host: api.telegram.org\r\n\r\n")
	;

	// switch to waiting state and set getting file flag
	setState(FishGramListen);
	_gettingFile = true;

	// wait for completion
	while (_state != FishGramRestart)
		loop();
	
	if(_ok)
	{
		// store the path
		path = _filePath;
		size = _fileSize;
		_filePath = NULL;
	}
	
	while (_state != FishGramIdle)
		loop();
	_gettingFile = false;

	return _ok;
}

// process received message
PROGMEM_STRING(__filePathStart, "https://api.telegram.org/file/bot");
bool FishGramClass::processMessage(void)
{

	// if sender id is 0, do nothing
	if (!_lastSenderId)
		return false;

	// if we're restricting to allowed user list, check it
	if (_restrictUsers)
	{
		bool ok = false;
		for (uint16_t i = 0; i < _numAllowedUsers; i++)
			if (_allowedUsers[i] == _lastSenderId)
			{
				ok = true;
				break;
			}
		if (!ok)
			return false;
	}

	// check if id is on blocked list
	for (uint16_t i = 0; i < _numBlockedUsers; i++)
		if (_blockedUsers[i] == _lastSenderId)
			return false;
		
	bool res = false;
	
	// process text message
	if(_messageEvent && (_gotElement & FishGramText))
		res |= _messageEvent(_lastSenderId, _lastSenderFirstName, _lastSenderLastName, _lastMessage);

	// process picture/audio message
	if(
		(_pictureEvent && (_gotElement & FishGramPhoto)) ||
		(_audioEvent && (_gotElement & FishGramAudio))
	)
	{
		// both need the file to be retrieved from Telegram
		// as doing it re-enter the loop, we shall save needed vars
		// and restore them later
		char *senderFirstName = _lastSenderFirstName;
		char *senderLastName = _lastSenderLastName;
		char *message = _lastMessage;
		char *fileId = _lastFile;
		_lastSenderFirstName = _lastSenderLastName = _lastMessage = _lastFile = NULL;
		uint32_t senderId = _lastSenderId;
		uint16_t imgWidth = _lastImgWidth;
		uint16_t imgHeight = _lastImgHeight;
		uint8_t gotElement = _gotElement;
		
		// get the file path and size from telegram persistent id
		char *filePath;
		uint32_t fileSize;
		if(!getFilePath(fileId, filePath, fileSize))
		{
//			DEBUG_ERROR("Error getting file path\n");
		}
		else
		{
			// create the full path
			// https://api.telegram.org/file/bot<TOKEN>/photos/file_42.jpg
			size_t pathLen = strlen(__filePathStart);
			if(_flashToken)
				pathLen += strlen((const __FlashStringHelper *)_token);
			else
				pathLen += strlen(_token);
			pathLen++; // '/' after token!
			pathLen += strlen(filePath);
			char *fullPath = (char *)malloc(pathLen + 1);
			if(fullPath)
			{
				strcpy(fullPath, __filePathStart);
				if(_flashToken)
					strcat(fullPath, (const __FlashStringHelper *)_token);
				else
					strcat(fullPath, _token);
				strcat(fullPath, "/");
				strcat(fullPath, filePath);
				
				// process picture message
				if(_pictureEvent && (gotElement & FishGramPhoto))
				{
					res |= _pictureEvent(senderId, senderFirstName, senderLastName, fullPath, imgWidth, imgHeight, message);
				}
				// process audio message
				else if(_audioEvent && (gotElement & FishGramAudio))
				{
					res |= _audioEvent(senderId, senderFirstName, senderLastName, fullPath);
				}

				free(fullPath);
			}
			else
			{
				DEBUG_ERROR("Out of memory\n");
			}
		}
		
		// free temporaries
		if(senderFirstName)
			free(senderFirstName);
		if(senderLastName)
			free(senderLastName);
		if(message)
			free(message);
		if(fileId)
			free(fileId);
		if(filePath)
			free(filePath);
	}
	
	// reset elements
	clear();

	return res;
}

// constructor
FishGramClass::FishGramClass()
{
	// not started
	setState(FishGramOff);
	setSubState(FishGramOut);

	// clear the token
	_flashToken = false;
	_token = NULL;

	// delay interval between queries
	// 2 seconds default
	_queryTime = 5000;
	_queryTimer = millis();

	// read timeout interval and timer
	_readTimeoutTime = 5000;
	_readTimeoutTimer = 0;

	// last update id
	_lastUpdateId = (uint32_t)-1;

	// last message id got from Telegram
	_lastMessageId = 0;

	// last sender id
	_lastSenderId = 0;

	// last message got from Telegram
	_lastMessage = NULL;

	// required picture size
	// defaults to 2.4' TFT display resolution
	_reqImgWidth = 240;
	_reqImgHeight = 320;
	
	// previous picture data (needed to choose the best one
	_prevFile = NULL;
	_prevImgWidth = 0;
	_prevImgHeight = 0;
	
	// last picture data
	_lastFile = NULL;
	_lastImgWidth = 0;
	_lastImgHeight = 0;

	_lastSenderFirstName = NULL;
	_lastSenderLastName = NULL;
	
	_filePath = NULL;
	_fileSize = 0;

	// number of old messages to retrieve
	// the ones sent with app off
	// -1 means ALL of them, 0 none
	_recoveredOldMessages = 0;

	// restrict access from allowed user ids
	_restrictUsers = false;

	// for restricted access, number of allowed users
	// and their ids
	_numAllowedUsers = 0;
	_allowedUsers = NULL;

	// for restricted access, number of blocked users
	// and their ids
	_numBlockedUsers = 0;
	_blockedUsers = NULL;

	// functions for event processing
	_messageEvent = NULL;
	_pictureEvent = NULL;
	_audioEvent = NULL;

	// not sending
	_sending = false;
	
	// not getting file
	_gettingFile = false;

	// got element in previous message
	_gotElement = 0;
		
	// setup the parser
	_parser.reset();
	_parser.setCallback(__JSONCallback, this);
}

// destructor
FishGramClass::~FishGramClass()
{
	end();

	if (_allowedUsers)
		free(_allowedUsers);
	if (_blockedUsers)
		free(_blockedUsers);
}

// true begin function (after begin() has set token...)
bool FishGramClass::begin0(void)
{
	// must be called in off state
	if (_state != FishGramOff)
		return false;

	// setup number of old messages to retrieve
	if (_recoveredOldMessages == -1)
		_lastUpdateId = 0;
	else if (_recoveredOldMessages == 0)
		_lastUpdateId = -1;
	else
		_lastUpdateId = -_recoveredOldMessages;

	// if we don't want ANY old message, we shall set _lastUpdateId to -1
	// AND drop the first message got (there's no way to drop it automatically)
	if (_recoveredOldMessages == 0)
	{
		runQuery(-1);
		uint32_t tim = millis() + 2000;
		while (millis() < tim)
			if (_client.available())
				break;
		while (_client.available())
			_client.read();
		_client.stop();
		_lastUpdateId = 0;
	}

	// set state to idle
	setState(FishGramIdle);

	return true;
}

// begin
// use bot's access token
bool FishGramClass::begin(const __FlashStringHelper *token)
{
	_flashToken = true;
	_token = (const char *)token;
	return begin0();
}

bool FishGramClass::begin(const char *token)
{
	_flashToken = false;
	_token = token;
	return begin0();
}

// end
// terminate fishgram processing
bool FishGramClass::end(void)
{
	clear();
	setState(FishGramOff);

	return true;
}

// clear local data
FishGramClass &FishGramClass::clear(void)
{
	_parser.reset();
	
	_gotElement = 0;
	setSubState(FishGramOut);

	if (_lastMessage)
		free(_lastMessage);
	_lastMessage = NULL;
	if (_lastSenderFirstName)
		free(_lastSenderFirstName);
	_lastSenderFirstName = NULL;
	if (_lastSenderLastName)
		free(_lastSenderLastName);
	_lastSenderLastName = NULL;

	// previous picture data (needed to choose the best one
	if(_prevFile)
		free(_prevFile);
	_prevFile = NULL;

	_prevImgWidth = 0;
	_prevImgHeight = 0;
	
	// last picture data
	if(_lastFile)
		free(_lastFile);
	_lastFile = NULL;
	
	_lastImgWidth = 0;
	_lastImgHeight = 0;

	_lastMessageId = 0;
	_lastSenderId = 0;
	
	if(_filePath)
		free(_filePath);
	_filePath = NULL;
	_fileSize = 0;

	return *this;
}

// enable/disable allowed users list
FishGramClass &FishGramClass::restrict(bool b)
{
	_restrictUsers = b;

	return *this;
}

// add an user to allowed users list
FishGramClass &FishGramClass::allow(uint32_t id)
{
	if (!_numAllowedUsers)
	{
		_allowedUsers = (uint32_t *)malloc(sizeof(uint32_t));
		_allowedUsers[0] = id;
		_numAllowedUsers++;

	}
	else
	{
		_allowedUsers = (uint32_t *)realloc(_allowedUsers, (_numAllowedUsers + 1) * sizeof(uint32_t));
		_allowedUsers[_numAllowedUsers] = id;
		_numAllowedUsers++;
	}
	return *this;
}

// add an user to blocked users list
FishGramClass &FishGramClass::block(uint32_t id)
{
	if (!_numBlockedUsers)
	{
		_blockedUsers = (uint32_t *)malloc(sizeof(uint32_t));
		_blockedUsers[0] = id;
		_numBlockedUsers++;
	}
	else
	{
		_blockedUsers = (uint32_t *)realloc(_blockedUsers, (_numBlockedUsers + 1) * sizeof(uint32_t));
		_blockedUsers[_numBlockedUsers] = id;
		_numBlockedUsers++;
	}
	return *this;
}

// set number of old messages to recover
// -1 for ALL of them, 0 for none
// must be called BEFORE begin()
FishGramClass &FishGramClass::recoverOldMessages(uint32_t n)
{
	_recoveredOldMessages = n;

	return *this;
}

// add an event function that will be called on
// each received text message
FishGramClass &FishGramClass::messageEvent(FishGramMessageEvent e)
{
	_messageEvent = e;

	return *this;
}

// add an event function that will be called on
// each received picture
FishGramClass &FishGramClass::pictureEvent(FishGramPictureEvent e)
{
	_pictureEvent = e;
	_reqImgWidth = 0;
	_reqImgHeight = 0;

	return *this;
}

FishGramClass &FishGramClass::pictureEvent(FishGramPictureEvent e, uint16_t requestedWidth, uint16_t requestedHeight)
{
	_pictureEvent = e;
	_reqImgWidth = requestedWidth;
	_reqImgHeight = requestedHeight;

	return *this;
}

// add an event function that will be called on
// each received audio message
FishGramClass &FishGramClass::audioEvent(FishGramAudioEvent e)
{
	_audioEvent = e;

	return *this;
}

// send a message to a given ID
bool FishGramClass::sendMessage(uint32_t const &id, const char *msg)
{
	// start the message
	if (!startMessage(id, strlen(msg)))
		return false;

	// send message body
	if (!contMessage(msg))
		return false;

	// end and send the message
	return endMessage();
}

bool FishGramClass::sendMessage(uint32_t const &id, const __FlashStringHelper *msg)
{
	// start the message
	if (!startMessage(id, strlen_P((const char *)msg)))
		return false;

	// send message body
	if (!contMessage(msg))
		return false;

	// end and send the message
	return endMessage();
}

// the same, but in 3 steps; allows to send big messages
// without using much RAM. The only caveat is that you
// must know message length in advance
// to use, just call 'startMessage' with id and message len,
// 'contMessage' as many times as you need with message parts
// and 'endMessage' to terminate and send it
bool FishGramClass::startMessage(uint32_t const &id, uint16_t len)
{
	// if processing a query, wait till finished
	// (state must be idle)
	while (_state != FishGramIdle)
		loop();

	// connect to host
	if (!connect())
		return false;

	// signal we're sending
	_sending = true;
	_ok = false;

	// build first part of json message string
	String json = String(F("{\"chat_id\":\"")) + String(id) + String(F("\",\"text\":\""));

	// adjust given length to count json extra data
	// (last 2 bytes are message closing quotes and json } char)
	len += json.length() + 2;

	// send header and first part of json data
	_client << F("POST /bot");
	if (_flashToken)
		_client << (const __FlashStringHelper *)_token;
	else
		_client << _token;
	_client
	<< F("/sendMessage HTTP/1.1\r\n")
	<< F("Host: api.telegram.org\r\n")
	<< F("Content-Type: application/json\r\n")
	<< F("Content-Length: ") << len  << "\r\n\r\n"
	<< json
	;
	return true;
}

bool FishGramClass::contMessage(const char *msg)
{
	// send current message part
	_client << msg;
	return true;
}

bool FishGramClass::contMessage(const __FlashStringHelper *msg)
{
	// send current message part
	_client << msg;
	return true;
}

bool FishGramClass::contMessage(char c)
{
	// send current message part
	_client << c;
	return true;
}

bool FishGramClass::endMessage(void)
{
	// terminate the json and switch to listen state
	_client << "\"}\r\n";

	// switch to waiting state
	_state = FishGramListen;

	// wait for completion
	while (_state != FishGramIdle)
		loop();

	// reset _sending flag
	_sending = false;

	return _ok;
}

// loop
// to be called from inside main loop() function
// ask telegram for incoming messages and process them
bool FishGramClass::loop(void)
{
	switch (_state)
	{
			// do nothing if not started
		case FishGramOff:
			return false;

			// on error state or end of stream simply close the connection,
			// reset some stuffs and switch back to idle
		case FishGramRestart:
			{
				// end connection
				_client.stop();

				// reset query timer
				_queryTimer = millis() + _queryTime;

				// clear local data
				clear();

				// switch back to idle state
				setState(FishGramIdle);
				return true;
			}

			// if idle, check if it's time to start a new query
		case FishGramIdle:
			{
				// if sending, keep in idle state
				// (send routines will change it)
				if (_sending)
					return true;

				// if not expired, just return
				if (millis() < _queryTimer)
					return true;

				// reset query timer
				_queryTimer = millis() + _queryTime;

				// start a new query
				if (!runQuery(_lastUpdateId))
				{
					setState(FishGramRestart);
					return false;
				}

				// switch to listen state
				_readTimeoutTimer = millis() + _readTimeoutTime;
				setState(FishGramListen);
				return true;
			}

			// if listening for an answer, check for client data
			// or timeout, which comes first
		case FishGramListen:
			{
				// if data available, switch to header state
				if (_client.available())
				{
					setState(FishGramHeadersStartLine);
					return true;
				}

				// if timeout timer expired, close connection and switch back to idle
				else
					if (millis() > _readTimeoutTimer)
					{
						DEBUG_WARNING("Didn't get data from server\n");
						setState(FishGramRestart);
						return false;
					}

				// if none of the above, keep listening
					else
						return true;
			}

			// if on headers start of line, check if there's a newline character
			// which means end of headers
		case FishGramHeadersStartLine:
			{
				// read next character
				int c = readChar();

				// check for read errors
				if (c == -1)
				{
					// read error... stop
					setState(FishGramRestart);
					return false;
				}
				if (c == '\r')
				{
					// end of headers detected
					// skip '\n' character and switch to FishGramJSon state
					readChar();
					setState(FishGramJSon);

					return true;
				}
				// not at start line, go to headers body state
				setState(FishGramHeaders);
				return true;
			}

			// if we're inside headers, keep reading up to end of line and skip it
		case FishGramHeaders:
			{
				// read next character
				int c = readChar();

				// check for read errors
				if (c == -1)
				{
					// read error... stop
					setState(FishGramRestart);
					return false;
				}
				if (c == '\r')
				{
					// end of line detected, skip it and switch to FishGramHeadersStartLine
					// state back
					readChar();
					setState(FishGramHeadersStartLine);
					return true;
				}

				// if none of the above, just eat header's char
				return true;
			}

			// and last, if we're on JSOn state, we feed the beast up to end of characters
		case FishGramJSon:
			{
				// read next character
				int c = readChar();

				// check for read errors
				if (c == -1)
				{
					// read error... stop
					setState(FishGramRestart);
					return false;
				}

				// feed the beast
				_parser.feed((char)c);

				// if parser has finished, close the connection
				// and switch back to idle state
				if (_parser.isFinished() || _parser.isError())
				{
					// close the client -- remember... just ONE https at a time
					// processMessage may need to open one!
					_client.stop();
					
					if (!_parser.isError() && !_sending && !_gettingFile)
						processMessage();
					setState(FishGramRestart);
					return !_parser.isError();
				}

				// keep processing chars
				return true;
			}

			// should not happen, but...
		default:
			DEBUG_ERROR_FUNCTION("Unknown state %u\n", _state);
			setState(FishGramRestart);
			return false;
	}
}

FishGramClass &__fishGram(void)
{
	static FishGramClass fish;

	return fish;
}
