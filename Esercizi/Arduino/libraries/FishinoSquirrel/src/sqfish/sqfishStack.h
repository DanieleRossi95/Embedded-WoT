#ifndef __SQFISH__STACK__H
#define __SQFISH__STACK__H

#include <stdint.h>

#ifdef UPP_TEST
#include <Core/Core.h>
using namespace Upp;
#else
#define DEBUG_LEVEL_ERROR
#include <FishinoDebug.h>
#endif

namespace sqfish
{

	/////////////////////////////////////////////////////////////
	//                     STACK GETTERS                       //
	/////////////////////////////////////////////////////////////
	
	// generic fall-back template for unsupported parameters
	// gets a pointer for a class instance from stack
	// the class MUST be a C++ class registered on system
	// otherwise you'll get a runtime error
	// the class must also have a copy constructor
	template<typename T> T GetParam(HSQUIRRELVM vm, int pPos) {
		HSQOBJECT obj;
		sq_getstackobj(vm, pPos, &obj);
		void *instance = utils::getInstance(obj, vm);

		// because of the way squirrel handles it, we can't throw an error
		// from here if the instance is null. So... we just return it
		// which will cause an undefined behaviour in case T is not a
		// registered class
		typedef typename utils::remove_all<T>::type TT;
		return *(TT *)instance;
	}
	
	// specializations for integer types
	template<> char			GetParam<char		>(HSQUIRRELVM vm, int pPos) ;
	template<> short		GetParam<short		>(HSQUIRRELVM vm, int pPos) ;
	template<> int			GetParam<int		>(HSQUIRRELVM vm, int pPos) ;
	template<> long			GetParam<long		>(HSQUIRRELVM vm, int pPos) ;
	template<> long long	GetParam<long long	>(HSQUIRRELVM vm, int pPos) ;

	template<> unsigned char		GetParam<unsigned char		>(HSQUIRRELVM vm, int pPos) ;
	template<> unsigned short		GetParam<unsigned short		>(HSQUIRRELVM vm, int pPos) ;
	template<> unsigned int			GetParam<unsigned int		>(HSQUIRRELVM vm, int pPos) ;
	template<> unsigned long		GetParam<unsigned long		>(HSQUIRRELVM vm, int pPos) ;
	template<> unsigned long long	GetParam<unsigned long long	>(HSQUIRRELVM vm, int pPos) ;

	// specializations for float types
	template<> float GetParam<float>(HSQUIRRELVM vm, int pPos) ;
	template<> double GetParam<double>(HSQUIRRELVM vm, int pPos) ;
	
	// specializations for string types
	template<> char * GetParam<char *>(HSQUIRRELVM vm, int pPos) ;
	template<> const char * GetParam<const char *>(HSQUIRRELVM vm, int pPos) ;
	
	/////////////////////////////////////////////////////////////
	//                     STACK SETTERS                       //
	/////////////////////////////////////////////////////////////
	
	// generic fall-back template for unsupported types
	// it pushes a class that MUST be registered with sqfish
	template<typename T> void _PushValue(HSQUIRRELVM vm, T *val, bool owns)
	{
		// get the squirrel class name corresponding to T
		const char *name = utils::getRegisteredClass(vm, typeid(T));

		// if it's an empty string, just push NULL... no way to trigger an exception from here
		if(!strcmp(name, ""))
		{
			if(owns)
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
				delete val;
			#pragma GCC diagnostic pop
			sq_pushnull(vm);
		}
		else
		{
			// we create an instance of class T on stack
			utils::PushInstance(vm, name, *val, owns);
		}
	}

	template<typename T> void PushValue(HSQUIRRELVM vm, T val) {
		_PushValue<T>(vm, new T(val), true);
	}
	
	// specialization for boolean
	template<> void PushValue<bool		>(HSQUIRRELVM vm, bool		val  ) ;

