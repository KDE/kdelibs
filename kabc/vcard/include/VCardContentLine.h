/*
	libvcard - vCard parsing library for vCard version 3.0
	
	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
