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

#ifndef  NVALUE_H
#define  NVALUE_H

#include <VCardValue.h>

namespace VCARD
{

class KVCARD_EXPORT NValue : public Value
{
#include "NValue-generated.h"
	NValue *clone();

	Q3CString family()	{ parse(); return family_;	}
	Q3CString given()	{ parse(); return given_;	}
	Q3CString middle()	{ parse(); return middle_;	}
	Q3CString prefix()	{ parse(); return prefix_;	}
	Q3CString suffix()	{ parse(); return suffix_;	}

	void setFamily	(const Q3CString & s) { family_	= s; assembled_ = false; }
	void setGiven	(const Q3CString & s) { given_	= s; assembled_ = false; }
	void setMiddle	(const Q3CString & s) { middle_	= s; assembled_ = false; }
	void setPrefix	(const Q3CString & s) { prefix_	= s; assembled_ = false; }
	void setSuffix	(const Q3CString & s) { suffix_	= s; assembled_ = false; }
	
	private:
	
	Q3CString family_, given_, middle_, prefix_, suffix_;
};

}

#endif
