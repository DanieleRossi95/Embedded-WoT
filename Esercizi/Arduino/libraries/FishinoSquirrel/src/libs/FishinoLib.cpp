#include "FishinoLib.h"
#include "StreamLib.h"

#include <Fishino.h>

using namespace sqfish;

//////////////////////////////////////////////////////////////////////////////////////////

struct SqIPAddress : public IPAddress
{
	SqIPAddress() : IPAddress() {}
	SqIPAddress(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) : IPAddress(b1, b2, b3, b4) {}
	SqIPAddress(uint32_t address) : IPAddress(address) {}

	std::string tostring(void);
	
	virtual ~SqIPAddress() {}
};

std::string SqIPAddress::tostring(void)
{
	char buf[16];
	snprintf(buf, 16, "%u:%u:%u:%u", operator[](0), operator[](1), operator[](2), operator[](3));
	buf[15] = 0;
	return buf;
}

//////////////////////////////////////////////////////////////////////////////////////////

struct SqMacAddress
{
	uint8_t _bytes[6];
	SqMacAddress();
	SqMacAddress(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6);
	std::string tostring(void); 
};

SqMacAddress::SqMacAddress()
{
	memset(_bytes, 0, 6);
}

SqMacAddress::SqMacAddress(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6)
{
	_bytes[0] = b1;
	_bytes[1] = b2;
	_bytes[2] = b3;
	_bytes[3] = b4;
	_bytes[4] = b5;
	_bytes[5] = b6;
}

std::string SqMacAddress::tostring(void)
{
	char buf[18];
	snprintf(buf, 18, "%02x:%02x:%02x:%02x:%02x:%02x:",
		_bytes[0], _bytes[1], _bytes[2], _bytes[3], _bytes[4], _bytes[5]
	);
	buf[17] = 0;
	return buf;
}

//////////////////////////////////////////////////////////////////////////////////////////

