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

#ifndef  EMAILPARAM_H
#define  EMAILPARAM_H

#include <qcstring.h>

#include <VCardParam.h>

namespace VCARD
{

class EmailParam : public Param
{
	
#include "EmailParam-generated.h"
	
	QCString	emailType() { parse(); return emailType_;	}
	bool		pref()		{ parse(); return pref_;		}
	
	void setEmailType(const QCString & s)
	{ emailType_ = s; assembled_ = false; }
	
	void setPref(bool b)
	{ pref_ = b; assembled_ = false; }
	
	private:
	
		QCString	emailType_;
		bool		pref_;
};

}

#endif