	// specializations for integer types
	template<> void PushValue<char		>(HSQUIRRELVM vm, char		val  ) ;
	template<> void PushValue<short		>(HSQUIRRELVM vm, short		val  ) ;
	template<> void PushValue<int		>(HSQUIRRELVM vm, int		val  ) ;
	template<> void PushValue<long		>(HSQUIRRELVM vm, long		val  ) ;
	template<> void PushValue<long long	>(HSQUIRRELVM vm, long long	val  ) ;
	
	template<> void PushValue<unsigned char			>(HSQUIRRELVM vm, unsigned char			val  ) ;
	template<> void PushValue<unsigned short		>(HSQUIRRELVM vm, unsigned short		val  ) ;
	template<> void PushValue<unsigned int			>(HSQUIRRELVM vm, unsigned int			val  ) ;
	template<> void PushValue<unsigned long			>(HSQUIRRELVM vm, unsigned long			val  ) ;
	template<> void PushValue<unsigned long long	>(HSQUIRRELVM vm, unsigned long long	val  ) ;
	
	// specializations for float types
	template<> void PushValue<float			>(HSQUIRRELVM vm, float			val) ;
	template<> void PushValue<double		>(HSQUIRRELVM vm, double		val) ;
	template<> void PushValue<long double	>(HSQUIRRELVM vm, long double	val) ;
	
	// specializations for string types
	template<> void PushValue<char *>(HSQUIRRELVM vm,  char *val) ;
	template<> void PushValue<const char *>(HSQUIRRELVM vm, const char *val) ;
	template<> void PushValue<std::string>(HSQUIRRELVM vm, std::string val) ;
	template<> void PushValue<std::string &>(HSQUIRRELVM vm, std::string &val) ;
	template<> void PushValue<std::string const &>(HSQUIRRELVM vm, std::string const &val);
	
#ifdef _FISHINO_PIC32_
	template<> void PushValue<String>(HSQUIRRELVM vm, String val);
	template<> void PushValue<String &>(HSQUIRRELVM vm, String &val);
	template<> void PushValue<String const &>(HSQUIRRELVM vm, String const &val);
#endif

	// template code to bind squirrel stack arguments to a function
	namespace detail
	{
		// The structure that encapsulates index lists
		template <SQInteger... INDEXES> struct index_list{
		};

		// Declare primary template for index range builder
		template <SQInteger MIN, SQInteger N, SQInteger... INDEXES>struct range_builder;
	
		// Base step
		template <SQInteger MIN, SQInteger... INDEXES> struct range_builder<MIN, MIN, INDEXES...>{
			typedef index_list<INDEXES...> type;
		};
	
		// Induction step
		template <SQInteger MIN, SQInteger N, SQInteger... INDEXES>
		struct range_builder : public range_builder < MIN, N - 1, N - 1, INDEXES... >{
		};
	
		// Meta-function that returns a [MIN, MAX) index range
		template<SQInteger MIN, SQInteger MAX> using index_range = typename range_builder<MIN, MAX>::type;
	
		//////////////////////////////////////////
		// for global functions returning a value
		template<typename RES, typename... ARGS, SQInteger... INDEXES> typename utils::disable_void<RES, RES>::type StackBind0(HSQUIRRELVM vm, RES (*fn)(ARGS...), index_list<INDEXES...>) {
			return fn(GetParam<ARGS>(vm, INDEXES - (SQInteger)sizeof...(ARGS))...);
		}
	
		template<typename RES, typename... ARGS> typename utils::disable_void<RES, RES>::type StackBind0(HSQUIRRELVM vm, RES(*fn)(ARGS...)) {
			return StackBind0(vm, fn, index_range<0, sizeof...(ARGS)>());
		}

		//////////////////////////////////////////
		// for global functions returning void
		template<typename RES, typename... ARGS, SQInteger... INDEXES> typename utils::enable_void<RES, RES>::type StackBind0(HSQUIRRELVM vm, RES (*fn)(ARGS...), index_list<INDEXES...>) {
			return fn(GetParam<ARGS>(vm, INDEXES - (SQInteger)sizeof...(ARGS))...);
		}
	
