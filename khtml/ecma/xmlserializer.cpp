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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "xmlserializer.h"
#include "xmlserializer.lut.h"

#include "dom/dom_exception.h"
#include "dom/dom_doc.h"
#include "xml/dom_docimpl.h"

#include <kdebug.h>

using namespace KJS;

////////////////////// XMLSerializer Object ////////////////////////

/* Source for XMLSerializerProtoTable.
@begin XMLSerializerProtoTable 1
  serializeToString XMLSerializer::SerializeToString DontDelete|Function 1
@end
*/
DEFINE_PROTOTYPE("XMLSerializer",XMLSerializerProto)
IMPLEMENT_PROTOFUNC_DOM(XMLSerializerProtoFunc)
IMPLEMENT_PROTOTYPE(XMLSerializerProto,XMLSerializerProtoFunc)

namespace KJS {

XMLSerializerConstructorImp::XMLSerializerConstructorImp(ExecState *)
    : ObjectImp()
{
}

bool XMLSerializerConstructorImp::implementsConstruct() const
{
  return true;
}

Object XMLSerializerConstructorImp::construct(ExecState *exec, const List &)
{
  return Object(new XMLSerializer(exec));
}

const ClassInfo XMLSerializer::info = { "XMLSerializer", 0, 0, 0 };

XMLSerializer::XMLSerializer(ExecState *exec)
  : DOMObject(XMLSerializerProto::self(exec))
{
}

Value XMLSerializerProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  if (!thisObj.inherits(&XMLSerializer::info)) {
    Object err = Error::create(exec,TypeError);
    exec->setException(err);
    return err;
  }

  switch (id) {
  case XMLSerializer::SerializeToString:
    {
      if (args.size() != 1) {
	return Undefined();
      }

      if (!args[0].toObject(exec).inherits(&DOMDocument::info)) {
	return Undefined();
      }

      DOM::Node docNode = static_cast<KJS::DOMDocument *>(args[0].toObject(exec).imp())->toNode();
      DOM::DocumentImpl *doc = static_cast<DOM::DocumentImpl *>(docNode.handle());

      if (!doc) {
	return Undefined();
      }

      QString body;

      try {
	  body = doc->toString().string();
      } catch(DOM::DOMException& e) {
	  Object err = Error::create(exec, GeneralError, "Exception serializing document");
	  exec->setException(err);
	  return err;
      }

      return getString(body);
    }
  }

  return Undefined();
}

} // end namespace
