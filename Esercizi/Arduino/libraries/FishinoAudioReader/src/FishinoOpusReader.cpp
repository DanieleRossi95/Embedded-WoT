//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoOpusReader.cpp									//
//						Ogg Opus Audio Reader Interface								//
//					Copyright (c) 2017 Massimo Del Fedele							//
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
//	VERSION 1.0.0	2017		INITIAL VERSION										//
//  Version 6.0.2	June 2017	Some small fixes									//
//  Version 6.0.3	June 2017	Fix a crash when trying to read on a closed stream	//
//  Version 6.0.4	June 2017	Some small fixes									//
//  Version 6.0.5	June 2017	Fixed MP3 for weird files							//
//  Version 7.3.1	24/12/2017	Fixed mips16 compilation issues						//
//																					//
//////////////////////////////////////////////////////////////////////////////////////

//#define DEBUG_LEVEL_ERROR
#include <FishinoDebug.h>

#include "FishinoAudioReader.h"

#ifndef ARDUINO_STREAMING
#define ARDUINO_STREAMING
template<class T> inline Print &operator <<(Print &stream, T arg)
{
	stream.print(arg);
	return stream;
}
#endif

/*****************************MACROS*******************************************/
#define OPUS_INPUT_BUFFER_SIZE (1024*2)
#define OPUS_OUTPUT_BUFFER_SIZE (1024*7)
#define OPUS_MAX_FRAME_SIZE (960*6) // 120ms @ 48Khz

const char *OPUS_ERROR_MESSAGES[] =
{
	"",
	"OPUS_SUCCESS",
	"OPUS_READ_ERROR",
	"OPUS_STREAM_ERROR",
	"OPUS_BUFF_ERROR",
	"OPUS_STREAM_END",
	"OPUS_PLAYBACK_ERROR",
	"OPUS_OUT_OF_MEM_ERROR",
	"OPUS_DISK_ERROR",
	"OPUS_GENERAL_ERROR",
	"OPUS_UNKNOWN_ERROR"
};

static const char *opusErrorMsg(int err) __attribute__((used));
static const char *opusErrorMsg(int err)
{
	if(err < 1 || err >= 10)
		return OPUS_ERROR_MESSAGES[10];
	else
		return OPUS_ERROR_MESSAGES[err];
}

typedef enum
{
	OGG_ID_MAGIC	=  0x5367674F   //'SggO', OggS, the magic page capture

} eOggId;

typedef enum
{
	OGG_FLAG_CONTINUATION	 = 0x1,	// continuation packet
	OGG_FLAG_BOS		     = 0x2,	// first page of the logical stream
	OGG_FLAG_EOS		     = 0x4,	// last page of the logical stream

} eOggHeaderFlags;	// Flags in the Ogg header


/////////////////////////////////////////////////////////////////////////////////////////////////

OPUS_ERROR_MSG OpusReader::OggReadPageHdr(sOggPageHdr *pOggHdr, sOpusStreamDcpt *pOpusDcpt)
{
	uint32_t pageBytes = 0;
	bool useStreamId;
	int streamId = 0;
	OPUS_ERROR_MSG res = OPUS_SUCCESS;
	uint32_t readRet;

	if ((useStreamId = (pOpusDcpt != 0)))
	{
		streamId = pOpusDcpt->pageHdr.streamNo;
		if ((pOpusDcpt->pageHdr.headerFlags&OGG_FLAG_EOS) != 0)
		{
			DEBUG_INFO_FUNCTION("OPUS STREAM END\n");
			// end of stream
			res = OPUS_STREAM_END;
			
		}
	}

	while (res == OPUS_SUCCESS)
	{
		pageBytes = sizeof(*pOggHdr) - sizeof(pOggHdr->segmentTbl);	// page with no segment table
		readRet = inStreamRead((uint8_t*)pOggHdr, pageBytes);
		if (readRet != pageBytes)
		{
			DEBUG_ERROR_FUNCTION("Read error 1, expecting %d bytes, got %d\n", (int)pageBytes, (int)readRet);
			res = OPUS_READ_ERROR;
			break;
		}
		else
			if (pOggHdr->pageCapture != OGG_ID_MAGIC)
			{
				// not an Ogg Stream
				DEBUG_ERROR_FUNCTION("NOT AN OPUS STREAM\n");
				res = OPUS_STREAM_ERROR;
				break;
			}
			else
				if ((pageBytes = pOggHdr->pageSegments))
				{

					readRet = inStreamRead((uint8_t*)pOggHdr->segmentTbl, pageBytes);
					if (readRet == pageBytes)
					{
						if (!useStreamId || pOggHdr->streamNo == streamId)
						{
							// found it
							break;
						}
						else
							if (!(pOggHdr->headerFlags&OGG_FLAG_EOS))
							{
								res = OggSkipPage(pOggHdr);
								continue;
							}
							else
							{
								// wrong stream, end of stream
								res = OPUS_STREAM_END;
								break;
							}
					}
					else
					{
						DEBUG_ERROR_FUNCTION("Read error 2, expecting %d bytes, got %d\n", (int)pageBytes, (int)readRet);
						res = OPUS_READ_ERROR;
						break;
					}
				}
				else
					if (pOggHdr->headerFlags&OGG_FLAG_EOS)
					{
						res = OPUS_STREAM_END;
					}
					else
					{
						// empty page ?
						res = OPUS_STREAM_ERROR;
						DEBUG_INFO_FUNCTION("EMPY PAGE?\n");
						break;
					}
	}

	return res;
}



