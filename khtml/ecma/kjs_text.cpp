// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
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

#include "kjs_text.h"
#include "kjs_text.lut.h"
#include "kjs_dom.h"

#include <dom_string.h>
#include <dom_exception.h>

#include <kdebug.h>

using namespace KJS;

const ClassInfo DOMCharacterData::info = { "CharacterImp",
					  &DOMNode::info, &DOMCharacterDataTable, 0 };
/*
@begin DOMCharacterDataTable 2
  data		DOMCharacterData::Data		DontDelete
  length	DOMCharacterData::Length	DontDelete|ReadOnly
@end
@begin DOMCharacterDataProtoTable 7
  substringData	DOMCharacterData::SubstringData	DontDelete|Function 2
  appendData	DOMCharacterData::AppendData	DontDelete|Function 1
  insertData	DOMCharacterData::InsertData	DontDelete|Function 2
  deleteData	DOMCharacterData::DeleteData	DontDelete|Function 2
  replaceData	DOMCharacterData::ReplaceData	DontDelete|Function 2
@end
*/
DEFINE_PROTOTYPE("DOMCharacterData",DOMCharacterDataProto)
IMPLEMENT_PROTOFUNC(DOMCharacterDataProtoFunc)
IMPLEMENT_PROTOTYPE(DOMCharacterDataProto,DOMCharacterDataProtoFunc)

DOMCharacterData::DOMCharacterData(ExecState *exec, DOM::CharacterData d)
 : DOMNode(DOMCharacterDataProto::self(exec), d) {}

DOMCharacterData::DOMCharacterData(Object proto, DOM::CharacterData d)
 : DOMNode(proto, d) {}

Value DOMCharacterData::tryGet(ExecState *exec, const UString &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070)<<"DOMCharacterData::tryGet "<<p.string().string()<<endl;
#endif
  return DOMObjectLookupGetValue<DOMCharacterData,DOMNode>(exec,p,&DOMCharacterDataTable,this);
}

Value DOMCharacterData::getValue(ExecState *, int token) const
{
  DOM::CharacterData data = static_cast<DOM::CharacterData>(node);
  switch (token) {
  case Data:
    return String(data.data());
  case Length:
    return Number(data.length());
 default:
   kdWarning() << "Unhandled token in DOMCharacterData::getValue : " << token << endl;
   return Value();
  }
}

void DOMCharacterData::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
  if (propertyName == "data")
    static_cast<DOM::CharacterData>(node).setData(value.toString(exec).string());
  else
    DOMNode::tryPut(exec, propertyName,value,attr);
}

Value DOMCharacterDataProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::CharacterData data = static_cast<DOMCharacterData *>(thisObj.imp())->toData();
  switch(id) {
    case DOMCharacterData::SubstringData:
      return getString(data.substringData(args[0].toInteger(exec),args[1].toInteger(exec)));
    case DOMCharacterData::AppendData:
      data.appendData(args[0].toString(exec).string());
      return Undefined();
      break;
    case DOMCharacterData::InsertData:
      data.insertData(args[0].toInteger(exec),args[1].toString(exec).string());
      return  Undefined();
      break;
    case DOMCharacterData::DeleteData:
      data.deleteData(args[0].toInteger(exec),args[1].toInteger(exec));
      return  Undefined();
      break;
    case DOMCharacterData::ReplaceData:
      data.replaceData(args[0].toInteger(exec),args[1].toInteger(exec),args[2].toString(exec).string());
      return Undefined();
      break;
    default:
      return Undefined();
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMText::info = { "Text",
				 &DOMCharacterData::info, 0, 0 };
/*
@begin DOMTextProtoTable 1
  splitText	DOMText::SplitText	DontDelete|Function 1
@end
*/
DEFINE_PROTOTYPE("DOMText",DOMTextProto)
IMPLEMENT_PROTOFUNC(DOMTextProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMTextProto,DOMTextProtoFunc,DOMCharacterDataProto)

DOMText::DOMText(ExecState *exec, DOM::Text t)
  : DOMCharacterData(DOMTextProto::self(exec), t) { }

Value DOMText::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "")
    return Undefined(); // ### TODO
  else
    return DOMCharacterData::tryGet(exec, p);
}

Value DOMTextProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::Text text = static_cast<DOMText *>(thisObj.imp())->toText();
  switch(id) {
    case DOMText::SplitText:
      return getDOMNode(exec,text.splitText(args[0].toInteger(exec)));
      break;
    default:
      return Undefined();
  }
}

