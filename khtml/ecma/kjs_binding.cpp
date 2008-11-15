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

#include <config-khtml.h>
#if defined(HAVE_VALGRIND_MEMCHECK_H)

#include <valgrind/memcheck.h>
#define VALGRIND_SUPPORT

#endif


#include "kjs_dom.h"
#include "kjs_range.h"

#include <dom/css_stylesheet.h>
#include <dom/dom_exception.h>
#include <dom/dom2_range.h>
#include <xml/dom2_eventsimpl.h>
#include <khtmlpart_p.h>

#include <kdebug.h>
#include <kparts/browserextension.h>
#include <kmessagebox.h>

#include "debugger/debugwindow.h"

#include <QtCore/QList>

#include <assert.h>
#include <stdlib.h>

using namespace KJSDebugger;

namespace KJS {

UString DOMObject::toString(ExecState *) const
{
  return "[object " + className() + "]";
}

HashMap<void*, DOMObject*>* ScriptInterpreter::s_allDomObjects;

typedef QList<ScriptInterpreter*> InterpreterList;
static InterpreterList *interpreterList;

ScriptInterpreter::ScriptInterpreter( JSGlobalObject *global, khtml::ChildFrame* frame )
  : Interpreter( global ), m_frame( frame ),
    m_evt( 0L ), m_inlineCode(false), m_timerCallback(false)
{
#ifdef KJS_VERBOSE
  kDebug(6070) << "ScriptInterpreter::ScriptInterpreter " << this << " for part=" << m_frame;
#endif
  if ( !interpreterList )
    interpreterList = new InterpreterList;
  interpreterList->append( this );
}

ScriptInterpreter::~ScriptInterpreter()
{
#ifdef KJS_VERBOSE
  kDebug(6070) << "ScriptInterpreter::~ScriptInterpreter " << this << " for part=" << m_frame;
#endif
  assert( interpreterList && interpreterList->contains( this ) );
  interpreterList->removeAll( this );
  if ( interpreterList->isEmpty() ) {
    delete interpreterList;
    interpreterList = 0;
  }
}

void ScriptInterpreter::forgetDOMObject( void* objectHandle )
{
  if( !interpreterList ) return;

  for (int i = 0; i < interpreterList->size(); ++i)
    interpreterList->at(i)->m_domObjects.remove( objectHandle );
  allDomObjects()->remove( objectHandle );
}

void ScriptInterpreter::mark(bool isMain)
{
  Interpreter::mark(isMain);
#ifdef KJS_VERBOSE
  kDebug(6070) << "ScriptInterpreter::mark " << this << " marking " << m_domObjects.size() << " DOM objects";
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
    kDebug(6070) << "Window.open, smart policy: id=" << id << " eventOk=" << eventOk;
    if (eventOk)
      return true;
  } else // no event
  {
    if ( m_inlineCode && !m_timerCallback )
    {
      // This is the <a href="javascript:window.open('...')> case -> we let it through
      return true;
      kDebug(6070) << "Window.open, smart policy, no event, inline code -> ok";
    }
    else // This is the <script>window.open(...)</script> case or a timer callback -> block it
      kDebug(6070) << "Window.open, smart policy, no event, <script> tag -> refused";
  }
  return false;
}

bool ScriptInterpreter::s_disableCPUGuard = false;

void ScriptInterpreter::startCPUGuard()
{
  if (s_disableCPUGuard) return;

  unsigned time = 5000;
#ifdef VALGRIND_SUPPORT
  if (RUNNING_ON_VALGRIND)
    time *= 50;
#endif

  setTimeoutTime(time);
  startTimeoutCheck();
}

void ScriptInterpreter::stopCPUGuard()
{
   if (s_disableCPUGuard) return;
   stopTimeoutCheck();
}


