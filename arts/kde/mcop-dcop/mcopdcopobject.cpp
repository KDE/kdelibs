/*
   Copyright (c) 2001 Nikolas Zimmermann <wildfox@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <kdebug.h>

#include <core.h>
//#include <object.h>
//#include <reference.h>
#include <dynamicrequest.h>

#include <qmap.h>
#include <qdatastream.h>

using namespace std;

#include "mcopdcoptools.h"
#include "mcopdcopobject.h"

class MCOPDCOPObjectPrivate
{
public:
	QMap<QCString, MCOPEntryInfo *> dynamicFunctions;
};

MCOPDCOPObject::MCOPDCOPObject(QCString name) : DCOPObject(name)
{
    d = new MCOPDCOPObjectPrivate();
}

MCOPDCOPObject::~MCOPDCOPObject()
{
    delete d;
}

QCStringList MCOPDCOPObject::functionsDynamic()
{
	QCStringList returnList;
	
	QMap<QCString, MCOPEntryInfo *>::iterator it;
	for(it = d->dynamicFunctions.begin(); it != d->dynamicFunctions.end(); ++it)
		returnList.append(it.key());
	
	return returnList;
}

Arts::Buffer *MCOPDCOPObject::callFunction(MCOPEntryInfo *entry, QCString ifaceName, const QByteArray &data)
{
	Arts::Object workingObject = Arts::SubClass(string(ifaceName));
	Arts::DynamicRequest request(workingObject);
	request.method(string(entry->functionName()));

	if(entry->signatureList().size() > 0)
	{
		QCStringList list = entry->signatureList();

		QCStringList::iterator it;
		for(it = list.begin(); it != list.end(); ++it)
		{
			QCString param = *it;

			kdDebug() << "PARAM: " << param << endl;
			
			QDataStream argStream(data, IO_ReadOnly);

			if(param == "long")
				request.param(MCOPDCOPTools::getLong(argStream));
			else if(param == "string")
				request.param(MCOPDCOPTools::getString(argStream));
		}
	}

	Arts::AnyRef result;	
	if(!request.invoke(result))
		return 0;

	Arts::Buffer *newBuffer = new Arts::Buffer();
	result.write(newBuffer);

	return newBuffer;
}

bool MCOPDCOPObject::processDynamic(const QCString &fun, const QByteArray &data, QCString &replyType, QByteArray &replyData)
{
	QMap<QCString, MCOPEntryInfo *>::iterator it;
	for(it = d->dynamicFunctions.begin(); it != d->dynamicFunctions.end(); ++it)
	{
		MCOPEntryInfo *entry = it.data();

		if((entry->functionName() + entry->signature()) == fun)
		{
			QCString type = entry->functionType();

			if(type == "void")
			{
				replyType = type;

				Arts::Buffer *result = callFunction(entry, objId(), data);
				
				if(result != 0)
					delete result;
			}
			else if(type == "string")
			{
				replyType = "QCString";
				
				QDataStream reply(replyData, IO_WriteOnly);
				reply << "fooo!";
			}
			else if(type == "long")
			{
				replyType = type;

				long returnCode = -1;
				
				Arts::Buffer *result = callFunction(entry, objId(), data);
			
				if(result != 0)
				{
					returnCode = result->readLong();
					delete result;
				}
				
				QDataStream reply(replyData, IO_WriteOnly);
				reply << returnCode;
			}
			
			return true;
		}
	}

	return false;
}

void MCOPDCOPObject::addDynamicFunction(QCString value, MCOPEntryInfo *entry)
{
	d->dynamicFunctions.insert(value, entry);
}
