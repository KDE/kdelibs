// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2003 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001-2003 David Faure (faure@kde.org)
 *  Copyright (C) 2003 Apple Computer, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "kjs_binding.h"
#include "kjs_dom.h"

#include "dom/dom_exception.h"
#include "dom/dom2_range.h"
#include "xml/dom2_eventsimpl.h"
#include "khtmlpart_p.h"

#include <kdebug.h>
#include <kparts/browserextension.h>

#include <assert.h>

using namespace KJS;

/* TODO:
 * The catch all (...) clauses below shouldn't be necessary.
 * But they helped to view for example www.faz.net in an stable manner.
 * Those unknown exceptions should be treated as severe bugs and be fixed.
 *
 * these may be CSS exceptions - need to check - pmk
 */

Value DOMObject::get(ExecState *exec, const Identifier &p) const
{
  Value result;
  try {
    result = tryGet(exec,p);
  }
  catch (DOM::DOMException e) {
    // ### translate code into readable string ?
    // ### oh, and s/QString/i18n or I18N_NOOP (the code in kjs uses I18N_NOOP... but where is it translated ?)
    //     and where does it appear to the user ?
    Object err = Error::create(exec, GeneralError, QString("DOM exception %1").arg(e.code).toLocal8Bit());
    exec->setException( err );
    result = Undefined();
  }
  catch (...) {
    kdError(6070) << "Unknown exception in DOMObject::get()" << endl;
    result = String("Unknown exception");
  }

  return result;
}

void DOMObject::put(ExecState *exec, const Identifier &propertyName,
                    const Value &value, int attr)
{
  try {
    tryPut(exec, propertyName, value, attr);
  }
  catch (DOM::DOMException e) {
    Object err = Error::create(exec, GeneralError, QString("DOM exception %1").arg(e.code).toLocal8Bit());
    exec->setException(err);
  }
  catch (...) {
    kdError(6070) << "Unknown exception in DOMObject::put()" << endl;
  }
}

void DOMObject::tryPut(ExecState *exec, const Identifier &propertyName,
                        const Value& value, int attr)
{
    static_cast<ScriptInterpreter*>(exec->dynamicInterpreter())->customizedDOMObject(this);
    ObjectImp::put(exec,propertyName,value,attr);
}

UString DOMObject::toString(ExecState *) const
{
  return "[object " + className() + "]";
}

Value DOMFunction::get(ExecState *exec, const Identifier &propertyName) const
{
  try {
    return tryGet(exec, propertyName);
  }
  catch (DOM::DOMException e) {
    Object err = Error::create(exec, GeneralError, QString("DOM exception %1").arg(e.code).toLocal8Bit());
    exec->setException(err);
    return Undefined();
  }
  catch (...) {
    kdError(6070) << "Unknown exception in DOMFunction::get()" << endl;
    return String("Unknown exception");
  }
}

Value DOMFunction::call(ExecState *exec, Object &thisObj, const List &args)
{
  try {
    return tryCall(exec, thisObj, args);
  }
  // pity there's no way to distinguish between these in JS code
  // ### Look into setting prototypes of these & the use of instanceof so the exception
  // type can be determined. See what other browsers do.
  catch (DOM::DOMException e) {
    Object err = Error::create(exec, GeneralError, QString("DOM Exception %1").arg(e.code).toLocal8Bit());
    err.put(exec, "code", Number(e.code));
    exec->setException(err);
    return Undefined();
  }
  catch (DOM::RangeException e) {
    Object err = Error::create(exec, GeneralError, QString("DOM Range Exception %1").arg(e.code).toLocal8Bit());
    err.put(exec, "code", Number(e.code));
    exec->setException(err);
    return Undefined();
  }
  catch (DOM::CSSException e) {
    Object err = Error::create(exec, GeneralError, QString("CSS Exception %1").arg(e.code).toLocal8Bit());
    err.put(exec, "code", Number(e.code));
    exec->setException(err);
    return Undefined();
  }
  catch (DOM::EventException e) {
    Object err = Error::create(exec, GeneralError, QString("DOM Event Exception %1").arg(e.code).toLocal8Bit());
    err.put(exec, "code", Number(e.code));
    exec->setException(err);
    return Undefined();
  }
  catch (...) {
    kdError(6070) << "Unknown exception in DOMFunction::call()" << endl;
    Object err = Error::create(exec, GeneralError, "Unknown exception");
    exec->setException(err);
    return Undefined();
  }
}

typedef Q3PtrList<ScriptInterpreter> InterpreterList;
static InterpreterList *interpreterList;

