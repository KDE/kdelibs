// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
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
 *
 *  $Id$
 */

#include "kjs_window.h"
#include "kjs_events.h"
#include "kjs_views.h"
#include "kjs_proxy.h"
#include "kjs_debugwin.h"
#include <qptrdict.h>
#include <qptrlist.h>
#include <kdebug.h>
#include <xml/dom_nodeimpl.h>

using namespace KJS;

QPtrDict<DOMEvent> events;

// -------------------------------------------------------------------------

JSEventListener::JSEventListener(Object _listener, const Object &_win, bool _html)
{
    listener = _listener;
    html = _html;
    win = _win;
    static_cast<Window*>(win.imp())->jsEventListeners.append(this);
}

JSEventListener::~JSEventListener()
{
    static_cast<Window*>(win.imp())->jsEventListeners.removeRef(this);
}

void JSEventListener::handleEvent(DOM::Event &evt)
{
#ifdef KJS_DEBUGGER
  if (KJSDebugWin::instance() && KJSDebugWin::instance()->inSession())
    return;
#endif
  KHTMLPart *part = static_cast<Window*>(win.imp())->part();
  if (part && listener.implementsCall()) {

    KJS::Interpreter *interpreter = KJSProxy::proxy( part )->interpreter();
    ExecState *exec = interpreter->globalExec();

    List args;
    args.append(getDOMEvent(evt));

    // Add the event's target element to the scope
    // (and the document, and the form - see KJS::HTMLElement::eventHandlerScope)
    Object thisObj = Object::dynamicCast(getDOMNode(evt.currentTarget()));
    List scope;
    List oldScope = listener.scope();
    //if (thisVal.type() != NullType)
    if ( !thisObj.isNull() ) {
      scope = static_cast<DOMNode*>(thisObj.imp())->eventHandlerScope();
      if ( !scope.isEmpty() ) {
        List curScope = oldScope.copy();
        curScope.prependList( scope );
        listener.setScope( curScope );
      }
    }

    Window *window = static_cast<Window*>(win.imp());
    // Set the event we're handling in the Window object
    window->setCurrentEvent( &evt );

    Value retval = listener.call(exec, thisObj, args);
    QVariant ret = ValueToVariant(exec, retval);
    if ( !scope.isEmpty() ) {
      listener.setScope( oldScope );
    }
    window->setCurrentEvent( 0 );
    if (ret.type() == QVariant::Bool && ret.toBool() == false)
        evt.preventDefault();
  }
}

DOM::DOMString JSEventListener::eventListenerType()
{
    if (html)
	return "_khtml_HTMLEventListener";
    else
	return "_khtml_JSEventListener";
}

Value KJS::getNodeEventListener(DOM::Node n, int eventId)
{
    DOM::EventListener *listener = n.handle()->getHTMLEventListener(eventId);
    if (listener)
	return static_cast<JSEventListener*>(listener)->listenerObj();
    else
	return Null();
}

// -------------------------------------------------------------------------

const ClassInfo EventPrototype::info = { "EventPrototype", 0, 0, 0 };
// ### make this protype of Event objects?

Value EventPrototype::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "CAPTURING_PHASE")
    return Number((unsigned int)DOM::Event::CAPTURING_PHASE);
  else if (p == "AT_TARGET")
    return Number((unsigned int)DOM::Event::AT_TARGET);
  else if (p == "BUBBLING_PHASE")
    return Number((unsigned int)DOM::Event::BUBBLING_PHASE);

  return DOMObject::tryGet(exec,p);
}

