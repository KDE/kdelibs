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

#include "anyref.h"

using namespace Arts;
using namespace std;

string AnyRefBase::type() const
{
	switch(rep)
	{
		case repVoid:		return "void";

		// primitive types
		case repByte:		return "byte";
		case repInt:
		case repLong:		return "long";
		case repFloat:
		case repDouble:		return "float";
		case repString:
		case repCString:	return "string";

		// sequences of primitive types
		case repByteSeq:	return "*byte";
		case repLongSeq:	return "*long";
		case repFloatSeq:	return "*float";
		case repStringSeq:	return "*string";
	}
	assert(false);
}

void AnyRefBase::_write(Buffer *b) const
{
	switch(rep)
	{
		case repVoid:	
			break;

		// primitive types
		case repByte:		b->writeByte(*(mcopbyte *)data);
			break;

		case repInt:		b->writeLong(*(int *)data);
			break;

		case repLong:		b->writeLong(*(long *)data);
			break;

		case repFloat:		b->writeFloat(*(float *)data);
			break;

		case repDouble:		b->writeFloat(*(double *)data);
			break;

		case repString:		b->writeString(*(string *)data);
			break;

		case repCString:	b->writeString((const char *)data);
			break;

		// sequences of primitive types
		case repByteSeq:	b->writeByteSeq(*(vector<mcopbyte> *)data);
			break;

		case repLongSeq:	b->writeLongSeq(*(vector<long> *)data);
			break;

		case repFloatSeq:	b->writeFloatSeq(*(vector<float> *)data);
			break;

		case repStringSeq:	b->writeStringSeq(*(vector<string> *)data);
			break;

		default:			assert(false);
	}
}

void AnyRefBase::_read(Buffer *b) const
{
	switch(rep)
	{
		case repVoid:
			break;

		// primitive types
		case repByte:		*(mcopbyte *)data = b->readByte();
			break;

		case repInt:		*(int *)data  = b->readLong();
			break;

		case repLong:		*(long *)data = b->readLong();
			break;

		case repFloat:		*(float *)data = b->readFloat();
			break;

		case repDouble:		*(double *)data = b->readFloat();
			break;

		case repString:		b->readString(*(string *)data);
			break;

		// sequences of primitive types
		case repByteSeq:	b->readByteSeq(*(vector<mcopbyte> *)data);
			break;

		case repLongSeq:	b->readLongSeq(*(vector<long> *)data);
			break;

		case repFloatSeq:	b->readFloatSeq(*(vector<float> *)data);
			break;

		case repStringSeq:	b->readStringSeq(*(vector<string> *)data);
			break;

		default:			assert(false);
	}
}
