// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
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

#include "kjs_binding.h"
#include "kjs_dom.h"

#include <dom_exception.h>
#include <dom2_events.h>
#include <dom2_range.h>
#include <qvariant.h>
#include <kdebug.h>

using namespace KJS;

/* TODO:
 * The catch all (...) clauses below shouldn't be necessary.
 * But they helped to view for example www.faz.net in an stable manner.
 * Those unknown exceptions should be treated as severe bugs and be fixed.
 *
 * these may be CSS exceptions - need to check - pmk
 */

Value DOMObject::get(ExecState *exec, const UString &p) const
{
  Value result;
  try {
    result = tryGet(exec,p);
  }
  catch (DOM::DOMException e) {
    // ### translate code into readable string ?
    // ### oh, and s/QString/i18n or I18N_NOOP (the code in kjs uses I18N_NOOP... but where is it translated ?)
    //     and where does it appear to the user ?
    Object err = Error::create(exec, GeneralError, QString("DOM exception %1").arg(e.code).local8Bit());
    exec->setException( err );
    result = Undefined();
  }
  catch (...) {
    kdError(6070) << "Unknown exception in DOMObject::get()" << endl;
    result = String("Unknown exception");
  }

  return result;
}

void DOMObject::put(ExecState *exec, const UString &propertyName,
                    const Value &value, int attr)
{
  try {
    tryPut(exec, propertyName, value, attr);
  }
  catch (DOM::DOMException e) {
    Object err = Error::create(exec, GeneralError, QString("DOM exception %1").arg(e.code).local8Bit());
    exec->setException(err);
  }
  catch (...) {
    kdError(6070) << "Unknown exception in DOMObject::put()" << endl;
  }
}

String DOMObject::toString(ExecState *) const
{
  return String("[object " + getClass() + "]");
}

const ClassInfo DOMObjectProto::info = { "DOMObjectProto", 0, 0, 0 };

Value DOMFunction::get(ExecState *exec, const UString &propertyName) const
{
  Value result;
  try {
    result = tryGet(exec, propertyName);
  }
  catch (DOM::DOMException e) {
    result = Undefined();
    Object err = Error::create(exec, GeneralError, QString("DOM exception %1").arg(e.code).local8Bit());
    exec->setException(err);
  }
  catch (...) {
    kdError(6070) << "Unknown exception in DOMFunction::get()" << endl;
    result = String("Unknown exception");
  }

  return result;
}

Value DOMFunction::call(ExecState *exec, Object &thisObj, const List &args)
{
  Value val;
  try {
    val = tryCall(exec, thisObj, args);
  }
  // pity there's no way to distinguish between these in JS code
  catch (DOM::DOMException e) {
    Object err = Error::create(exec, GeneralError, QString("DOM Exception %1").arg(e.code).local8Bit());
    err.put(exec, "code", Number(e.code));
    exec->setException(err);
  }
  catch (DOM::RangeException e) {
    Object err = Error::create(exec, GeneralError, QString("DOM Range Exception %1").arg(e.code).local8Bit());
    err.put(exec, "code", Number(e.code));
    exec->setException(err);
  }
  catch (DOM::CSSException e) {
    Object err = Error::create(exec, GeneralError, QString("CSS Exception %1").arg(e.code).local8Bit());
    err.put(exec, "code", Number(e.code));
    exec->setException(err);
  }
  catch (DOM::EventException e) {
    Object err = Error::create(exec, GeneralError, QString("DOM Event Exception %1").arg(e.code).local8Bit());
    err.put(exec, "code", Number(e.code));
    exec->setException(err);
  }
  catch (...) {
    kdError(6070) << "Unknown exception in DOMFunction::call()" << endl;
    Object err = Error::create(exec, GeneralError, "Unknown exception");
    exec->setException(err);
  }
  return val;
}

UString::UString(const QString &d)
{
  unsigned int len = d.length();
  UChar *dat = new UChar[len];
  memcpy(dat, d.unicode(), len * sizeof(UChar));
  rep = UString::Rep::create(dat, len);
}

UString::UString(const DOM::DOMString &d)
{
  if (d.isNull()) {
    attach(&Rep::null);
    return;
  }

  unsigned int len = d.length();
  UChar *dat = new UChar[len];
  memcpy(dat, d.unicode(), len * sizeof(UChar));
  rep = UString::Rep::create(dat, len);
}

DOM::DOMString UString::string() const
{
  return DOM::DOMString((QChar*) data(), size());
}

QString UString::qstring() const
{
  return QString((QChar*) data(), size());
}

QConstString UString::qconststring() const
{
  return QConstString((QChar*) data(), size());
}

DOM::Node KJS::toNode(const Value& val)
{
  Object obj = Object::dynamicCast(val);
  if (obj.isNull() || !obj.inherits(&DOMNode::info))
    return DOM::Node();

  const DOMNode *dobj = static_cast<const DOMNode*>(obj.imp());
  return dobj->toNode();
}

Value KJS::getString(DOM::DOMString s)
{
  if (s.isNull())
    return Null();
  else
    return String(s);
}

bool KJS::originCheck(const KURL &kurl1, const KURL &kurl2)
{
  if ( kurl1.protocol().find( "javascript", 0, false ) == 0 ||
       kurl2.protocol().find( "javascript", 0, false ) == 0 )
    return true;
  if ( kurl1.isEmpty() || kurl2.isEmpty() )
    return true;

  if (kurl1.protocol() == kurl2.protocol() &&
      kurl1.host() == kurl2.host() &&
      //kurl1.port() == kurl2.port() && // commented out, to fix www.live365.com (uses ports 80 and 89)
      kurl1.user() == kurl2.user() &&
      kurl1.pass() == kurl2.pass())
    return true;
  else
  {
    kdDebug(6070) << "KJS::originCheck DENIED! " << kurl1.url() << " <-> " << kurl2.url() << endl;
    return false;
  }
}

QVariant KJS::ValueToVariant(ExecState* exec, const Value &val) {
  QVariant res;
  switch (val.type()) {
  case BooleanType:
    res = QVariant(val.toBoolean(exec).value(), 0);
    break;
  case NumberType:
    res = QVariant(val.toNumber(exec).value());
    break;
  case StringType:
    res = QVariant(val.toString(exec).value().qstring());
    break;
  default:
    // everything else will be 'invalid'
    break;
  }
  return res;
}