Value KJS::getEventPrototype(ExecState *exec)
{
  Value proto = exec->interpreter()->globalObject().get(exec, "[[event.prototype]]");
  if (!proto.isNull())
    return proto;
  else
  {
    Object eventProto( new EventPrototype );
    exec->interpreter()->globalObject().put(exec, "[[event.prototype]]", eventProto);
    return eventProto;
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMEvent::info = { "Event", 0, 0, 0 };


DOMEvent::~DOMEvent()
{
  events.remove(event.handle());
}


Value DOMEvent::tryGet(ExecState *exec, const UString &p) const
{
#ifdef KJS_VERBOSE
  kdDebug() << "KJS::DOMEvent::tryGet " << p.qstring() << endl;
#endif

  if (p == "type")
    return String(event.type());
  else if (p == "target")
    return getDOMNode(event.target());
  else if (p == "currentTarget" || p == "toElement" /*MSIE extension*/)
    return getDOMNode(event.currentTarget());
  else if (p == "eventPhase")
    return Number((unsigned int)event.eventPhase());
  else if (p == "bubbles")
    return Boolean(event.bubbles());
  else if (p == "cancelable")
    return Boolean(event.cancelable());
  else if (p == "timeStamp")
    return Number((long unsigned int)event.timeStamp()); // ### long long ?
  else if (p == "stopPropagation")
    return new DOMEventFunc(event,DOMEventFunc::StopPropagation);
  else if (p == "preventDefault")
    return new DOMEventFunc(event,DOMEventFunc::PreventDefault);
  else if (p == "initEvent")
    return new DOMEventFunc(event,DOMEventFunc::InitEvent);
  else
    return DOMObject::tryGet(exec,p);
}

Value DOMEventFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  switch (id) {
    case StopPropagation:
      event.stopPropagation();
      result = Undefined();
      break;
    case PreventDefault:
      event.preventDefault();
      result = Undefined();
      break;
    case InitEvent:
      event.initEvent(args[0].toString(exec).value().string(),args[1].toBoolean(exec).value(),args[2].toBoolean(exec).value());
      result = Undefined();
      break;
  };

  return result;
}

Value KJS::getDOMEvent(DOM::Event e)
{
  DOMEvent *ret;
  if (e.isNull())
    return Null();
  else if ((ret = events[e.handle()]))
    return ret;

  DOM::DOMString module = e.eventModuleName();
  if (module == "UIEvents")
    ret = new DOMUIEvent(static_cast<DOM::UIEvent>(e));
  else if (module == "MouseEvents")
    ret = new DOMMouseEvent(static_cast<DOM::MouseEvent>(e));
  else if (module == "MutationEvents")
    ret = new DOMMutationEvent(static_cast<DOM::MutationEvent>(e));
  else
    ret = new DOMEvent(e);

  events.insert(e.handle(),ret);
  return ret;
}

DOM::Event KJS::toEvent(const Value& val)
{
  Object obj = Object::dynamicCast(val);
  if (obj.isNull() || !obj.inherits(&DOMEvent::info))
    return DOM::Event();

  const DOMEvent *dobj = static_cast<const DOMEvent*>(obj.imp());
  return dobj->toEvent();
}

// -------------------------------------------------------------------------


const ClassInfo EventExceptionPrototype::info = { "EventExceptionPrototype", 0, 0, 0 };
// ### make this protype of EventException objects?

Value EventExceptionPrototype::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "UNSPECIFIED_EVENT_TYPE_ERR")
    return Number((unsigned int)DOM::EventException::UNSPECIFIED_EVENT_TYPE_ERR);

  return DOMObject::tryGet(exec, p);
}

Value KJS::getEventExceptionPrototype(ExecState *exec)
{
  Value proto = exec->interpreter()->globalObject().get(exec, "[[eventException.prototype]]");
  if (!proto.isNull())
    return proto;
  else
  {
    Object eventExceptionProto( new EventExceptionPrototype );
    exec->interpreter()->globalObject().put(exec, "[[eventException.prototype]]", eventExceptionProto);
    return eventExceptionProto;
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMUIEvent::info = { "UIEvent", &DOMEvent::info, 0, 0 };


DOMUIEvent::~DOMUIEvent()
{
}


Value DOMUIEvent::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "view")
    return getDOMAbstractView(static_cast<DOM::UIEvent>(event).view());
  else if (p == "detail")
    return Number(static_cast<DOM::UIEvent>(event).detail());
  else if (p == "initUIEvent")
    return new DOMUIEventFunc(static_cast<DOM::UIEvent>(event),DOMUIEventFunc::InitUIEvent);
  else
    return DOMEvent::tryGet(exec, p);
}

Value DOMUIEventFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  switch (id) {
    case InitUIEvent: {
      DOM::AbstractView v = toAbstractView(args[3]);
      static_cast<DOM::UIEvent>(uiEvent).initUIEvent(args[0].toString(exec).value().string(),
                                                     args[1].toBoolean(exec).value(),
                                                     args[2].toBoolean(exec).value(),
                                                     v,
                                                     args[4].toNumber(exec).intValue());
      }
      result = Undefined();
      break;
  };

  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMMouseEvent::info = { "MouseEvent", &DOMUIEvent::info, 0, 0 };


DOMMouseEvent::~DOMMouseEvent()
{
}