struct SqNtpTime
{
	SQInteger hour;
	SQInteger minute;
	SQInteger second;
	std::string tostring(void)
	{
		char buf[9];
		snprintf(buf, 9, "%02d:%02d:%02d", hour, minute, second);
		return buf;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////

class SqFishino : public FishinoClass
{
	private:
		
	protected:
		
	public:
		
		std::string getStaSSID()
		{
			char *ssid, *pass;
			if(!getStaConfig(ssid, pass))
				return "";
			std::string res = ssid;
			free(ssid);
			free(pass);
			return res;
		}
		
		std::string getStaPass()
		{
			char *ssid, *pass;
			if(!getStaConfig(ssid, pass))
				return "";
			std::string res = pass;
			free(ssid);
			free(pass);
			return res;
		}
		
		SqMacAddress macAddress(void)
		{
			const uint8_t *a = FishinoClass::macAddress();
			return SqMacAddress(a[0], a[1], a[2], a[3], a[4], a[5]);
		}

		SqMacAddress BSSID(void)
		{
			const uint8_t* a = FishinoClass::BSSID();
			return SqMacAddress(a[0], a[1], a[2], a[3], a[4], a[5]);
		}

		SqMacAddress BSSID(uint8_t n)
		{
			uint8_t buf[6];
			const uint8_t* a = FishinoClass::BSSID(n, buf);
			return SqMacAddress(a[0], a[1], a[2], a[3], a[4], a[5]);
		}
		
		bool setStaMAC(SqMacAddress const &mac)
		{
			return FishinoClass::setStaMAC(mac._bytes);
		}

		bool setApMAC(SqMacAddress const &mac)
		{
			return FishinoClass::setApMAC(mac._bytes);
		}

		IPAddress hostByName(const char *aHostname)
		{
			IPAddress res;
			FishinoClass::hostByName(aHostname, res);
			return res;
		}

		std::string softApGetSSID(void)
		{
			char *r = FishinoClass::softApGetSSID();
			std::string res = r;
			free(r);
			return res;
		}
		
		std::string softApGetPassword(void)
		{
			char *r = FishinoClass::softApGetPassword();
			std::string res = r;
			free(r);
			return res;
		}
		
		IPAddress ntpGetServer(void)
		{
			IPAddress res;
			FishinoClass::ntpGetServer(res);
			return res;
		}

		SqNtpTime ntpTime(void)
		{
			uint8_t h, m, s;
			FishinoClass::ntpTime(h, m, s);
			SqNtpTime nt;
			nt.hour = h;
			nt.minute = m;
			nt.second = s;
			return nt;
		}
};

//////////////////////////////////////////////////////////////////////////////////////////

class _SqFishinoClient : public SqStream
{
	private:
		
	protected:
		
		FishinoClient *_client;
		
	public:
		
		_SqFishinoClient() { _client = NULL; }
		virtual ~_SqFishinoClient() { if(_client) { delete _client; } }
		
		// stream interface
		virtual void  flush(void)
			{ _client->flush(); }
		virtual size_t  write(char c)
			{ return _client->write(c); }
		virtual size_t  write(const char *buf, size_t len)
			{ return _client->write(buf, len); }
		virtual int  read(void)
			{ return _client->read(); }
		virtual size_t  read(char *buf, size_t maxLen)
			{ return _client->read((uint8_t *)buf, maxLen); }
		virtual int  available(void)
			{ return _client->available(); }
		virtual int  peek(void)
			{ return _client->peek(); }
		virtual size_t  tell(void)
			{ return 0; }
		virtual size_t  length(void)
			{ return _client->available(); }
		virtual bool  eof(void)
			{ return !_client->available(); }
			
		// FishinoClient functions
		uint8_t  getSocket(void) const
			{ return _client->getSocket(); }
		bool status()
			{ return _client->status(); }
		int connect(IPAddress ip, uint16_t port)
			{ return _client->connect(ip, port); }
		int connect(const char *host, uint16_t port)
			{ return _client->connect(host, port); }
		void stop()
			{ _client->stop(); }
		uint8_t connected()
			{ return _client->connected(); }
		bool setBufferedMode(bool b)
			{ return _client->setBufferedMode(b); }
		bool getBufferedMode(void)
			{ return _client->getBufferedMode(); }
		bool setNoDelay(bool b)
			{ return _client->setNoDelay(b); }
		bool getNoDelay(void)
			{ return _client->getNoDelay(); }
		bool setForceCloseTime(uint32_t tim)
			{ return _client->setForceCloseTime(tim); }
		uint32_t getForceCloseTime(void)
			{ return _client->getForceCloseTime(); }
};

//////////////////////////////////////////////////////////////////////////////////////////

class SqFishinoClient : public _SqFishinoClient
{
	private:
		
	protected:
		
	public:
		
		SqFishinoClient() { _client = new FishinoClient(); }
		SqFishinoClient(FishinoClient const &c) { _client = new FishinoClient(c); }
		SqFishinoClient(SqFishinoClient const &c) { _client = new FishinoClient(*c._client); }
		~SqFishinoClient() {}
};

//////////////////////////////////////////////////////////////////////////////////////////

class SqFishinoSecureClient : public _SqFishinoClient
{
	private:
		
	protected:
		
	public:
		
		SqFishinoSecureClient() { _client = new FishinoSecureClient(); }
		SqFishinoSecureClient(FishinoSecureClient const &c) { _client = new FishinoSecureClient(c); }
		SqFishinoSecureClient(SqFishinoSecureClient const &c) { _client = new FishinoSecureClient(*c._client); }
		~SqFishinoSecureClient() {}
};

//////////////////////////////////////////////////////////////////////////////////////////

class SqFishinoServer : public FishinoServer
{
	private:
		
	protected:
		
	public:
		
		SqFishinoServer(uint16_t port) : FishinoServer(port) {}
		
		SqFishinoClient available() { return SqFishinoClient(FishinoServer::available()); }
		
};

//////////////////////////////////////////////////////////////////////////////////////////

class SqFishinoUDP : public SqStream
{
	private:
		
		FishinoUDP _udp;
		
	protected:
		
	public:
		
		SqFishinoUDP() {}
		SqFishinoUDP(FishinoUDP const &u) : _udp(u) {}
		SqFishinoUDP(SqFishinoUDP const &u) : _udp(u._udp) {}
		virtual ~SqFishinoUDP() { _udp.stop(); }
		
		// stream interface
		virtual void  flush(void)
			{ _udp.flush(); }
		virtual size_t  write(char c)
			{ return _udp.write(c); }
		virtual size_t  write(const char *buf, size_t len)
			{ return _udp.write(buf, len); }
		virtual int  read(void)
			{ return _udp.read(); }
		virtual size_t  read(char *buf, size_t maxLen)
			{ return _udp.read((uint8_t *)buf, maxLen); }
		virtual int  available(void)
			{ return _udp.available(); }
		virtual int  peek(void)
			{ return _udp.peek(); }
		virtual size_t  tell(void)
			{ return 0; }
		virtual size_t  length(void)
			{ return _udp.available(); }
		virtual bool  eof(void)
			{ return !_udp.available(); }
			
		uint8_t begin(uint16_t p)
			{ return _udp.begin(p); }
		void stop()
			{ return _udp.stop(); }
		int beginPacket(IPAddress ip, uint16_t port)
			{ return _udp.beginPacket(ip, port); }
		int beginPacket(const char *host, uint16_t port)
			{ return _udp.beginPacket(host, port); }
		int endPacket()
			{ return _udp.endPacket(); }
		int parsePacket()
			{ return _udp.parsePacket(); }
		IPAddress remoteIP()
			{ return _udp.remoteIP(); }
		uint16_t remotePort()
			{ return _udp.remotePort(); }
};

//////////////////////////////////////////////////////////////////////////////////////////




SQInteger registerFishinoLib(HSQUIRRELVM v)
{
	RootTable(v)
		.Class<SqIPAddress>("IPAddress"_LIT)
			.Alias<IPAddress>()
			.Constructor<>											()
			.Constructor<uint8_t, uint8_t, uint8_t, uint8_t>		()
			.Constructor<uint32_t>									()
			.Func													("_tostring"_LIT				, &SqIPAddress::tostring)
			--
		.Class<SqMacAddress>("MacAddress"_LIT)
			.Constructor<>											()
			.Constructor<uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t>		()
			.Func													("_tostring"_LIT				, &SqMacAddress::tostring)
			--
		.Class<SqNtpTime>("NtpTime"_LIT)
			.Var("hour"_LIT			, &SqNtpTime::hour)
			.Var("minute"_LIT		, &SqNtpTime::minute)
			.Var("second"_LIT		, &SqNtpTime::second)
			.Func("_tostring"_LIT	, &SqNtpTime::tostring)
			--
		.Class<SqFishino>("FishinoClass"_LIT)
			.NoCreate()
			.Func													("reset"_LIT					, &SqFishino::reset)
			.Func													("firmwareVersion"_LIT			, &SqFishino::firmwareVersion)
			.Func													("firmwareVersionStr"_LIT		, &SqFishino::firmwareVersionStr)
/*
			.Func													("getLastError"				, &SqFishino::getLastError)
			.Func													("getLastErrorString"		, &SqFishino::getLastErrorString)
			.Func													("getErrorString"			, &SqFishino::getErrorString)
			.Func													("clearLastError"			, &SqFishino::clearLastError)
*/
			.Func													("setMode"_LIT					, &SqFishino::setMode)
			.Func<uint8_t, const char *>							("begin"_LIT					, &SqFishino::begin)
			.Func<uint8_t, const char *, const char *>				("begin"_LIT					, &SqFishino::begin)
			.Func<bool, const char *>								("setStaConfig"_LIT				, &SqFishino::setStaConfig)
			.Func<bool, const char *, const char *>					("setStaConfig"_LIT				, &SqFishino::setStaConfig)
			.Func													("getStaSSID"_LIT				, &SqFishino::getStaSSID)
			.Func													("getStaPass"_LIT				, &SqFishino::getStaPass)
			.Func													("joinAp"_LIT					, &SqFishino::joinAp)
			.Func													("quitAp"_LIT					, &SqFishino::quitAp)
			.Func<bool, IPAddress>									("config"_LIT					, &SqFishino::config)
			.Func<bool, IPAddress, IPAddress>						("config"_LIT					, &SqFishino::config)
			.Func<bool, IPAddress, IPAddress, IPAddress>			("config"_LIT					, &SqFishino::config)
			.Func<bool, IPAddress, IPAddress, IPAddress, IPAddress>	("config"_LIT					, &SqFishino::config)
			.Func<bool, IPAddress>									("setDNS"_LIT					, &SqFishino::setDNS)
			.Func<bool, IPAddress, IPAddress>						("setDNS"_LIT					, &SqFishino::setDNS)
			.Func<bool>												("disconnect"_LIT				, &SqFishino::disconnect)
			.Func													("macAddress"_LIT				, &SqFishino::macAddress)
			.Func													("localIP"_LIT					, &SqFishino::localIP)
			.Func													("subnetMask"_LIT				, &SqFishino::subnetMask)
			.Func													("gatewayIP"_LIT				, &SqFishino::gatewayIP)
			.Func<String>											("SSID"_LIT						, &SqFishino::SSID)
			.Func<SqMacAddress>										("BSSID"_LIT					, &SqFishino::BSSID)			
			.Func<int32_t>											("RSSI"_LIT						, &SqFishino::RSSI)			
			.Func<uint8_t>											("encryptionType"_LIT			, &SqFishino::encryptionType)			
			.Func													("scanNetworks"_LIT				, &SqFishino::scanNetworks)
			.Func													("getNumNetworks"_LIT			, &SqFishino::getNumNetworks)
			.Func<String, uint8_t>									("SSID"_LIT						, &SqFishino::SSID)
			.Func<SqMacAddress, uint8_t>							("BSSID"_LIT					, &SqFishino::BSSID)			
			.Func<int32_t, uint8_t>									("RSSI"_LIT						, &SqFishino::RSSI)			
			.Func<uint8_t, uint8_t>									("encryptionType"_LIT			, &SqFishino::encryptionType)			
			.Func<uint8_t>											("status"_LIT					, &SqFishino::status)			
			.Func<bool, const char *>								("setHostName"_LIT				, &SqFishino::setHostName)
			.Func													("getHostName"_LIT				, &SqFishino::getHostName)
			.Func													("setStaIP"_LIT					, &SqFishino::setStaIP)
			.Func													("setStaMAC"_LIT				, &SqFishino::setStaMAC)
			.Func													("setStaGateway"_LIT			, &SqFishino::setStaGateway)
			.Func													("setStaNetMask"_LIT			, &SqFishino::setStaNetMask)
			.Func													("staStartDHCP"_LIT				, &SqFishino::staStartDHCP)
			.Func													("staStopDHCP"_LIT				, &SqFishino::staStopDHCP)
			.Func													("getStaDHCPStatus"_LIT			, &SqFishino::getStaDHCPStatus)
			.Func													("setApIP"_LIT					, &SqFishino::setApIP)
			.Func													("setApMAC"_LIT					, &SqFishino::setApMAC)
			.Func													("setApGateway"_LIT				, &SqFishino::setApGateway)
			.Func													("setApNetMask"_LIT				, &SqFishino::setApNetMask)
			.Func													("setApIPInfo"_LIT				, &SqFishino::setApIPInfo)
			.Func													("hostByName"_LIT				, &SqFishino::hostByName)
			.Func<bool>												("softApStartDHCPServer"_LIT	, &SqFishino::softApStartDHCPServer)
			.Func<bool, IPAddress, IPAddress>						("softApStartDHCPServer"_LIT	, &SqFishino::softApStartDHCPServer)
			.Func													("softApStopDHCPServer"_LIT		, &SqFishino::softApStopDHCPServer)
			.Func													("getSoftApDHCPServerStatus"_LIT, &SqFishino::getSoftApDHCPServerStatus)
			.Func													("softApGetSSID"_LIT			, &SqFishino::softApGetSSID)
			.Func													("softApGetPassword"_LIT		, &SqFishino::softApGetPassword)
			.Func													("softApGetChannel"_LIT			, &SqFishino::softApGetChannel)
			.Func													("softApGetHidden"_LIT			, &SqFishino::softApGetHidden)
			.Func<bool, const char *, const char *, uint8_t, bool>	("softApConfig"_LIT				, &SqFishino::softApConfig)
			.Func													("setMaxTcpConnections"_LIT		, &SqFishino::setMaxTcpConnections)
			.Func													("getMaxTcpConnections"_LIT		, &SqFishino::getMaxTcpConnections)
			.Func													("getPhyMode"_LIT				, &SqFishino::getPhyMode)
			.Func													("setPhyMode"_LIT				, &SqFishino::setPhyMode)
			.Func<bool, IPAddress const &>							("ntpSetServer"_LIT				, &SqFishino::ntpSetServer)
			.Func<bool, const char *>								("ntpSetServer"_LIT				, &SqFishino::ntpSetServer)
			.Func													("ntpGetServer"_LIT				, &SqFishino::ntpGetServer)
			.Func													("ntpEpoch"_LIT					, &SqFishino::ntpEpoch)
			.Func<SqNtpTime>										("ntpTime"_LIT					, &SqFishino::ntpTime)
			
			--
		.Instance<SqFishino>("Fishino"_LIT, "FishinoClass"_LIT, (SqFishino &)Fishino)
		
		.Class<_SqFishinoClient>("Stream"_LIT, "_FishinoClient"_LIT)
			.NoCreate()
			.Func													("getSocket"_LIT				, &SqFishinoClient::getSocket)
			.Func													("status"_LIT					, &SqFishinoClient::status)
			.Func<int, IPAddress, uint16_t>							("connect"_LIT					, &SqFishinoClient::connect)
			.Func<int, const char *, uint16_t>						("connect"_LIT					, &SqFishinoClient::connect)
			.Func													("stop"_LIT						, &SqFishinoClient::stop)
			.Func													("connected"_LIT				, &SqFishinoClient::connected)

			.Func													("setBufferedMode"_LIT			, &SqFishinoClient::setBufferedMode)
			.Func													("getBufferedMode"_LIT			, &SqFishinoClient::getBufferedMode)
			.Func													("setNoDelay"_LIT				, &SqFishinoClient::setNoDelay)
			.Func													("getNoDelay"_LIT				, &SqFishinoClient::getNoDelay)
			.Func													("setForceCloseTime"_LIT		, &SqFishinoClient::setForceCloseTime)
			.Func													("getForceCloseTime"_LIT		, &SqFishinoClient::getForceCloseTime)
			--

		.Class<SqFishinoClient>("_FishinoClient"_LIT, "FishinoClient"_LIT)
			.Constructor<>											()
			.Constructor<SqFishinoClient const &>					()
			--

		.Class<SqFishinoSecureClient>("_FishinoClient"_LIT, "FishinoSecureClient"_LIT)
			.Constructor<>											()
			.Constructor<SqFishinoSecureClient const &>				()
			--
			
		.Class<SqFishinoServer>("FishinoServer"_LIT)
			.Constructor<uint16_t>									()
			.Func													("hasClients"_LIT				, &SqFishinoServer::hasClients)
			.Func													("available"_LIT				, &SqFishinoServer::available)
			.Func													("begin"_LIT					, &SqFishinoServer::begin)
			.Func													("close"_LIT					, &SqFishinoServer::close)
			.Func													("stop"_LIT						, &SqFishinoServer::stop)
			.Func													("setNoDelay"_LIT				, &SqFishinoServer::setNoDelay)
			.Func													("getNoDelay"_LIT				, &SqFishinoServer::getNoDelay)
			.Func<size_t, uint8_t>									("write"_LIT					, &SqFishinoServer::write)
			.Func<size_t, const char *, size_t>						("write"_LIT					, &SqFishinoServer::write)
			.Func													("setBufferedMode"_LIT			, &SqFishinoServer::setBufferedMode)
			.Func													("getBufferedMode"_LIT			, &SqFishinoServer::getBufferedMode)
			.Func													("setClientsForceCloseTime"_LIT	, &SqFishinoServer::setClientsForceCloseTime)
			.Func													("getClientsForceCloseTime"_LIT	, &SqFishinoServer::getClientsForceCloseTime)
			.Func													("setMaxClients"_LIT			, &SqFishinoServer::setMaxClients)
			.Func													("getMaxClients"_LIT			, &SqFishinoServer::getMaxClients)
			--
			
		.Class<SqFishinoUDP>("FishinoUPD"_LIT)
			.Constructor<>											()
			.Constructor<SqFishinoUDP const &>						()
			.Func													("begin"_LIT					, &SqFishinoUDP::begin)
			.Func													("stop"_LIT						, &SqFishinoUDP::stop)
			.Func<int, IPAddress, uint16_t>							("beginPacket"_LIT				, &SqFishinoUDP::beginPacket)
			.Func<int, const char *, uint16_t>						("beginPacket"_LIT				, &SqFishinoUDP::beginPacket)
			.Func													("endPacket"_LIT				, &SqFishinoUDP::endPacket)
			.Func													("parsePacket"_LIT				, &SqFishinoUDP::parsePacket)
			.Func													("remoteIP"_LIT					, &SqFishinoUDP::remoteIP)
			.Func													("remotePort"_LIT				, &SqFishinoUDP::remotePort)
			--
	;
	
	return 0;
}