ScriptInterpreter::ScriptInterpreter( const Object &global, khtml::ChildFrame* frame )
  : Interpreter( global ), m_frame( frame ), m_domObjects(1021),
    m_evt( 0L ), m_inlineCode(false), m_timerCallback(false)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "ScriptInterpreter::ScriptInterpreter " << this << " for part=" << m_frame << endl;
#endif
  if ( !interpreterList )
    interpreterList = new InterpreterList;
  interpreterList->append( this );
}

ScriptInterpreter::~ScriptInterpreter()
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "ScriptInterpreter::~ScriptInterpreter " << this << " for part=" << m_frame << endl;
#endif
  assert( interpreterList && interpreterList->contains( this ) );
  interpreterList->remove( this );
  if ( interpreterList->isEmpty() ) {
    delete interpreterList;
    interpreterList = 0;
  }
}

void ScriptInterpreter::forgetDOMObject( void* objectHandle )
{
  if( !interpreterList ) return;

  Q3PtrListIterator<ScriptInterpreter> it( *interpreterList );
  while ( it.current() ) {
    (*it)->deleteDOMObject( objectHandle );
    ++it;
  }
}

void ScriptInterpreter::mark()
{
  Interpreter::mark();
#ifdef KJS_VERBOSE
  kdDebug(6070) << "ScriptInterpreter::mark " << this << " marking " << m_customizedDomObjects.count() << " DOM objects" << endl;
#endif
  Q3PtrDictIterator<void> it( m_customizedDomObjects );
  for( ; it.current(); ++it )
    static_cast<DOMObject*>(it.currentKey())->mark();
}

KParts::ReadOnlyPart* ScriptInterpreter::part() const {
    return m_frame->m_part;
}