int32_t OpusReader::OggPageDataSize(sOggPageHdr* pOggPage)
{
	int32_t	ix;
	int32_t	dataBytes;

	for (ix = 0, dataBytes = 0; ix < pOggPage->pageSegments; ix++)
	{
		dataBytes += pOggPage->segmentTbl[ix];
	}
	return dataBytes;
}

OPUS_ERROR_MSG OpusReader::OggSkipPage(sOggPageHdr* pOggPage)
{
	OPUS_ERROR_MSG ret;

	int32_t	dataBytes = OggPageDataSize(pOggPage);

	if(inStreamSkip(dataBytes))
		ret = OPUS_SUCCESS;
	else
	{
		DEBUG_ERROR_FUNCTION("Skip error\n");
		ret = OPUS_READ_ERROR;
	}
	return ret;
}

// returns the number of bytes in the current packet, completed or not
// also returns true if the packet pointed by the current segIx is completed on this page
// updates the current segIx
int OpusReader::OggPageGetPktSize(sOpusStreamDcpt* pOpusDcpt, int* pPktComplete)
{
	int segIx;
	int nBytes;
	int totBytes = 0;

	*pPktComplete = 0;
	for (segIx = pOpusDcpt->segIx; segIx < pOpusDcpt->pageHdr.pageSegments; segIx++)
	{
		totBytes += (nBytes = pOpusDcpt->pageHdr.segmentTbl[segIx]);
		if (nBytes < 255)
		{
			*pPktComplete = 1;
			segIx++;
			break;
		}
	}

	pOpusDcpt->segIx = segIx;
	return totBytes;
}

// This function read packet by packet

// read next data packet
// the read will extend across page boundary if the packet doesn't end on the current page
// in case of page lost, the correct packet number should be returned

