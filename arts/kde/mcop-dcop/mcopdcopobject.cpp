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

#define protected public
#include <object.h>
#undef protected
#include <soundserver.h>

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

Arts::Buffer *MCOPDCOPObject::callFunction(MCOPEntryInfo *entry, QCString ifaceName, const QByteArray &data)
{
	long methodID = -1;
	long requestID;
	Arts::Buffer *request, *result;
	
	Arts::Object workingObject = Arts::SubClass(std::string(ifaceName));
	Arts::InterfaceRepo ifaceRepo = Arts::Dispatcher::the()->interfaceRepo();
	Arts::InterfaceDef ifaceDef = ifaceRepo.queryInterface(string(ifaceName));

	vector<Arts::MethodDef> ifaceMethods = ifaceDef.methods;

	vector<Arts::MethodDef>::iterator ifaceMethodsIterator;
	for(ifaceMethodsIterator = ifaceMethods.begin(); ifaceMethodsIterator != ifaceMethods.end(); ifaceMethodsIterator++)
	{
		Arts::MethodDef currentMethod = *ifaceMethodsIterator;

		if(currentMethod.name.c_str() == entry->functionName())
		{		
			
			methodID = workingObject._lookupMethod(currentMethod);
			break;
		}
	}

	if(methodID == -1)
		return 0;
	
	Arts::Object_base *obj = workingObject._get_pool()->base;
	
	request = Arts::Dispatcher::the()->createRequest(requestID, obj->_getObjectID(), methodID);

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
			{
				long arg;
			    argStream >> arg;

				request->writeLong(arg);
			}
			else if(param == "string")
			{
				QCString arg;
				argStream >> arg;

				request->writeString(std::string(arg));
			}
		}
	}

	request->patchLength();
	
	(obj->_getConnection())->qSendBuffer(request);

	result = Arts::Dispatcher::the()->waitForResult(requestID, obj->_getConnection());
	if(!result)
		return 0;

	return result;
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
