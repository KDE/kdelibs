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
#include "core.h"
#include "startupmanager.h"
#include "debug.h"
#include <assert.h>

using namespace Arts;
using namespace std;

/*
 * private utility class to deal with any types
 */
namespace Arts {
	static class AnyRefHelper {
	private:
		Arts::InterfaceRepoV2 interfaceRepo;

	public:
		AnyRefHelper() {
			interfaceRepo = DynamicCast(Dispatcher::the()->interfaceRepo());
		}
		void skipType(Buffer& buffer, const string& type);
	} *anyRefHelper = 0;

	static class AnyRefHelperStartup : public StartupClass {
		void startup()	{
			assert(anyRefHelper == 0);
			anyRefHelper = new AnyRefHelper();
		}
		void shutdown()	{
			delete anyRefHelper;
			anyRefHelper = 0;
		}
	}	The_AnyRefHelperStartup;
};

string AnyRefBase::type() const
{
/*
   keep common types here, so that the string constructor doesn't need
   to get called each and every time
 */
	static const string tVoid("void");
	static const string tByte("byte");
	static const string tsByte("*byte");
	static const string tLong("long");
	static const string tsLong("*long");
	static const string tFloat("float");
	static const string tsFloat("*float");
	static const string tString("string");
	static const string tsString("*string");
	static const string tBool("bool");
	static const string tsBool("*bool");

	switch(rep)
	{
		case repVoid:		return tVoid;

		// primitive types
		case repByte:		return tByte;
		case repInt:
		case repLong:		return tLong;
		case repFloat:
		case repDouble:		return tFloat;
		case repString:
		case repCString:	return tString;
		case repBool:		return tBool;

		// sequences of primitive types
		case repByteSeq:	return tsByte;
		case repLongSeq:	return tsLong;
		case repFloatSeq:	return tsFloat;
		case repStringSeq:	return tsString;
		case repBoolSeq:	return tsBool;

		case repAny:		return ((Any *)data)->type;
	}
	assert(false);
	return tVoid; // silence compiler
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

		case repBool:		b->writeBool(*(bool *)data);
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

		case repBoolSeq:	b->writeBoolSeq(*(vector<bool> *)data);
			break;

		case repAny:		b->write(((Any *)data)->value);
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

		case repBool:		*(bool *)data = b->readBool();
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

		case repBoolSeq:	b->readBoolSeq(*(vector<bool> *)data);
			break;

		case repAny:	
			{
				// find out the size by skipping over it for the first time
				long startPos = b->size() - b->remaining();
				anyRefHelper->skipType(*b, ((Any *)data)->type);

				// if everything went well, read the raw value in one step
				long size = (b->size() - b->remaining()) - startPos;
				if(!b->readError())
				{
					b->rewind();
					b->skip(startPos);
					b->read(((Any *)data)->value, size);
				}
			}
			break;

		default:			assert(false);
	}
}

/**
 * correct skipping of an arbitary type not known at compile-time (this is
 * a problem, since the size of the type will vary, due to the sequence<...>s
 * contained)
 */
void AnyRefHelper::skipType(Buffer& buffer, const string& type)
{
	/* sequences */
	if(type[0] == '*')
	{
		long seqlen = buffer.readLong();
		while(seqlen > 0 && !buffer.readError())
		{
			skipType(buffer, type.c_str()+1);
			seqlen--;
		}
	}
	else
	{
		TypeIdentification ti = interfaceRepo.identifyType(type);
		switch(ti)
		{
			case tiString:
				{
					string s;
					buffer.readString(s);
				}
				break;

			case tiLong:
				buffer.readLong();
				break;

			case tiFloat:
				buffer.readFloat();
				break;

			case tiByte:
				buffer.readByte();
				break;

			case tiBoolean:
				buffer.readBool();
				break;

			case tiVoid:
				/* nothing to do */
				break;

			case tiType:
				{
					Arts::TypeDef td = interfaceRepo.queryType(type);

					if(td.name == type)
					{
						vector<TypeComponent>::iterator tci;
						for(tci = td.contents.begin(); tci != td.contents.end();
																		tci++)
						{
							skipType(buffer,tci->type);
						}
					}
					else
					{
						arts_warning("unknown type %s",type.c_str());
					}
				}
				break;

			case tiEnum:
				buffer.readLong();
				break;

			case tiInterface:
				{
					ObjectReference oref;
					oref.readType(buffer);
				}
				break;

			default:
				arts_warning("AnyRefHelper: can't read %s",type.c_str());
				break;
		}
	}
}
