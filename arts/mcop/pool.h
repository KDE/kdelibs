    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#ifndef ARTS_POOL_H
#define ARTS_POOL_H


/*
 * BC - Status (2000-09-30): Pool<type>
 *
 * Needs to be kept binary compatible by NOT TOUCHING. When you want something
 * else, write a fresh one (used as part of Arts::Dispatcher, thus changing
 * this breaks Arts::Dispatcher binary compatibility).
 */


/**
 * A pool object of the type T keeps a pool of T* pointers, that are numbered.
 *
 * You allocate and release slots, and store T*'s in there. It should take
 * about no time to find a new free slot to store the T object into and to
 * release a slot to be reused.
 *
 * The pool object internally keeps track which slots are used.
 */
#include <stack>
#include <vector>
#include <list>

namespace Arts {

template <class T>
class Pool {
	std::stack<unsigned long> freeIDs;
	std::vector<T *> storage;
public:
	inline T*& operator[](unsigned long n) { return storage[n]; }
	inline void releaseSlot(unsigned long n) {
		freeIDs.push(n);
		storage[n] = 0;
	}
	unsigned long allocSlot() {
		unsigned long slot;
		if(freeIDs.empty())
		{
			unsigned long n;
			for(n=0;n<32;n++) {
				freeIDs.push(storage.size());
				storage.push_back(0);
			}
		}
		slot = freeIDs.top();
		freeIDs.pop();
		return slot;
	}
	std::list<T *> enumerate() {
		std::list<T *> items;
		//std::vector<T *>::iterator i;
		int n,max = storage.size();

		for(n=0; n < max; n++)
			if(storage[n]) items.push_back(storage[n]);

		return items;
	}
	unsigned long max() { return storage.size(); }
};

};
#endif /* POOL_H */
