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

#ifndef COMMON_H
#define COMMON_H

#include "buffer.h"
#include "type.h"
#include "dispatcher.h"
#include "object.h"
#include "connection.h"
#include "objectmanager.h"
#include "idlfilereg.h"
#include "asyncstream.h"
#include "mcoputils.h"
#include "anyref.h"

/*
 * BC - Status (2000-09-30): Read/WriteObject(Seq)
 *
 * Will be kept binary compatible by NOT TOUCHING AT ALL. Do not change this.
 * (Interaction with generated code).
 */

/* define this to see what mcop transfers around */
#undef DEBUG_IO

/* define this to see what mcop messages are processed */
#undef DEBUG_MESSAGES

#include "core.h"

namespace Arts {
/* some marshalling helpers */

template<class T>
void readTypeSeq(Buffer& stream, std::vector<T>& sequence) {
	sequence.clear();

	unsigned long l = stream.readLong();
	while(l--) sequence.push_back(T(stream));
}

template<class T>
void writeTypeSeq(Buffer& stream, const std::vector<T>& sequence) {
	stream.writeLong(sequence.size());
	for(unsigned long l=0;l<sequence.size();l++)
		sequence[l].writeType(stream);
}

template<class T>
void writeObject(Buffer& stream, T* object) {
	if(object)
	{
		/*
	 	 * perhaps reimplement directly (without conversion to/from string)
	 	 * for more speed
	 	 */

		std::string s = object->_toString();

		Buffer buffer;
		buffer.fromString(s,"MCOP-Object");
		ObjectReference reference(buffer);

		object->_copyRemote();	// prevents that the object is freed for a while
		reference.writeType(stream);
	}
	else
	{
		ObjectReference null_reference;

		null_reference.serverID = "null";
		null_reference.objectID = 0;
		null_reference.writeType(stream);
	}
}

template<class T>
void readObject(Buffer& stream, T*& result) {
	ObjectReference reference(stream);

	if(reference.serverID == "null")
		result = 0;		// null reference?
	else
		result = T::_fromReference(reference,false);
}

template<class T>
void readObjectSeq(Buffer& stream, std::vector<T>& sequence)
{
	sequence.clear();

	unsigned long l = stream.readLong();
	while(l--)
	{
		typename T::_base_class *temp;
		readObject(stream, temp);
		sequence.push_back(T::_from_base(temp));
	}
}

template<class T>
void writeObjectSeq(Buffer& stream, const std::vector<T>& sequence)
{
	stream.writeLong(sequence.size());

	for(unsigned long l=0;l<sequence.size();l++)
	{
		T object = sequence[l];
		writeObject(stream,object._base());
	}
}

#ifndef MCOPBYTE_DEFINED
#define MCOPBYTE_DEFINED
typedef unsigned char mcopbyte;
#endif

};
#endif
