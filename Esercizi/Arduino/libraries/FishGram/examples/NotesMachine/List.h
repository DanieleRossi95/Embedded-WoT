#ifndef __List_h_
#define __List_h_

// generic list template
#include <Arduino.h>

template<class C>class List;

template<class C>class ListElement
{
	friend class List<C>;
	
	protected:
	
		ListElement<C>*_prev, *_next;
		
		// constructor
		ListElement()
		{
			_prev = _next = NULL;
		}
		
		// destructor
		virtual ~ListElement()
		{
		};
	public:

		inline C *next(void) { return (C*)_next; }
		inline C *prev(void) { return (C*)_prev; }
};

template<class C>class List
{
	friend class ListElement<C>;
	private:
	
		// first and last element in list
		ListElement<C>*_head, *_tail;
		
		// number of elements in list
		uint16_t _count;
		
	public:
	
		List()
		{
			_head = _tail = NULL;
			_count = 0;
		}
		
		~List()
		{
			clear();
		}
		
		void clear()
		{
			ListElement<C>* item;
			while(_head)
			{
				item = _head->_next;
				delete _head;
				_head = item;
			}
			_tail = NULL;
			_count = 0;
		}
	
		// add an element to list
		ListElement<C>* add(C *newItem)
		{
			newItem->_prev = _tail;
			newItem->_next = NULL;
			if(_tail)
				_tail->_next = newItem;
			if(!_head)
				_head = newItem;
			_tail = newItem;
			_count++;
			return newItem;
		}
		
		// remove an element from list
		bool remove(ListElement<C>* item)
		{
			if(!item)
				return false;
			if(item->_prev)
				item->_prev->_next = item->_next;
			else
				_head = item->_next;
			if(item->_next)
				item->_next->_prev = item->_prev;
			else
				_tail = item->_prev;
			delete item;
			_count--;
			return true;
		}
		
		// get a pointer to first and last elements
		C *head(void) { return (C*)_head; }
		C *tail(void) { return (C*)_tail; }
		
		// detach first or last element from list
		// and return its pointer
		C* popHead(void)
		{
			if(!_head)
				return NULL;
			ListElement<C>* item = _head;
			_head = _head->_next;
			if(!_head)
				_tail = NULL;
			else
				_head->_prev = NULL;
			_count--;
			return (C *)item;
		}
		
		C* popTail(void)
		{
			if(!_tail)
				return NULL;
			ListElement<C>* item = _tail;
			_tail = _tail->_prev;
			if(!_tail)
				_head = NULL;
			else
				_tail->_next = NULL;
			_count--;
			return (C *)item;
		}
		
		// appends an element at head or at tail
		C* addTail(C *elem) { return add(elem); }
		C* addHead(C *elem)
		{
			elem->_next = _head;
			elem->_prev = NULL;
			if(_head)
				_head->_prev = elem;
			if(!_tail)
				_tail = elem;
			_head = elem;
			elem->_owner = this;
			_count++;
			return elem;
		}
		
		// drops an element from head or tail
		void dropHead(void)
		{
			C* item = popHead();
			if(item)
				delete item;
		}
		void dropTail(void)
		{
			C* item = popTail();
			if(item)
				delete item;
		}
		
		uint16_t count() { return _count; }
};

#endif
