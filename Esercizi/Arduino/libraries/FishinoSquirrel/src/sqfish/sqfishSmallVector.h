#ifndef __SQFISH__SMALLVECTOR__H
#define __SQFISH__SMALLVECTOR__H

#include <stddef.h>
#include <stdlib.h>

// a small memory footprint vector class
// beware, it can handle ONLY moveable data
// and has no way to remove data from it
namespace sqfish {

	template<typename T> class smallvec
	{
		private:
			
			size_t _count;
			T* _data;
			
		protected:
			
		public:
			
			smallvec()
			{
				_count = 0;
				_data = NULL;
			}
			smallvec(T && src)
			{
				_data = src._data;
				_count = src._count;
				src._data = NULL;
				src._count = 0;
			}
			~smallvec()
			{
				clear();
			}
			T &add(T && t) {
				if(_data)
					_data = (T *)realloc(_data, sizeof(T)*(_count + 1));
				else
					_data = (T *)malloc(sizeof(T)*(_count + 1));
				new(&_data[_count])T(t);
				_count++;
				return _data[_count - 1];
			}
			size_t getCount(void) const { return _count; }
			T const &operator[](size_t idx) const { return _data[idx]; }
			T &operator[](size_t idx) { return _data[idx]; }
			void clear(void)
			{
				for(size_t i = 0; i < _count; i++)
					_data[i].~T();
				if(_data)
					free(_data);
				_data = NULL;
				_count = 0;
			}
			
	} __attribute__((packed));


}; // end sqfish namespace

#endif
