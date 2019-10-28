//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishGram.h											//
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

/*
  FishGram.h - Library to handle Telegram messaging with Fishino
  Copyright (c) 2017 Massimo Del Fedele and Andrea S. Costa. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  VERSION 1.0.0 - INITIAL VERSION
*/
#ifndef Fishgram_h
#define Fishgram_h

#include <Arduino.h>
#include <Fishino.h>
#include <JSONStreamingParser.h>

#define FishGram __fishGram()

///////////////////////////////////////////////////////////////////////////////////////////////////
// main FishGram class

// event function for FishGram's clients
typedef bool (*FishGramMessageEvent)(uint32_t id, const char *firstName, const char *lastName, const char *message);
typedef bool (*FishGramPictureEvent)(uint32_t id, const char *firstName, const char *lastName, const char *remotePath, uint16_t w, uint16_t h, const char *caption);
typedef bool (*FishGramAudioEvent)(uint32_t id, const char *firstName, const char *lastName, const char *remotePath);

class FishGramClass
{
	friend void __JSONCallback(uint8_t filter, uint8_t level, const char *name, const char *value, void *cbObj);

	public:
	
		enum States
		{
			FishGramOff					= 0,
			FishGramRestart				= 1,
			FishGramIdle				= 2,
			FishGramListen				= 3,
			FishGramHeadersStartLine	= 4,
			FishGramHeaders				= 5,
			FishGramJSon				= 6
		};
		
		enum SubStates
		{
			FishGramOut					= 0,
			FishGramInMessage			= 1,
			FishGramInFrom				= 2,
			FishGramInPhoto				= 3,
			FishGramInPhotoRes			= 4,
			FishGramInVoice				= 5,
		};
		
		enum Elements
		{
			FishGramNone				= 0,
			FishGramText				= 1,
			FishGramPhoto				= 2,
			FishGramAudio				= 4,
			FishGramFile				= 8
		};
		
	private:
	
		// state
		States _state;
		SubStates _subState;
		
		// sending flag
		bool _sending;
		
		// getting file flag
		bool _gettingFile;
		
		// ok flag, got with json responses
		bool _ok;
		
		// got element in previous message
		uint8_t _gotElement;
		
		// state setter -- for debug purposes
		void setState(States s);
		void setSubState(SubStates s);
	
		// the client
		FishinoSecureClient _client;
		
		// read char hook -- for debugging purposes
		int readChar(void);
		
		// the parser
		JSONStreamingParser _parser;
		
		// choose picture resolution -- called when receiving new picture format
		void setLastImage(void);
		void chooseResolution(void);
		
		// the internal callback
		void JSONCallback(uint8_t filter, uint8_t level, const char *name, const char *value);
		
		// the token
		bool _flashToken;
		const char *_token;
		
		// delay interval between queries
		// and query timer
		uint32_t _queryTime;
		uint32_t _queryTimer;
		
		// read timeout interval and timer
		uint32_t _readTimeoutTime;
		uint32_t _readTimeoutTimer;
		
		// last update id
		int32_t _lastUpdateId;
		
		// last message id got from Telegram
		uint32_t _lastMessageId;
		
		// last sender id got from telegram
		uint32_t _lastSenderId;
		
		// last message got from Telegram
		char *_lastMessage;
		
		// required picture size
		uint16_t _reqImgWidth, _reqImgHeight;
		
		// previous picture data (needed to choose the best one
		char *_prevFile;
		uint16_t _prevImgWidth, _prevImgHeight;
		
		// last picture data
		char *_lastFile;
		uint16_t _lastImgWidth, _lastImgHeight;
		
		// last message sender first and last names
		char *_lastSenderFirstName;
		char *_lastSenderLastName;
		
		// variables used on 'getFile' queries
		uint32_t _fileSize;
		char *_filePath;
		
		// number of old messages to retrieve
		// the ones sent with app off
		// -1 means ALL of them, 0 none
		int32_t _recoveredOldMessages;
		
		// restrict access from allowed user ids
		bool _restrictUsers;
		
		// for restricted access, number of allowed users
		// and their ids
		uint16_t _numAllowedUsers;
		uint32_t *_allowedUsers;
		
		// for restricted access, number of blocked users
		// and their ids
		uint16_t _numBlockedUsers;
		uint32_t *_blockedUsers;
		
		// function for message event processing
		FishGramMessageEvent _messageEvent;
		
		// function for picture event processing
		FishGramPictureEvent _pictureEvent;

		// function for audio event processing
		FishGramAudioEvent _audioEvent;
		
		// true begin function (after begin() has set token...)
		bool begin0(void);
		
		// connect to telegram server, with some retry on failuse
		bool connect(void);
		
		// run the query
		bool runQuery(int32_t id = -1);
		
		// get file path from persistent ID
		// return a dynamically allocated variable
		// that must be freed by caller
		bool getFilePath(const char *id, char *&path, uint32_t &size);
		
		// process received message
		bool processMessage(void);
		
	protected:
	
	public:
	
		// constructor
		FishGramClass();
		
		// destructor
		~FishGramClass();
		
		// begin
		// use bot's access token
		bool begin(const __FlashStringHelper *token);
		bool begin(const char *token);
		
		// end
		// terminate fishgram processing
		bool end(void);
		
		// clear local data
		FishGramClass &clear(void);
		
		// enable/disable allowed users list
		FishGramClass &restrict(bool b = true);
		FishGramClass &noRestrict(void) { return restrict(false); }
		
		// add an user to allowed users list
		FishGramClass &allow(uint32_t id);
		
		// add an user to blocked users list
		FishGramClass &block(uint32_t id);
		
		// set number of old messages to recover
		// -1 for ALL of them, 0 for none
		// must be called BEFORE begin()
		FishGramClass &recoverOldMessages(uint32_t n = (uint32_t)-1);
		
		// add an event function that will be called on
		// each received message
		FishGramClass &messageEvent(FishGramMessageEvent e);
		
		// add an event function that will be called on
		// each received picture
		FishGramClass &pictureEvent(FishGramPictureEvent e);
		FishGramClass &pictureEvent(FishGramPictureEvent e, uint16_t requestedWidth, uint16_t requestedHeight);
		
		// add an event function that will be called on
		// each received audio message
		FishGramClass &audioEvent(FishGramAudioEvent e);
		
		// send a message to a given ID
		bool sendMessage(uint32_t const &id, const char *msg);
		bool sendMessage(uint32_t const &id, const __FlashStringHelper *msg);
		
		// the same, but in 3 steps; allows to send big messages
		// without using much RAM. The only caveat is that you
		// must know message length in advance
		// to use, just call 'startMessage' with id and message len,
		// 'contMessage' as many times as you need with message parts
		// and 'endMessage' to terminate and send it
		bool startMessage(uint32_t const &id, uint16_t len);
		bool contMessage(const char *msg);
		bool contMessage(const __FlashStringHelper *msg);
		bool contMessage(char c);
		bool endMessage(void);
		
		// loop
		// to be called from inside main loop() function
		// ask telegram for incoming messages and process them
		bool loop(void);
};


extern FishGramClass &__fishGram(void);

#endif