Value DOMMouseEvent::tryGet(ExecState *exec, const UString &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMMouseEvent::get " << p.qstring() << endl;
#endif
  if (p == "screenX")
    return Number(static_cast<DOM::MouseEvent>(event).screenX());
  else if (p == "screenY")
    return Number(static_cast<DOM::MouseEvent>(event).screenY());
  else if (p == "clientX" || p == "x")
    return Number(static_cast<DOM::MouseEvent>(event).clientX());
  else if (p == "clientY" || p == "y")
    return Number(static_cast<DOM::MouseEvent>(event).clientY());
  else if (p == "ctrlKey")
    return Boolean(static_cast<DOM::MouseEvent>(event).ctrlKey());
  else if (p == "shiftKey")
    return Boolean(static_cast<DOM::MouseEvent>(event).shiftKey());
  else if (p == "altKey")
    return Boolean(static_cast<DOM::MouseEvent>(event).altKey());
  else if (p == "metaKey")
    return Boolean(static_cast<DOM::MouseEvent>(event).metaKey());
  else if (p == "button")
  {
    // Tricky. The DOM (and khtml) use 0 for LMB, 1 for MMB and 2 for RMB
    // but MSIE uses 1=LMB, 2=RMB, 4=MMB, as a bitfield
    int domButton = static_cast<DOM::MouseEvent>(event).button();
    int button = domButton==0 ? 1 : domButton==1 ? 4 : domButton==2 ? 2 : 0;
    return Number( (unsigned int)button );
  }
  else if (p == "relatedTarget" || p == "fromElement" /*MSIE extension*/)
    return getDOMNode(static_cast<DOM::MouseEvent>(event).relatedTarget());
  else if (p == "initMouseEvent")
    return new DOMMouseEventFunc(static_cast<DOM::MouseEvent>(event),DOMMouseEventFunc::InitMouseEvent);
  else
    return DOMUIEvent::tryGet(exec,p);
}

Value DOMMouseEventFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  switch (id) {
    case InitMouseEvent:
      mouseEvent.initMouseEvent(args[0].toString(exec).value().string(), // typeArg
                                args[1].toBoolean(exec).value(), // canBubbleArg
                                args[2].toBoolean(exec).value(), // cancelableArg
                                toAbstractView(args[3]), // viewArg
                                args[4].toNumber(exec).intValue(), // detailArg
                                args[5].toNumber(exec).intValue(), // screenXArg
                                args[6].toNumber(exec).intValue(), // screenYArg
                                args[7].toNumber(exec).intValue(), // clientXArg
                                args[8].toNumber(exec).intValue(), // clientYArg
                                args[9].toBoolean(exec).value(), // ctrlKeyArg
                                args[10].toBoolean(exec).value(), // altKeyArg
                                args[11].toBoolean(exec).value(), // shiftKeyArg
                                args[12].toBoolean(exec).value(), // metaKeyArg
                                args[13].toNumber(exec).intValue(), // buttonArg
                                toNode(args[14])); // relatedTargetArg
      result = Undefined();
      break;
  };

  return result;
}

// -------------------------------------------------------------------------

const ClassInfo MutationEventPrototype::info = { "MutationEventPrototype", 0, 0, 0 };
// ### make this protype of MutationEvent objects?
// ### should the prototype of this be EventPrototype?

Value MutationEventPrototype::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "MODIFICATION")
    return Number((unsigned int)DOM::MutationEvent::MODIFICATION);
  else if (p == "ADDITION")
    return Number((unsigned int)DOM::MutationEvent::ADDITION);
  else if (p == "REMOVAL")
    return Number((unsigned int)DOM::MutationEvent::REMOVAL);

  return DOMObject::tryGet(exec,p);
}

Value KJS::getMutationEventPrototype(ExecState *exec)
{
  Value proto = exec->interpreter()->globalObject().get(exec, "[[mutationEvent.prototype]]");
  if (!proto.isNull())
    return proto;
  else
  {
    Object mutationEventProto( new MutationEventPrototype );
    exec->interpreter()->globalObject().put(exec, "[[mutationEvent.prototype]]", mutationEventProto);
    return mutationEventProto;
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMMutationEvent::info = { "MutationEvent", &DOMEvent::info, 0, 0 };


DOMMutationEvent::~DOMMutationEvent()
{
}


Value DOMMutationEvent::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "relatedNode")
    return getDOMNode(static_cast<DOM::MutationEvent>(event).relatedNode());
  else if (p == "prevValue")
    return String(static_cast<DOM::MutationEvent>(event).prevValue());
  else if (p == "newValue")
    return String(static_cast<DOM::MutationEvent>(event).newValue());
  else if (p == "attrName")
    return String(static_cast<DOM::MutationEvent>(event).attrName());
  else if (p == "attrChange")
    return Number((unsigned int)static_cast<DOM::MutationEvent>(event).attrChange());
  else if (p == "initMutationEvent")
    return new DOMMutationEventFunc(static_cast<DOM::MutationEvent>(event),DOMMutationEventFunc::InitMutationEvent);
  else
    return DOMEvent::tryGet(exec,p);
}

Value DOMMutationEventFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  switch (id) {
    case InitMutationEvent:
      mutationEvent.initMutationEvent(args[0].toString(exec).value().string(), // typeArg,
                                      args[1].toBoolean(exec).value(), // canBubbleArg
                                      args[2].toBoolean(exec).value(), // cancelableArg
                                      toNode(args[3]), // relatedNodeArg
                                      args[4].toString(exec).value().string(), // prevValueArg
                                      args[5].toString(exec).value().string(), // newValueArg
                                      args[6].toString(exec).value().string(), // attrNameArg
                                      args[7].toNumber(exec).intValue()); // attrChangeArg
      result = Undefined();
      break;
  };

  return result;
}
