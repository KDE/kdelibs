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

#ifndef  PARAM_H
#define  PARAM_H

#include <q3cstring.h>
#include <q3ptrlist.h>

#include <VCardEntity.h>

namespace VCARD
{

class KVCARD_EXPORT Param : public Entity
{

#include "Param-generated.h"

        Param(const Q3CString &name, const Q3CString &value);

	void setName(const Q3CString &);
	void setValue(const Q3CString &);
		
	Q3CString name();
	Q3CString value();
	
	private:
		
		Q3CString name_;
		Q3CString value_;
};

typedef Q3PtrList<Param> ParamList;
typedef Q3PtrListIterator<Param> ParamListIterator;

}

#endif