// param: pBuff: input buffer pointer
OPUS_ERROR_MSG OpusReader::OggGetPacketData(sOpusStreamDcpt* pOpusDcpt, void* pBuff, int nBytes, void* pResult)
{
	int		rdBytes;
	int		pktBytes;
	int		pktComplete;
	int		prevPgNo;
	int		loopCount;

	OPUS_ERROR_MSG	res = OPUS_SUCCESS;
	sOpusPktDcpt*	pPktDcpt = (sOpusPktDcpt*)pResult;


	pPktDcpt->pktBytes = 0;
	loopCount = 0;

	while (1)
	{
		pktBytes = OggPageGetPktSize(pOpusDcpt, &pktComplete);
		if (pktBytes == 0 && pktComplete)
		{
			// if loopCount==0, we have a 0 len pkt mid page
			res = loopCount ? OPUS_SUCCESS : OPUS_STREAM_ERROR;
			break;
		}

		// either pktBytes or pktComplete==0
		if (pktBytes)
		{
			if (nBytes < pktBytes)
			{
				res = OPUS_BUFF_ERROR;	// should accommodate at least a packet
				break;
			}
			rdBytes = inStreamRead((uint8_t*)pBuff, pktBytes);

			if (rdBytes != pktBytes)
			{
				DEBUG_ERROR_FUNCTION("Read error, expecting %d bytes, got %d\n", pktBytes, rdBytes);
				res = OPUS_READ_ERROR;
				break;
			}
		}

		if (pktComplete)
		{
			pPktDcpt->pktBytes += pktBytes;
			pPktDcpt->pktSeqNo = ++pOpusDcpt->pktIx;	// was in sync

			// Found all data in this packet,
			// break this loop
			break;
		}


		// read in a new page
		pBuff = (char*)pBuff + pktBytes;
		nBytes -= pktBytes;

		prevPgNo = pOpusDcpt->pageHdr.pageNo;
		res = OggReadPageHdr(&pOpusDcpt->pageHdr, pOpusDcpt);
		if (res != OPUS_SUCCESS)
		{
			res = OPUS_STREAM_END;	// eos
			break;		// could not end in the middle of the packet!
		}
		pOpusDcpt->segIx = 0;	// brand new page

		if (pOpusDcpt->pageHdr.pageNo != prevPgNo + 1)
		{
			// we're out of sync
			// TODO: update pSpxDcpt->pktIx with lost packets: based on frameSamples, framesPerPkt and granulePos
			// for now, no op. We maintain sequential packet numbers!
		}
		loopCount++;
	}

	if(res != OPUS_SUCCESS)
	{
		DEBUG_ERROR_FUNCTION("OPUS ERROR : %s\n", opusErrorMsg(res));
		setEof();
	}
	return res;
}
/////////////////////////////////////////////////////////////////////////////////////////////////

int32_t OpusReader::readPacket(uint8_t *inBuff)
{
	OPUS_ERROR_MSG res;
	sOpusPktDcpt pktDcpt;
	int32_t readBytes = 0;

//    pDDcpt->cdc.nOutBytes=0;
	while (pDDcpt->processedPktNo == pDDcpt->currPktNo)
	{
		readBytes = inStreamTell();
		pktDcpt.pktSeqNo = pDDcpt->currPktNo;
		res = OggGetPacketData(pOpusDcpt, inBuff, OPUS_INPUT_BUFFER_SIZE, &pktDcpt);
		readBytes = inStreamTell() - readBytes;
		// zero byte packet, read next packet
		if (readBytes != 0)
		{

			pDDcpt->currPktNo = pktDcpt.pktSeqNo;
			pDDcpt->nInBytes = pktDcpt.pktBytes;
			return readBytes;
		}
		if (res != OPUS_SUCCESS)
		{
			setEof();
			
			// stream end is returned as an error
			if(res == OPUS_STREAM_END)
				return 0;
			DEBUG_ERROR_FUNCTION("Opus error code : %s\n", opusErrorMsg(res));
			return -1;
		}

		pDDcpt->processedPktNo = pDDcpt->currPktNo;
//        return readBytes;
	}
	return -1;
}

