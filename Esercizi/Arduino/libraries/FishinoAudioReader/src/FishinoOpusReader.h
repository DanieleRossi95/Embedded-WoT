//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoOpusReader.h										//
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
#ifndef __FISHINOOPUSREADER_H
#define __FISHINOOPUSREADER_H

#include "opus/include/opus.h"

typedef enum
{
	OPUS_SUCCESS = 1,
	OPUS_READ_ERROR,
	OPUS_STREAM_ERROR,
	OPUS_BUFF_ERROR,
	OPUS_STREAM_END,
	OPUS_PLAYBACK_ERROR,
	OPUS_OUT_OF_MEM_ERROR,
	OPUS_DISK_ERROR,
	OPUS_GENERAL_ERROR
	
} OPUS_ERROR_MSG;


typedef struct
{
	char signature[8]; // Magic signature: "OpusHead"
	uint8_t version;   // Version number: 0x01 for this spec
	uint8_t channels; // Number of channels: 1..255
	uint16_t preskip; // This is the number of samples (at 48kHz) to discard from the decoder output when starting playback
	uint32_t input_sample_rate; // Original input sample rate in Hz, this is not the sample rate to use for playback of the encoded data
	uint16_t gain; // output gain in dB
	uint8_t channel_mapping; // This byte indicates the order and semantic meaning of various channels encoded in each Opus packet
	/* The rest is only used if channel_mapping != 0 */
	int8_t nb_streams; // This field indicates the total number of streams so the decoder can correctly parse the packed Opus packets inside the Ogg packet
	int8_t nb_coupled; // Describes the number of streams whose decoders should be configured to produce two channels
	unsigned char stream_map[255];
	
} sOpusHeader;



typedef struct  __attribute__((packed))
{
	int32_t	pageCapture;		// should be OGG_ID_MAGIC
	int8_t	struct_ver;         // version of the Ogg file format. Should be 0 (RFC3533)
	int8_t	headerFlags;		// an eOggHeaderFlags value
	int64_t	granulePos;         // stream dependent position info
	int32_t	streamNo;           // logical bit stream identifier
	int32_t	pageNo;             // page sequence number
	int32_t	pageCrc;            // CRC32 checksum of the page
	uint8_t	pageSegments;		// number of page segments to follow
	uint8_t	segmentTbl[255];	// actually segmentTbl[pageSegments]; contains the lace
	// values for all segments in the page
	
} sOggPageHdr;	// header of an Ogg page, full segment info included

typedef struct
{
	int		pktBytes;		// how many bytes in this packet
	int		pktSeqNo;		// packet sequence number
	
} sOpusPktDcpt;	            // decoder data packet descriptor

typedef struct
{
	sOggPageHdr	pageHdr;        // current page header
	int		segIx;			    // current packet segment index in the current page
	int		pktIx;			    // current packet index, 0 -> ...
	int		prevBytes;		    // previous value of the bytes in the encoded output buffer
	
} sOpusStreamDcpt;		        // info needed by the stream at run-time

typedef struct
{
	int		processedPktNo;		// counter of processed packets
	int		currPktNo;		    // number of the currently received packet from the stream
	int		nInBytes;		    // bytes available in the input buffer
	
} opusDecDcpt;

class OpusReader : public AudioReader
{
	private:

		// OPUS file always play at 48Khz, see OPUS spec
		static const uint16_t	OPUS_FS = 48000;
		sOpusStreamDcpt			pOpusDcpt[1]; 
		sOpusHeader				pOpusHdr[1];
		OpusDecoder				*pOpusDecoder;  
		opusDecDcpt				pDDcpt[1];

		OPUS_ERROR_MSG			OggReadPageHdr(sOggPageHdr *pOggHdr, sOpusStreamDcpt *pOpusDcpt);
		int32_t					OggPageDataSize(sOggPageHdr* pOggPage);
		OPUS_ERROR_MSG			OggSkipPage(sOggPageHdr* pOggPage);
		int						OggPageGetPktSize(sOpusStreamDcpt* pOpusDcpt, int* pPktComplete);
		OPUS_ERROR_MSG			OggGetPacketData(sOpusStreamDcpt* pOpusDcpt, void* pBuff, int nBytes, void* pResult);

		uint8_t					getChannels(void) const { return pOpusHdr->channels; }
		uint32_t				getSamplingRate(void) const { return OPUS_FS; }

		int32_t					readPacket(uint8_t *inBuff);
		OPUS_ERROR_MSG			decode(const uint8_t *input, uint16_t inSize, uint16_t *read, int16_t *output, uint16_t *written, uint16_t outSize);

		// input buffer
		uint8_t *_oggBuf;

		bool _opusOpened;

	protected:

		// starts processing data and prefill buffers
		virtual bool initialize(void);

		// terminate processing and free resources
		// (should be called in final class destructor)
		virtual void finalize(void);

		// fill buffer -- called by timer interrupt handler
		virtual uint32_t getSamples(uint32_t *buf, uint32_t len);

	public:

		// constructor -- takes a stream
		OpusReader(FishinoStream &s);

		// destructor
		~OpusReader();

};

#endif
