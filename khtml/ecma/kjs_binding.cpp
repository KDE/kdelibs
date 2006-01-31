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

#include "dom/css_stylesheet.h"
#include "dom/dom_exception.h"
#include "dom/dom2_range.h"
#include "xml/dom2_eventsimpl.h"
#include "khtmlpart_p.h"

#include <kdebug.h>
#include <kparts/browserextension.h>
#include <QList>

#include <assert.h>
#include <stdlib.h>

namespace KJS {

UString DOMObject::toString(ExecState *) const
{
  return "[object " + className() + "]";
}

typedef QList<ScriptInterpreter*> InterpreterList;
static InterpreterList *interpreterList;

ScriptInterpreter::ScriptInterpreter( ObjectImp *global, khtml::ChildFrame* frame )
  : Interpreter( global ), m_frame( frame ), 
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

  for (int i = 0; i < interpreterList->size(); ++i)
    interpreterList->at(i)->deleteDOMObject( objectHandle );
}

void ScriptInterpreter::mark()
{
  Interpreter::mark();
#ifdef KJS_VERBOSE
  kdDebug(6070) << "ScriptInterpreter::mark " << this << " marking " << m_domObjects.size() << " DOM objects" << endl;
#endif
  HashMap<void*, DOMObject*>::iterator it = m_domObjects.begin();
  while (it != m_domObjects.end()) {
    DOMObject* obj = it->second;
    if (obj->shouldMark())
        obj->mark();
    ++it;
  }
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
  UChar *dat = static_cast<UChar*>(fastMalloc(sizeof(UChar)*len));
  memcpy(dat, d.unicode(), len * sizeof(UChar));
  m_rep = UString::Rep::create(dat, len);
}

UString::UString(const DOM::DOMString &d)
{
  if (d.isNull()) {
    // we do a conversion here as null DOMStrings shouldn't cross
    // the boundary to kjs. They should either be empty strings
    // or explicitly converted to KJS::Null via getString().
    m_rep = &Rep::empty;
    return;
  }

  unsigned int len = d.length();
  UChar *dat = static_cast<UChar*>(fastMalloc(sizeof(UChar)*len));
  memcpy(dat, d.unicode(), len * sizeof(UChar));
  m_rep = UString::Rep::create(dat, len);
}

DOM::DOMString UString::domString() const
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

DOM::DOMString Identifier::domString() const
{
  return DOM::DOMString((QChar*) data(), size());
}

QString Identifier::qstring() const
{
  return QString((QChar*) data(), size());
}

DOM::NodeImpl* toNode(ValueImp *val)
{
  ObjectImp *obj = val->getObject();
  if (!obj || !obj->inherits(&DOMNode::info))
    return 0;

  const DOMNode *dobj = static_cast<const DOMNode*>(obj);
  return dobj->impl();
}

ValueImp* getStringOrNull(DOM::DOMString s)
{
  if (s.isNull())
    return Null();
  else
    return String(s);
}

QVariant ValueToVariant(ExecState* exec, ValueImp *val) {
  QVariant res;
  switch (val->type()) {
  case BooleanType:
    res = QVariant(val->toBoolean(exec), 0);
    break;
  case NumberType:
    res = QVariant(val->toNumber(exec));
    break;
  case StringType:
    res = QVariant(val->toString(exec).qstring());
    break;
  default:
    // everything else will be 'invalid'
    break;
  }
  return res;
}

void setDOMException(ExecState *exec, int DOMExceptionCode)
{
  //### CHECKME: Was this i18n'd in t
  if (DOMExceptionCode == 0 || exec->hadException())
    return;

  const char *type = "DOM";
  int code = DOMExceptionCode;

  if (code >= DOM::RangeException::_EXCEPTION_OFFSET && code <= DOM::RangeException::_EXCEPTION_MAX) {
    type = "DOM Range";
    code -= DOM::RangeException::_EXCEPTION_OFFSET;
  } else if (code >= DOM::CSSException::_EXCEPTION_OFFSET && code <= DOM::CSSException::_EXCEPTION_MAX) {
    type = "CSS";
    code -= DOM::CSSException::_EXCEPTION_OFFSET;
  } else if (code >= DOM::EventException::_EXCEPTION_OFFSET && code <= DOM::EventException::_EXCEPTION_MAX) {
    type = "DOM Events";
    code -= DOM::EventException::_EXCEPTION_OFFSET;
  }
  char buffer[100]; // needs to fit 20 characters, plus an integer in ASCII, plus a null character
  snprintf(buffer, 99, "%s Exception %d", type, code);

  ObjectImp *errorObject = throwError(exec, GeneralError, buffer);
  errorObject->put(exec, "code", Number(code));
}


class EmbedLiveConnect : public ObjectImp
{
  friend ValueImp* KJS::getLiveConnectValue(KParts::LiveConnectExtension *lc, const QString & name, const int type, const QString & value, int id);
  EmbedLiveConnect(KParts::LiveConnectExtension *lc, UString n, KParts::LiveConnectExtension::Type t, int id);
public:
  ~EmbedLiveConnect();

  virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
  virtual void put(ExecState * exec, const Identifier &prop, ValueImp *value, int=None);

  virtual ValueImp *callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args);

  virtual bool implementsCall() const;
  virtual bool toBoolean(ExecState *) const;
  virtual ValueImp *toPrimitive(ExecState *exec, Type) const;
  virtual UString toString(ExecState *) const;

private:
  EmbedLiveConnect(const EmbedLiveConnect &);
  QPointer<KParts::LiveConnectExtension> m_liveconnect;
  UString name;
  KParts::LiveConnectExtension::Type objtype;
  unsigned long objid;
};

ValueImp *getLiveConnectValue(KParts::LiveConnectExtension *lc, const QString & name, const int type, const QString & value, int id)
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
      return new EmbedLiveConnect(lc, name, t, id);
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

#warning "LiveConnect stuff is broken"
KDE_NO_EXPORT
bool EmbedLiveConnect::getOwnPropertySlot(ExecState *, const Identifier& prop, PropertySlot& slot)
{
  if (m_liveconnect) {
    KParts::LiveConnectExtension::Type rettype;
    QString retval;
    unsigned long retobjid;
    if (m_liveconnect->get(objid, prop.qstring(), rettype, retobjid, retval)) {
      ValueImp* val = getLiveConnectValue(m_liveconnect, prop.qstring(), rettype, retval, retobjid);
      return true;
    }
  }
  return false;
}

KDE_NO_EXPORT
void EmbedLiveConnect::put(ExecState * exec, const Identifier &prop, ValueImp* value, int)
{
  if (m_liveconnect)
    m_liveconnect->put(objid, prop.qstring(), value->toString(exec).qstring());
}

KDE_NO_EXPORT
bool EmbedLiveConnect::implementsCall() const {
  return objtype == KParts::LiveConnectExtension::TypeFunction;
}

KDE_NO_EXPORT
ValueImp* EmbedLiveConnect::callAsFunction(ExecState *exec, ObjectImp*, const List &args)
{
  if (m_liveconnect) {
    QStringList qargs;
    for (ListIterator i = args.begin(); i != args.end(); ++i)
      qargs.append((*i)->toString(exec).qstring());
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
ValueImp *EmbedLiveConnect::toPrimitive(ExecState *exec, Type) const {
  return String(toString(exec));
}

KDE_NO_EXPORT
UString EmbedLiveConnect::toString(ExecState *) const {
  QString str;
  const char *type = objtype == KParts::LiveConnectExtension::TypeFunction ? "Function" : "Object";
  str.sprintf("[object %s ref=%d]", type, (int) objid);
  return UString(str);
}

}
