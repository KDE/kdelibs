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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <kdebug.h>

#include <qmap.h>
#include <qdatastream.h>
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
				replyType = type;
			else if(type == "string")
			{
				replyType = "QCString";
				
				QDataStream reply(replyData, IO_WriteOnly);
				reply << "fooo!";
			}
			else if(type == "long")
			{
				replyType = type;
				
				QDataStream reply(replyData, IO_WriteOnly);
				reply << -1;
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
