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

#ifndef TYPE_H
#define TYPE_H

#include "buffer.h"

/*
 * BC - Status (2000-09-30): Type
 *
 * Keep binary compatible. DO NOT TOUCH, DO NOT CHANGE.
 */

namespace Arts {

/**
 * Base class for the IDL mapping of struct-like types
 */
class Type {
private:
	static long _staticTypeCount;

public:
	inline static long _typeCount() { return _staticTypeCount; }
	/**
	 * constructor
	 */
	inline Type()
	{
		_staticTypeCount++;
	}

	/**
	 * copy constructor
	 */
	inline Type(const Type&)
	{
		_staticTypeCount++;
	}

	/**
	 * virtual destructor (since we have virtual functions and this stuff
	 */
	virtual ~Type();

	// FIXME:
	//
	// How exactly should error handling for marshalling look? Is asking
	// the stream wether a read error occured after demarshalling a type
	// enough?

	/**
	 * read the type from a stream
	 */
	virtual void readType(Buffer& stream) = 0;

	/**
	 * write the type to a stream
	 */
	virtual void writeType(Buffer& stream) const = 0;
};

};

#endif