bool ScriptInterpreter::isWindowOpenAllowed() const
{
  if ( m_evt )
  {
    int id = m_evt->handle()->id();
    bool eventOk = ( // mouse events
      id == DOM::EventImpl::CLICK_EVENT ||
      id == DOM::EventImpl::MOUSEUP_EVENT || id == DOM::EventImpl::MOUSEDOWN_EVENT ||
      id == DOM::EventImpl::KHTML_ECMA_CLICK_EVENT || id == DOM::EventImpl::KHTML_ECMA_DBLCLICK_EVENT ||
      // keyboard events
      id == DOM::EventImpl::KEYDOWN_EVENT || id == DOM::EventImpl::KEYPRESS_EVENT ||
      id == DOM::EventImpl::KEYUP_EVENT ||
      // other accepted events
      id == DOM::EventImpl::SELECT_EVENT || id == DOM::EventImpl::CHANGE_EVENT ||
      id == DOM::EventImpl::SUBMIT_EVENT );
    kdDebug(6070) << "Window.open, smart policy: id=" << id << " eventOk=" << eventOk << endl;
    if (eventOk)
      return true;
  } else // no event
  {
    if ( m_inlineCode && !m_timerCallback )
    {
      // This is the <a href="javascript:window.open('...')> case -> we let it through
      return true;
      kdDebug(6070) << "Window.open, smart policy, no event, inline code -> ok" << endl;
    }
    else // This is the <script>window.open(...)</script> case or a timer callback -> block it
      kdDebug(6070) << "Window.open, smart policy, no event, <script> tag -> refused" << endl;
  }
  return false;
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
    // we do a conversion here as null DOMStrings shouldn't cross
    // the boundary to kjs. They should either be empty strings
    // or explicitly converted to KJS::Null via getString().
    attach(&Rep::empty);
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

DOM::DOMString Identifier::string() const
{
  return DOM::DOMString((QChar*) data(), size());
}

QString Identifier::qstring() const
{
  return QString((QChar*) data(), size());
}

DOM::Node KJS::toNode(const Value& val)
{
  Object obj = Object::dynamicCast(val);
  if (!obj.isValid() || !obj.inherits(&DOMNode::info))
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

QVariant KJS::ValueToVariant(ExecState* exec, const Value &val) {
  QVariant res;
  switch (val.type()) {
  case BooleanType:
    res = QVariant(val.toBoolean(exec), 0);
    break;
  case NumberType:
    res = QVariant(val.toNumber(exec));
    break;
  case StringType:
    res = QVariant(val.toString(exec).qstring());
    break;
  default:
    // everything else will be 'invalid'
    break;
  }
  return res;
}

class EmbedLiveConnect : public ObjectImp
{
  friend Value KJS::getLiveConnectValue(KParts::LiveConnectExtension *lc, const QString & name, const int type, const QString & value, int id);
  EmbedLiveConnect(KParts::LiveConnectExtension *lc, UString n, KParts::LiveConnectExtension::Type t, int id);
public:
  ~EmbedLiveConnect();

  virtual Value get(ExecState *, const Identifier & prop) const;
  virtual void put(ExecState * exec, const Identifier &prop, const Value & value, int=None);
  virtual Value call(ExecState * exec, Object &, const List &args);
  virtual bool implementsCall() const;
  virtual bool toBoolean(ExecState *) const;
  virtual Value toPrimitive(ExecState *exec, Type) const;
  virtual UString toString(ExecState *) const;

private:
  EmbedLiveConnect(const EmbedLiveConnect &);
  QPointer<KParts::LiveConnectExtension> m_liveconnect;
  UString name;
  KParts::LiveConnectExtension::Type objtype;
  unsigned long objid;
};

Value KJS::getLiveConnectValue(KParts::LiveConnectExtension *lc, const QString & name, const int type, const QString & value, int id)
{
  KParts::LiveConnectExtension::Type t=(KParts::LiveConnectExtension::Type)type;
  switch(t) {
    case KParts::LiveConnectExtension::TypeBool: {
      bool ok;
      int i = value.toInt(&ok);
      if (ok)
        return Boolean(i);
      return Boolean(!strcasecmp(value.latin1(), "true"));
    }
    case KParts::LiveConnectExtension::TypeObject:
    case KParts::LiveConnectExtension::TypeFunction:
      return Value(new EmbedLiveConnect(lc, name, t, id));
    case KParts::LiveConnectExtension::TypeNumber: {
      bool ok;
      int i = value.toInt(&ok);
      if (ok)
        return Number(i);
      else
        return Number(value.toDouble(&ok));
    }
    case KParts::LiveConnectExtension::TypeString:
      return String(value);
    case KParts::LiveConnectExtension::TypeVoid:
    default:
      return Undefined();
  }
}

/* only with gcc > 3.4 KDE_NO_EXPORT */
EmbedLiveConnect::EmbedLiveConnect(KParts::LiveConnectExtension *lc, UString n, KParts::LiveConnectExtension::Type t, int id)
  : m_liveconnect (lc), name(n), objtype(t), objid(id)
{}

/* only with gcc > 3.4 KDE_NO_EXPORT */
EmbedLiveConnect::~EmbedLiveConnect() {
  if (m_liveconnect)
    m_liveconnect->unregister(objid);
}

KDE_NO_EXPORT
Value EmbedLiveConnect::get(ExecState *, const Identifier & prop) const
{
  if (m_liveconnect) {
    KParts::LiveConnectExtension::Type rettype;
    QString retval;
    unsigned long retobjid;
    if (m_liveconnect->get(objid, prop.qstring(), rettype, retobjid, retval))
      return getLiveConnectValue(m_liveconnect, prop.qstring(), rettype, retval, retobjid);
  }
  return Undefined();
}

KDE_NO_EXPORT
void EmbedLiveConnect::put(ExecState * exec, const Identifier &prop, const Value & value, int)
{
  if (m_liveconnect)
    m_liveconnect->put(objid, prop.qstring(), value.toString(exec).qstring());
}

KDE_NO_EXPORT
bool EmbedLiveConnect::implementsCall() const {
  return objtype == KParts::LiveConnectExtension::TypeFunction;
}

KDE_NO_EXPORT
Value EmbedLiveConnect::call(ExecState *exec, Object&, const List &args)
{
  if (m_liveconnect) {
    QStringList qargs;
    for (ListIterator i = args.begin(); i != args.end(); ++i)
      qargs.append((*i).toString(exec).qstring());
    KParts::LiveConnectExtension::Type rtype;
    QString rval;
    unsigned long robjid;
    if (m_liveconnect->call(objid, name.qstring(), qargs, rtype, robjid, rval))
      return getLiveConnectValue(m_liveconnect, name.qstring(), rtype, rval, robjid);
  }
  return Undefined();
}

KDE_NO_EXPORT
bool EmbedLiveConnect::toBoolean(ExecState *) const {
  return true;
}

KDE_NO_EXPORT
Value EmbedLiveConnect::toPrimitive(ExecState *exec, Type) const {
  return String(toString(exec));
}

KDE_NO_EXPORT
UString EmbedLiveConnect::toString(ExecState *) const {
  QString str;
  const char *type = objtype == KParts::LiveConnectExtension::TypeFunction ? "Function" : "Object";
  str.sprintf("[object %s ref=%d]", type, (int) objid);
  return UString(str);
}