		template<typename RES, typename... ARGS> typename utils::enable_void<RES, RES>::type StackBind0(HSQUIRRELVM vm, RES(*fn)(ARGS...)) {
			return StackBind0(vm, fn, index_range<0, sizeof...(ARGS)>());
		}

		//////////////////////////////////////////
		// for member functions returning a value
		template<typename C, typename RES, typename... ARGS, SQInteger... INDEXES> typename utils::disable_void<RES, RES>::type StackBind0(HSQUIRRELVM vm, C *cls, RES (C::*fn)(ARGS...), index_list<INDEXES...>) {
			return (cls->*fn)(GetParam<ARGS>(vm, INDEXES - (SQInteger)sizeof...(ARGS))...);
		}
	
		template<typename C, typename RES, typename... ARGS> typename utils::disable_void<RES, RES>::type StackBind0(HSQUIRRELVM vm, C *cls, RES (C::*fn)(ARGS...)) {
			return StackBind0(vm, cls, fn, index_range<0, sizeof...(ARGS)>());
		}

		//////////////////////////////////////////
		// for member functions returning void
		template<typename C, typename RES, typename... ARGS, SQInteger... INDEXES> typename utils::enable_void<RES, RES>::type StackBind0(HSQUIRRELVM vm, C *cls, RES (C::*fn)(ARGS...), index_list<INDEXES...>) {
			(cls->*fn)(GetParam<ARGS>(vm, INDEXES - (SQInteger)sizeof...(ARGS))...);
		}
	
		template<typename C, typename RES, typename... ARGS> typename utils::enable_void<RES, RES>::type StackBind0(HSQUIRRELVM vm, C *cls, RES (C::*fn)(ARGS...)) {
			StackBind0(vm, cls, fn, index_range<0, sizeof...(ARGS)>());
		}

		//////////////////////////////////////////
		// for constructors
		template<typename C, typename... ARGS, SQInteger... INDEXES> C *StackBindNew0(HSQUIRRELVM vm, index_list<INDEXES...>) {
			return new C(GetParam<ARGS>(vm, INDEXES  - (SQInteger)sizeof...(ARGS))...);
		}
	
		template<typename C, typename... ARGS> C *StackBindNew0(HSQUIRRELVM vm) {
			return StackBindNew0<C, ARGS...>(vm, index_range<0, sizeof...(ARGS)>());
		}

	}
	
	//////////////////////////////////////////
	// for global functions returning a value
	template<typename RES, typename...ARGS> typename utils::disable_void<RES, RES>::type StackBind(HSQUIRRELVM vm, RES(*fn)(ARGS...)) {
		return detail::StackBind0(vm, fn);
	}

	//////////////////////////////////////////
	// for global functions returning void
	template<typename RES, typename...ARGS> typename utils::enable_void<RES, RES>::type StackBind(HSQUIRRELVM vm, RES(*fn)(ARGS...)) {
		return detail::StackBind0(vm, fn);
	}

	//////////////////////////////////////////
	// for member functions returning a value
	template<typename C, typename RES, typename...ARGS> typename utils::disable_void<RES, RES>::type StackBind(HSQUIRRELVM vm, C *cls, RES (C::*fn)(ARGS...)) {
		return detail::StackBind0(vm, cls, fn);
	}

	//////////////////////////////////////////
	// for member functions returning void
	template<typename C, typename RES, typename...ARGS> typename utils::enable_void<RES, RES>::type StackBind(HSQUIRRELVM vm, C *cls, RES (C::*fn)(ARGS...)) {
		detail::StackBind0(vm, cls, fn);
	}

	//////////////////////////////////////////
	// for constructors
	template<typename C, typename...ARGS> C *StackBindNew(HSQUIRRELVM vm) {
		return detail::StackBindNew0<C, ARGS...>(vm);
	}

}; // end namespace sqfish

#endif
