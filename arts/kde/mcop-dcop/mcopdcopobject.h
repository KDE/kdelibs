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

#ifndef MCOPDCOPOBJECT_H
#define MCOPDCOPOBJECT_H

#include <dcopobject.h>

#include <qvaluelist.h>
#include <qcstring.h>

typedef QValueList<QCString> QCStringList;

class MCOPDCOPObjectPrivate;

class MCOPDCOPObject : public DCOPObject
{
public:
	MCOPDCOPObject(QCString name);
	~MCOPDCOPObject();

	virtual QCStringList functions();

	void addDynamicFunction(QCString value);
	
private:
    MCOPDCOPObjectPrivate *d;
};


#endif

