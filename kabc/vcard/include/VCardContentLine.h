/*
	libvcard - vCard parsing library for vCard version 3.0
	
	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef  CONTENTLINE_H
#define  CONTENTLINE_H

#include <qptrlist.h>

#include "VCardEnum.h"
#include "VCardEntity.h"
#include "VCardParam.h"
#include "VCardValue.h"

namespace VCARD
{

class ContentLine : public Entity
{

#include "ContentLine-generated.h"
	
	QCString	group()		{ parse(); return group_;	}
	QCString	name()		{ parse(); return name_;	}
	Value	*	value()		{ parse(); return value_;	}
	ParamList	paramList()	{ parse(); return paramList_;	}
	ParamType	paramType()	{ parse(); return paramType_;	}
	ValueType	valueType()	{ parse(); return valueType_;	}
	EntityType	entityType()	{ parse(); return entityType_;	}
	
	void setGroup		(const QCString & s)
	{ group_ = s; assembled_ = false; }
	
	void setName		(const QCString & s)
	{ name_ = s; assembled_ = false; }
	
	void setValue		(Value *s)
	{ value_ = s; assembled_ = false; }
	
	void setParamList	(const ParamList & l)
	{ paramList_ = l; assembled_ = false; }

	void clear		();
	
	private:
		
		QCString		group_;
		QCString		name_;
		QPtrList<Param>		paramList_;
		Value			* value_;
		
		ParamType		paramType_;
		ValueType		valueType_;
		EntityType		entityType_;
};
}

#endif