OPUS_ERROR_MSG OpusReader::decode(const uint8_t *input, uint16_t inSize, uint16_t *read, int16_t *output, uint16_t *written, uint16_t outSize)
{
	OPUS_ERROR_MSG res = OPUS_SUCCESS;

	// opus_decode return value,
	// it is number of decoded pcm samples per channel,
	// thus, if this opus stream is stereo, the size of
	// returned pcm value is out_samples*4. (out_samples*bitdepth*channel)
	int output_samples = 0;

	if (pDDcpt->currPktNo == pDDcpt->processedPktNo + 1)
	{
		// in sync
		// regular decode
		// CAUTION: this decode function may return decoded data larger than output buffer size,
		// you'd better:
		//          1. has a output buffer size larger than or equal to OPUS_MAX_FRAME_SIZE*4,
		//             if the channel number is 2, or OPUS_MAX_FRAME_SIZE*2 if the channel number is 1.
		//          2. make sure each packet in opus file does not contain more than OPUS_MAX_FRAME_SIZE samples

		output_samples = opus_decode(pOpusDecoder, input, pDDcpt->nInBytes, output, OPUS_MAX_FRAME_SIZE, 0);

	}
	else
	{
		// lost frames, let decoder guess
		if (pOpusHdr->channels == 1)
			output_samples = opus_decode(pOpusDecoder, NULL, 0, output, outSize / 2, 0);
		else
			if (pOpusHdr->channels == 2)
			{
				output_samples = opus_decode(pOpusDecoder, NULL, 0, output, outSize / 4, 0);
			}
	}

	if (output_samples <= 0)
	{
		setEof();
		res = OPUS_STREAM_ERROR;
		*written = 0;
		return res;
	}

	// Convert to stereo
	if (pOpusHdr->channels == 1)
	{
		// preskip
		if (pDDcpt->currPktNo == 1 && pOpusHdr->preskip != 0)
		{

			output_samples = (pOpusHdr->preskip > output_samples) ? output_samples : output_samples - pOpusHdr->preskip;
			memcpy(output, output + pOpusHdr->preskip, 2*output_samples);
		}
		*written += 2 * output_samples; // multiply by 2, mono mode

	}
	else
		if (pOpusHdr->channels == 2)
		{
			if (pDDcpt->currPktNo == 1 && pOpusHdr->preskip != 0)
			{
				output_samples = (pOpusHdr->preskip > output_samples) ? output_samples : output_samples - pOpusHdr->preskip;
				memcpy(output, output + 2*pOpusHdr->preskip, 4*output_samples);
			}
			*written += 4 * output_samples; // multiply by 4, stereo mode
		}
	*read = inSize;

	if (res == OPUS_SUCCESS)
	{
		pDDcpt->processedPktNo++;	// packet processed
		return res;
	}
	setEof();
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

// fill buffer -- called by timer interrupt handler
uint32_t OpusReader::getSamples(uint32_t *buf, uint32_t len)
{
	//	DEBUG_INFO("\n\nFILL BUFFER %d\n", bufToFill);

	int32_t bytesRead = readPacket(_oggBuf);
	if(bytesRead < 0)
	{
		// should not happen
		DEBUG_ERROR_FUNCTION("Stream error\n");
		setEof();
		return 0;
	}
	else if(bytesRead == 0)
	{
		// end of file
		DEBUG_INFO_FUNCTION("EOF\n");
		setEof();
		return 0;
	}
	uint16_t decoded = 0;
	uint16_t written = 0;
	OPUS_ERROR_MSG res = decode(_oggBuf, bytesRead, &decoded, (int16_t *)buf, &written, len);
	
	if(res != OPUS_SUCCESS)
	{
		// should not happen
		DEBUG_ERROR_FUNCTION("Decode error\n");
		setEof();
		return 0;
	}
	
	// return current buffer size
	return written;
}

// constructor -- takes a stream
OpusReader::OpusReader(FishinoStream &s) : AudioReader(s)
{
	_opusOpened = false;
	pOpusDecoder = NULL;

	_oggBuf = NULL;

}

// destructor
OpusReader::~OpusReader()
{
	end();
	DEBUG_INFO_FUNCTION("DESTROYING READER\n");
}

// opens a file
bool OpusReader::initialize(void)
{
	// create ogg buffer
	_oggBuf = (uint8_t *)malloc(OPUS_INPUT_BUFFER_SIZE);
	if(!_oggBuf)
		return false;

	OPUS_ERROR_MSG ret = OPUS_SUCCESS;
	uint32_t readRet;
	bool commentFound;
	sOggPageHdr *pOggPageHdr = &pOpusDcpt->pageHdr;
	int err;

	while (ret == OPUS_SUCCESS)
	{
		if ((ret = OggReadPageHdr(pOggPageHdr, 0)) != OPUS_SUCCESS)
		{
			break;
		}

		if (
			pOggPageHdr->headerFlags != OGG_FLAG_BOS ||
			pOggPageHdr->granulePos != 0 ||
			pOggPageHdr->pageNo != 0 ||
			pOggPageHdr->pageSegments != 1
		)
		{
			// looking into a wrong Ogg stream
			// try to skip this page in case we have a multiple Ogg container...
			ret = OggSkipPage(pOggPageHdr);
			continue;	// skipped current page...
		}

		// read OpusHead content
		readRet = inStreamRead((uint8_t*)pOpusHdr, pOggPageHdr->segmentTbl[0]);

		if (readRet != pOggPageHdr->segmentTbl[0])
		{
			DEBUG_ERROR_FUNCTION("Read error, expecting %d bytes, got %d\n", pOggPageHdr->segmentTbl[0], (int)readRet);
			ret = OPUS_READ_ERROR;	// could not read the Stream
			break;
		}

		// verify magic signature field
		if (strncmp(pOpusHdr[0].signature, "OpusHead", 8))
		{
			// skip current page in case we have a multiple Ogg container...
			continue;
		}

		// finally, we got a valid Opus header.
		// try to get to the data page
		// for now, we ignore the opus tags page

		commentFound = 0;
		while (ret == OPUS_SUCCESS)
		{
			if ((ret = OggReadPageHdr(pOggPageHdr, pOpusDcpt)) != OPUS_SUCCESS)
			{
				// some error
				break;
			}

			// valid new page from the same stream
			if (!commentFound)
			{
				// this should be the opus comment block
				// THIS DOES NOT handle the case that comment header spans two or more pages.
				if (pOggPageHdr->granulePos == 0 && pOggPageHdr->pageNo == 1 && pOggPageHdr->segmentTbl[pOggPageHdr->pageSegments-1] < 255)
				{
					// seems to be the valid comment

					// verify comment header magic signature
					// strcmp() "OpusTags", skip this for efficiency
					commentFound = 1;
				}

				ret = OggSkipPage(pOggPageHdr);
				continue;	// just skip it anyway
			}
			else
				if (pOggPageHdr->pageNo == 2 && pOggPageHdr->granulePos)
				{
					// ok, we got the 1st data page
					break;
				}

			ret = OggSkipPage(pOggPageHdr);	// try to find another start of data page?
			// will fail, most likely
		}

		if (ret != OPUS_SUCCESS)
		{
			break;	// some error occurred
		}


		// THIS ONLY HANDLES Channel mapping family 0
		if (pOpusHdr->channel_mapping == 0)
		{
			pOpusHdr->nb_streams = 1;
			pOpusHdr->nb_coupled = pOpusHdr->channels - 1;
			pOpusHdr->stream_map[0] = 0;
			pOpusHdr->stream_map[1] = 1;

			pDDcpt->currPktNo = 0;
			pDDcpt->processedPktNo = 0;

			pOpusDcpt->pktIx = 0;
			pOpusDcpt->prevBytes = 0;
			pOpusDcpt->segIx = 0;


			pOpusDecoder = opus_decoder_create(48000, pOpusHdr->channels, &err);

			if (err != OPUS_OK)
			{
				ret = OPUS_GENERAL_ERROR;
				break;
			}
			if (!pOpusDecoder)
			{
				ret = OPUS_GENERAL_ERROR;
				break;
			}
		}
		else
		{

			ret = OPUS_GENERAL_ERROR;
			break;
		}
		break;
	}

	if(ret == OPUS_SUCCESS)
	{
		_opusOpened = true;
	
		// data is always 16 bits signed on opus
		_bits = 16;
	
		// get and display encoding sampling rate
		_sampleRate = getSamplingRate();
		DEBUG_INFO_FUNCTION("Sampling rate : %d\n", (int)_sampleRate);
	
		// check if stereo mode
		_stereo = (getChannels() > 1);
		DEBUG_INFO_FUNCTION("%s mode\n", _stereo ? "Stereo" : "Mono");
	
		return true;
	}
	else
	{
		DEBUG_ERROR_FUNCTION("Error initializing Opus decoder : %s\n", opusErrorMsg(ret));
		setEof();
		return false;
	}
}

// terminate processing and free resources
// (should be called in final class destructor)
void OpusReader::finalize(void)
{
	DEBUG_INFO_FUNCTION("OpusReader::finalize()\n");
	if (pOpusDecoder != NULL)
		opus_decoder_destroy(pOpusDecoder);
	pOpusDecoder = NULL;
	
	if(_oggBuf)
		free(_oggBuf);
	_oggBuf = NULL;
	
	_opusOpened = false;
}