bool ScriptInterpreter::shouldInterruptScript() const
{
#ifdef KJS_DEBUGGER
  if (DebugWindow::isBlocked())
    return false;
#endif

    kDebug(6070) << "alarmhandler";
  return KMessageBox::warningYesNo(0L, i18n("A script on this page is causing KHTML to freeze. If it continues to run, other applications may become less responsive.\nDo you want to abort the script?"), i18n("JavaScript"), KGuiItem(i18n("&Abort")), KStandardGuiItem::cont(), "kjscupguard_alarmhandler") == KMessageBox::Yes;
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

DOM::DOMString Identifier::domString() const
{
  return DOM::DOMString((QChar*) data(), size());
}

QString Identifier::qstring() const
{
  return QString((QChar*) data(), size());
}

JSValue* valueGetterAdapter(ExecState* exec, JSObject*, const Identifier& , const PropertySlot& slot)
{
  return static_cast<JSValue*>(slot.customValue());
}

DOM::NodeImpl* toNode(JSValue *val)
{
  JSObject *obj = val->getObject();
  if (!obj || !obj->inherits(&DOMNode::info))
    return 0;

  const DOMNode *dobj = static_cast<const DOMNode*>(obj);
  return dobj->impl();
}

JSValue* getStringOrNull(DOM::DOMString s)
{
  if (s.isNull())
    return jsNull();
  else
    return jsString(s);
}

DOM::DOMString valueToStringWithNullCheck(ExecState* exec, JSValue* val)
{
    if (val->isNull())
      return DOM::DOMString();
    return val->toString(exec).domString();
}

QVariant ValueToVariant(ExecState* exec, JSValue *val) {
  QVariant res;
  switch (val->type()) {
  case BooleanType:
    res = QVariant(val->toBoolean(exec));
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

static const char* const exceptionNames[] = {
    0,
    "INDEX_SIZE_ERR",
    "DOMSTRING_SIZE_ERR",
    "HIERARCHY_REQUEST_ERR",
    "WRONG_DOCUMENT_ERR",
    "INVALID_CHARACTER_ERR",
    "NO_DATA_ALLOWED_ERR",
    "NO_MODIFICATION_ALLOWED_ERR",
    "NOT_FOUND_ERR",
    "NOT_SUPPORTED_ERR",
    "INUSE_ATTRIBUTE_ERR",
    "INVALID_STATE_ERR",
    "SYNTAX_ERR",
    "INVALID_MODIFICATION_ERR",
    "NAMESPACE_ERR",
    "INVALID_ACCESS_ERR",
    "VALIDATION_ERR",
    "TYPE_MISMATCH_ERR",
};

static const char* const rangeExceptionNames[] = {
    0, "BAD_BOUNDARYPOINTS_ERR", "INVALID_NODE_TYPE_ERR"
};

static const char* const cssExceptionNames[] = {
    "SYNTAX_ERR", "INVALID_MODIFICATION_ERR"
};

static const char* const eventExceptionNames[] = {
    "UNSPECIFIED_EVENT_TYPE_ERR"
};

void setDOMException(ExecState *exec, int DOMExceptionCode)
{
  if (DOMExceptionCode == 0 || exec->hadException())
    return;

  const char *type = "DOM";
  int code = DOMExceptionCode;

  JSObject *errorObject = 0;
  const char* const* nameTable;
  int nameTableSize;

  // ### clean up after harmonizing exception objects. maybe use a
  // ### single class? Some human readable message would be nice, too.

  if (code >= DOM::RangeException::_EXCEPTION_OFFSET && code <= DOM::RangeException::_EXCEPTION_MAX) {
    type = "DOM Range";
    code -= DOM::RangeException::_EXCEPTION_OFFSET;
    nameTable = rangeExceptionNames;
    nameTableSize = sizeof(rangeExceptionNames) / sizeof(rangeExceptionNames[0]);
    errorObject = new RangeException(exec);
    exec->setException(errorObject);
    errorObject->put(exec, exec->propertyNames().name, jsString(UString(type) + " Exception"));
    errorObject->put(exec, exec->propertyNames().message, jsString(nameTable[code]));
  } else if (code >= DOM::CSSException::_EXCEPTION_OFFSET && code <= DOM::CSSException::_EXCEPTION_MAX) {
    type = "CSS";
    code -= DOM::CSSException::_EXCEPTION_OFFSET;
    nameTable = cssExceptionNames;
    nameTableSize = sizeof(cssExceptionNames) / sizeof(cssExceptionNames[0]);
  } else if (code >= DOM::EventException::_EXCEPTION_OFFSET && code <= DOM::EventException::_EXCEPTION_MAX) {
    type = "DOM Events";
    code -= DOM::EventException::_EXCEPTION_OFFSET;
    nameTable = eventExceptionNames;
    nameTableSize = sizeof(eventExceptionNames) / sizeof(eventExceptionNames[0]);
  } else {
    nameTable = exceptionNames;
    nameTableSize = sizeof(exceptionNames) / sizeof(exceptionNames[0]);
    errorObject = new JSDOMException(exec);
    exec->setException(errorObject);
    errorObject->put(exec, exec->propertyNames().name, jsString(UString(type) + " Exception"));
    errorObject->put(exec, exec->propertyNames().message, jsString(nameTable[code]));
  }

  const char* name = (code >= 0 && code < nameTableSize) ? nameTable[code] : 0;

  // 100 characters is a big enough buffer, because there are:
  //   13 characters in the message
  //   10 characters in the longest type, "DOM Events"
  //   27 characters in the longest name, "NO_MODIFICATION_ALLOWED_ERR"
  //   20 or so digits in the longest integer's ASCII form (even if int is 64-bit)
  //   1 byte for a null character
  // That adds up to about 70 bytes.
  char buffer[100];

  if (name)
    snprintf(buffer, 99, "%s: %s Exception %d", name, type, code);
  else
    snprintf(buffer, 99, "%s Exception %d", type, code);

  if (!errorObject)
    errorObject = throwError(exec, GeneralError, buffer);
  errorObject->put(exec, "code", jsNumber(code));
}


class EmbedLiveConnect : public JSObject
{
  friend JSValue* KJS::getLiveConnectValue(KParts::LiveConnectExtension *lc, const QString & name, const int type, const QString & value, int id);
  EmbedLiveConnect(KParts::LiveConnectExtension *lc, UString n, KParts::LiveConnectExtension::Type t, int id);
public:
  ~EmbedLiveConnect();

  virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
  virtual void put(ExecState * exec, const Identifier &prop, JSValue *value, int=None);

  virtual JSValue *callAsFunction(ExecState *exec, JSObject *thisObj, const List &args);

  virtual bool implementsCall() const;
  virtual bool toBoolean(ExecState *) const;
  virtual JSValue *toPrimitive(ExecState *exec, JSType) const;
  virtual UString toString(ExecState *) const;

private:
  EmbedLiveConnect(const EmbedLiveConnect &);
  QPointer<KParts::LiveConnectExtension> m_liveconnect;
  UString name;
  KParts::LiveConnectExtension::Type objtype;
  unsigned long objid;
};

JSValue *getLiveConnectValue(KParts::LiveConnectExtension *lc, const QString & name, const int type, const QString & value, int id)
{
  KParts::LiveConnectExtension::Type t=(KParts::LiveConnectExtension::Type)type;
  switch(t) {
    case KParts::LiveConnectExtension::TypeBool: {
      bool ok;
      int i = value.toInt(&ok);
      if (ok)
        return jsBoolean(i);
      return jsBoolean(!strcasecmp(value.toLatin1(), "true"));
    }
    case KParts::LiveConnectExtension::TypeObject:
    case KParts::LiveConnectExtension::TypeFunction:
      return new EmbedLiveConnect(lc, name, t, id);
    case KParts::LiveConnectExtension::TypeNumber: {
      bool ok;
      int i = value.toInt(&ok);
      if (ok)
        return jsNumber(i);
      else
        return jsNumber(value.toDouble(&ok));
    }
    case KParts::LiveConnectExtension::TypeString:
      return jsString(value);
    case KParts::LiveConnectExtension::TypeVoid:
    default:
      return jsUndefined();
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
bool EmbedLiveConnect::getOwnPropertySlot(ExecState *, const Identifier& prop, PropertySlot& slot)
{
  if (m_liveconnect) {
    KParts::LiveConnectExtension::Type rettype;
    QString retval;
    unsigned long retobjid;
    if (m_liveconnect->get(objid, prop.qstring(), rettype, retobjid, retval)) {
      return getImmediateValueSlot(this,
                  getLiveConnectValue(m_liveconnect, prop.qstring(), rettype, retval, retobjid), slot);
    }
  }
  return false;
}

KDE_NO_EXPORT
void EmbedLiveConnect::put(ExecState * exec, const Identifier &prop, JSValue* value, int)
{
  if (m_liveconnect)
    m_liveconnect->put(objid, prop.qstring(), value->toString(exec).qstring());
}

KDE_NO_EXPORT
bool EmbedLiveConnect::implementsCall() const {
  return objtype == KParts::LiveConnectExtension::TypeFunction;
}

KDE_NO_EXPORT
JSValue* EmbedLiveConnect::callAsFunction(ExecState *exec, JSObject*, const List &args)
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
  return jsUndefined();
}

KDE_NO_EXPORT
bool EmbedLiveConnect::toBoolean(ExecState *) const {
  return true;
}

KDE_NO_EXPORT
JSValue *EmbedLiveConnect::toPrimitive(ExecState *exec, JSType) const {
  return jsString(toString(exec));
}

KDE_NO_EXPORT
UString EmbedLiveConnect::toString(ExecState *) const {
  QString str;
  const char *type = objtype == KParts::LiveConnectExtension::TypeFunction ? "Function" : "Object";
  str.sprintf("[object %s ref=%d]", type, (int) objid);
  return UString(str);
}

}
