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

#ifndef  AGENTPARAM_H
#define  AGENTPARAM_H

#include <qcstring.h>

#include <VCardParam.h>
#include <VCardURIValue.h>

namespace VCARD
{

class AgentParam : public Param
{
	
#include "AgentParam-generated.h"
	
	bool refer()
	{ parse(); return refer_; }
	
	URIValue uri()
	{ parse(); return uri_; }
	
	void setRefer(bool b)
	{ refer_ = b; assembled_ = false; }

	void setURI(const QCString & s)
	{ uri_ = s; assembled_ = false; }
	
	private:
		
		bool		refer_;
		URIValue	uri_;
};

}

#endif
