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

#ifndef  SOURCEPARAM_H
#define  SOURCEPARAM_H

#include <qcstring.h>

#include <VCardParam.h>

namespace VCARD
{

class SourceParam : public Param
{

#include "SourceParam-generated.h"
	
	enum SourceParamType { TypeUnknown, TypeValue, TypeContext, TypeX };
	
		SourceParamType type()	{ parse(); return type_;}
		QCString par()			{ parse(); return par_; }
		QCString val()			{ parse(); return val_; }
		
		void setType(SourceParamType t) { type_	= t; assembled_ = false; }
		void setPar(const QCString & s) { par_	= s; assembled_ = false; }
		void setVal(const QCString & s) { val_	= s; assembled_ = false; }
	
	private:
		
		SourceParamType type_;
		// May be "VALUE = uri" or "CONTEXT = word" or "x-name = *SAFE-CHAR"
		QCString par_, val_; // Sub-parameter, value
};

}

#endif
