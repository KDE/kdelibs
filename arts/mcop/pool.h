    /*

    Copyright (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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

template <class T>
class Pool {
	stack<unsigned long> freeIDs;
	vector<T *> storage;
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
};
