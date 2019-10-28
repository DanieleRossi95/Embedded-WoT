#include "sqfish.h"

namespace sqfish
{
	
	/////////////////////////////////////////////////////////////
	//                     STACK GETTERS                       //
	/////////////////////////////////////////////////////////////
	
	template<> char			GetParam<char		>(HSQUIRRELVM vm, int pPos)	{
		SQInteger res;
		sq_getinteger(vm, pPos, &res);
		return (char)res;
	}

	template<> short		GetParam<short		>(HSQUIRRELVM vm, int pPos) {
		SQInteger res;
		sq_getinteger(vm, pPos, &res);
		return (short)res;
	}

	template<> int			GetParam<int		>(HSQUIRRELVM vm, int pPos)	{
		SQInteger res;
		sq_getinteger(vm, pPos, &res);
		return (int)res;
	}

	template<> long			GetParam<long		>(HSQUIRRELVM vm, int pPos)	{
		SQInteger res;
		sq_getinteger(vm, pPos, &res);
		return (long)res;
	}

	template<> long long	GetParam<long long	>(HSQUIRRELVM vm, int pPos)	{
		SQInteger res;
		sq_getinteger(vm, pPos, &res);
		return (long long)res;
	}

	template<> unsigned char		GetParam<unsigned char		>(HSQUIRRELVM vm, int pPos)
		{ return (unsigned char)GetParam<char>(vm, pPos); }
	template<> unsigned short		GetParam<unsigned short		>(HSQUIRRELVM vm, int pPos)
		{ return (unsigned short)GetParam<short>(vm, pPos); }
	template<> unsigned int			GetParam<unsigned int		>(HSQUIRRELVM vm, int pPos)
		{ return (unsigned int)GetParam<int>(vm, pPos); }
	template<> unsigned long		GetParam<unsigned long		>(HSQUIRRELVM vm, int pPos)
		{ return (unsigned long)GetParam<long>(vm, pPos); }
	template<> unsigned long long	GetParam<unsigned long long	>(HSQUIRRELVM vm, int pPos)
		{ return (unsigned long long)GetParam<long long>(vm, pPos); }

	// specializations for float types
	template<> double GetParam<double>(HSQUIRRELVM vm, int pPos) {
		SQFloat res;
		sq_getfloat(vm, pPos, &res);
		return (double)res;
	}
	
	template<> float GetParam<float>(HSQUIRRELVM vm, int pPos)
		{ return (float) GetParam<double>(vm, pPos); }
	
	// specializations for string types
	template<> char * GetParam<char *>(HSQUIRRELVM vm, int pPos) {
		const SQChar *res;
		sq_getstring(vm, pPos, &res);
		return (char *)res;
	}
	
	template<> const char * GetParam<const char *>(HSQUIRRELVM vm, int pPos)
		{ return GetParam<char *>(vm, pPos); }
	
	template<> std::string GetParam<std::string>(HSQUIRRELVM vm, int pPos) {
		const SQChar *res;
		sq_getstring(vm, pPos, &res);
		return res;
	}
	
	/////////////////////////////////////////////////////////////
	//                     STACK SETTERS                       //
	/////////////////////////////////////////////////////////////
	
	// specialization for boolean
	template<> void PushValue<bool		>(HSQUIRRELVM vm, bool		val  ) {
		sq_pushbool(vm, (SQBool)val);
	}

	// specializations for integer types
	template<> void PushValue<char		>(HSQUIRRELVM vm, char		val  ) {
		sq_pushinteger(vm, (SQInteger)val);
	}

	template<> void PushValue<short		>(HSQUIRRELVM vm, short		val  ) {
		sq_pushinteger(vm, (SQInteger)val);
	}

	template<> void PushValue<int		>(HSQUIRRELVM vm, int		val  ) {
		sq_pushinteger(vm, (SQInteger)val);
	}

	template<> void PushValue<long		>(HSQUIRRELVM vm, long		val  ) {
		sq_pushinteger(vm, (SQInteger)val);
	}

	template<> void PushValue<long long	>(HSQUIRRELVM vm, long long	val  ) {
		sq_pushinteger(vm, (SQInteger)val);
	}

	template<> void PushValue<unsigned char			>(HSQUIRRELVM vm, unsigned char			val  ) {
		sq_pushinteger(vm, (SQInteger)val);
	}

	template<> void PushValue<unsigned short		>(HSQUIRRELVM vm, unsigned short		val  ) {
		sq_pushinteger(vm, (SQInteger)val);
	}

	template<> void PushValue<unsigned int			>(HSQUIRRELVM vm, unsigned int			val  ) {
		sq_pushinteger(vm, (SQInteger)val);
	}

	template<> void PushValue<unsigned long			>(HSQUIRRELVM vm, unsigned long			val  ) {
		sq_pushinteger(vm, (SQInteger)val);
	}

	template<> void PushValue<unsigned long long	>(HSQUIRRELVM vm, unsigned long long	val  ) {
		sq_pushinteger(vm, (SQInteger)val);
	}

	// specializations for float types
	template<> void PushValue<float>(HSQUIRRELVM vm, float val) {
		sq_pushfloat(vm, (SQFloat)val);
	}
	
	template<> void PushValue<double>(HSQUIRRELVM vm, double val) {
		sq_pushfloat(vm, (SQFloat)val);
	}
	
	template<> void PushValue<long double>(HSQUIRRELVM vm, long double val) {
		sq_pushfloat(vm, (SQFloat)val);
	}
	
	// specializations for string types
	template<> void PushValue<char *>(HSQUIRRELVM vm,  char *val) {
		sq_pushstring(vm, val, strlen(val), SQFalse);
	}
	
	template<> void PushValue<const char *>(HSQUIRRELVM vm, const char *val) {
		sq_pushstring(vm, val, strlen(val), SQFalse);
	}

	template<> void PushValue<std::string>(HSQUIRRELVM vm, std::string val)
	{
		sq_pushstring(vm, val.c_str(), val.length(), SQFalse);
	}

	template<> void PushValue<std::string &>(HSQUIRRELVM vm, std::string &val)
	{
		sq_pushstring(vm, val.c_str(), val.length(), SQFalse);
	}
	
	template<> void PushValue<std::string const &>(HSQUIRRELVM vm, std::string const &val)
	{
		sq_pushstring(vm, val.c_str(), val.length(), SQFalse);
	}

#ifdef _FISHINO_PIC32_
	template<> void PushValue<String>(HSQUIRRELVM vm, String val)
	{
		sq_pushstring(vm, val.c_str(), val.length(), SQFalse);
	}

	template<> void PushValue<String &>(HSQUIRRELVM vm, String &val)
	{
		sq_pushstring(vm, val.c_str(), val.length(), SQFalse);
	}

	template<> void PushValue<String const &>(HSQUIRRELVM vm, String const &val)
	{
		sq_pushstring(vm, val.c_str(), val.length(), SQFalse);
	}
#endif

}; // end namespace sqfish

