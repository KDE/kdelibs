// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Apple Computer, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "domparser.h"
#include "domparser.lut.h"

#include "kjs_dom.h"
#include "kjs_window.h"
#include "xml/dom_nodeimpl.h"
#include "xml/dom_docimpl.h"

#include "html/html_documentimpl.h"

using DOM::DocumentImpl;

////////////////////// DOMParser Object ////////////////////////

/* Source for DOMParserProtoTable.
@begin DOMParserProtoTable 1
  parseFromString DOMParser::ParseFromString DontDelete|Function 2
@end
*/

using namespace KJS;

DEFINE_PROTOTYPE("DOMParser",DOMParserProto)
IMPLEMENT_PROTOFUNC_DOM(DOMParserProtoFunc)
IMPLEMENT_PROTOTYPE(DOMParserProto,DOMParserProtoFunc)

namespace KJS {

DOMParserConstructorImp::DOMParserConstructorImp(ExecState *, DOM::DocumentImpl *d)
    : doc(d)
{
}

bool DOMParserConstructorImp::implementsConstruct() const
{
  return true;
}

Object DOMParserConstructorImp::construct(ExecState *exec, const List &)
{
  return Object(new DOMParser(exec, doc.get()));
}

const ClassInfo DOMParser::info = { "DOMParser", 0, 0 /* &DOMParserTable*/, 0 };


DOMParser::DOMParser(ExecState *exec, DOM::DocumentImpl *d)
  : DOMObject(DOMParserProto::self(exec)), doc(d)
{
//   setPrototype(DOMParserProto::self(exec));
}


Value DOMParserProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  if (!thisObj.inherits(&DOMParser::info)) {
    Object err = Error::create(exec,TypeError);
    exec->setException(err);
    return err;
  }

  DOMParser *parser = static_cast<DOMParser *>(thisObj.imp());

  switch (id) {
  case DOMParser::ParseFromString:
    {
      if (args.size() != 2) {
				return Undefined();
      }

      QString str = args[0].toString(exec).qstring();
      QString contentType = args[1].toString(exec).qstring().trimmed();

      if (contentType == "text/xml" || contentType == "application/xml" || contentType == "application/xhtml+xml") {
        DocumentImpl *docImpl = parser->doc->implementation()->createDocument();

        docImpl->open();
        docImpl->write(str);
        docImpl->finishParsing();
        docImpl->close();

        return getDOMNode(exec, docImpl);
      }
    }
  }

  return Undefined();
}

} // end namespace


