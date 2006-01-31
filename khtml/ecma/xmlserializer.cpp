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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
namespace KJS {
KJS_DEFINE_PROTOTYPE(XMLSerializerProto)
KJS_IMPLEMENT_PROTOFUNC(XMLSerializerProtoFunc)
KJS_IMPLEMENT_PROTOTYPE("XMLSerializer", XMLSerializerProto,XMLSerializerProtoFunc)

XMLSerializerConstructorImp::XMLSerializerConstructorImp(ExecState *)
    : ObjectImp()
{
}

bool XMLSerializerConstructorImp::implementsConstruct() const
{
  return true;
}

ObjectImp *XMLSerializerConstructorImp::construct(ExecState *exec, const List &)
{
  return new XMLSerializer(exec);
}

const ClassInfo XMLSerializer::info = { "XMLSerializer", 0, 0, 0 };

XMLSerializer::XMLSerializer(ExecState *exec)
{
  setPrototype(XMLSerializerProto::self(exec));
}

ValueImp *XMLSerializerProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( XMLSerializer, thisObj );

  switch (id) {
  case XMLSerializer::SerializeToString:
    {
      if (args.size() != 1) {
	return Undefined();
      }

      if (!args[0]->toObject(exec)->inherits(&DOMDocument::info)) {
	return Undefined();
      }

      DOM::NodeImpl* docNode = static_cast<KJS::DOMDocument *>(args[0]->toObject(exec))->impl();
      DOM::DocumentImpl *doc = static_cast<DOM::DocumentImpl *>(docNode);

      if (!doc) {
	return Undefined();
      }

      QString body;

      try {
	  body = doc->toString().string();
      } catch(DOM::DOMException& e) {
	  ObjectImp *err = Error::create(exec, GeneralError, "Exception serializing document");
	  exec->setException(err);
	  return err;
      }

      return ::getStringOrNull(body);
    }
  }

  return Undefined();
}

} // end namespace
