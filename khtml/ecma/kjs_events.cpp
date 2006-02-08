// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
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

#include "kjs_window.h"
#include "kjs_events.h"
#include "kjs_events.lut.h"
#include "kjs_views.h"
#include "kjs_proxy.h"
#include "xml/dom_nodeimpl.h"
#include "xml/dom_docimpl.h"
#include "xml/dom2_eventsimpl.h"
#include "rendering/render_object.h"
#include "rendering/render_canvas.h"
#include "xml/dom2_eventsimpl.h"
#include "khtml_part.h"

#include <kdebug.h>

using namespace KJS;
using namespace DOM;

// -------------------------------------------------------------------------

JSEventListener::JSEventListener(Object _listener, ObjectImp *_compareListenerImp, const Object &_win, bool _html)
  : listener( _listener ), compareListenerImp( _compareListenerImp ), html( _html ), win( _win )
{
    //fprintf(stderr,"JSEventListener::JSEventListener this=%p listener=%p\n",this,listener.imp());
  if (compareListenerImp) {
    static_cast<Window*>(win.imp())->jsEventListeners.insert(compareListenerImp, this);
  }
}

JSEventListener::~JSEventListener()
{
  if (compareListenerImp) {
    static_cast<Window*>(win.imp())->jsEventListeners.remove(compareListenerImp);
  }
  //fprintf(stderr,"JSEventListener::~JSEventListener this=%p listener=%p\n",this,listener.imp());
}

void JSEventListener::handleEvent(DOM::Event &evt)
{
#ifdef KJS_DEBUGGER
  if (KJSDebugWin::debugWindow() && KJSDebugWin::debugWindow()->inSession())
    return;
#endif
  KHTMLPart *part = ::qt_cast<KHTMLPart *>(static_cast<Window*>(win.imp())->part());
  KJSProxy *proxy = 0L;
  if (part)
    proxy = part->jScript();

  if (proxy && listener.isValid() && listener.implementsCall()) {
    ref();

    KJS::ScriptInterpreter *interpreter = static_cast<KJS::ScriptInterpreter *>(proxy->interpreter());
    ExecState *exec = interpreter->globalExec();

    List args;
    args.append(getDOMEvent(exec,evt));

    // Set "this" to the event's current target
    Object thisObj = Object::dynamicCast(getDOMNode(exec,evt.currentTarget()));
    if ( !thisObj.isValid() ) {
      // Window events (window.onload/window.onresize etc.) must have 'this' set to the window.
      // DocumentImpl::defaultEventHandler sets currentTarget to 0 to mean 'window'.
      thisObj = win;
    }

    Window *window = static_cast<Window*>(win.imp());
    // Set the event we're handling in the Window object
    window->setCurrentEvent( &evt );
    // ... and in the interpreter
    interpreter->setCurrentEvent( &evt );

    KJSCPUGuard guard;
    guard.start();
    Value retval = listener.call(exec, thisObj, args);
    guard.stop();

    window->setCurrentEvent( 0 );
    interpreter->setCurrentEvent( 0 );
    if ( exec->hadException() )
      exec->clearException();
    else if (html)
    {
      QVariant ret = ValueToVariant(exec, retval);
      if (ret.type() == QVariant::Bool && ret.toBool() == false)
        evt.preventDefault();
    }
    window->afterScriptExecution();
    deref();
  }
}

DOM::DOMString JSEventListener::eventListenerType()
{
    if (html)
	return "_khtml_HTMLEventListener";
    else
	return "_khtml_JSEventListener";
}

Object JSEventListener::listenerObj() const
{
  return listener;
}

JSLazyEventListener::JSLazyEventListener(const QString &_code, const QString &_name, const Object &_win, DOM::NodeImpl* _originalNode)
  : JSEventListener(Object(), 0, _win, true),
    code(_code), name(_name),
    parsed(false)
{
  // We don't retain the original node, because we assume it
  // will stay alive as long as this handler object is around
  // and we need to avoid a reference cycle. If JS transfers
  // this handler to another node, parseCode will be called and
  // then originalNode is no longer needed.

  originalNode = _originalNode;
}

JSLazyEventListener::~JSLazyEventListener()
{
  if (listener.isValid()) {
    static_cast<Window*>(win.imp())->jsEventListeners.remove(listener.imp());
  }
}

void JSLazyEventListener::handleEvent(DOM::Event &evt)
{
  parseCode();
  if (listener.isValid()) {
    JSEventListener::handleEvent(evt);
  }
}


Object JSLazyEventListener::listenerObj() const
{
  parseCode();
  return listener;
}

void JSLazyEventListener::parseCode() const
{
  if (!parsed) {
    KHTMLPart *part = ::qt_cast<KHTMLPart *>(static_cast<Window*>(win.imp())->part());
    KJSProxy *proxy = 0L;
    if (part)
      proxy = part->jScript();

    if (proxy) {
      KJS::ScriptInterpreter *interpreter = static_cast<KJS::ScriptInterpreter *>(proxy->interpreter());
      ExecState *exec = interpreter->globalExec();

      //KJS::Constructor constr(KJS::Global::current().get("Function").imp());
      KJS::Object constr = interpreter->builtinFunction();
      KJS::List args;

      static KJS::String eventString("event");

      args.append(eventString);
      args.append(KJS::String(code));
      listener = constr.construct(exec, args); // ### is globalExec ok ?

      if (exec->hadException()) {
        exec->clearException();

        // failed to parse, so let's just make this listener a no-op
        listener = Object();
      } else if (!listener.inherits(&DeclaredFunctionImp::info)) {
        listener = Object();// Error creating function
      } else {
        DeclaredFunctionImp *declFunc = static_cast<DeclaredFunctionImp*>(listener.imp());
        declFunc->setName(Identifier(name));

        if (originalNode)
        {
          // Add the event's home element to the scope
          // (and the document, and the form - see KJS::HTMLElement::eventHandlerScope)
          ScopeChain scope = listener.scope();

          Object thisObj = Object::dynamicCast(getDOMNode(exec, originalNode));

          if (thisObj.isValid()) {
            static_cast<DOMNode*>(thisObj.imp())->pushEventHandlerScope(exec, scope);

            listener.setScope(scope);
          }
        }
      }
    }

    // no more need to keep the unparsed code around
    code = QString();

    if (listener.isValid()) {
      static_cast<Window*>(win.imp())->jsEventListeners.insert(listener.imp(),
                                                               (KJS::JSEventListener *)(this));
    }

    parsed = true;
  }
}

// -------------------------------------------------------------------------

const ClassInfo EventConstructor::info = { "EventConstructor", 0, &EventConstructorTable, 0 };
/*
@begin EventConstructorTable 3
  CAPTURING_PHASE	DOM::Event::CAPTURING_PHASE	DontDelete|ReadOnly
  AT_TARGET		DOM::Event::AT_TARGET		DontDelete|ReadOnly
  BUBBLING_PHASE	DOM::Event::BUBBLING_PHASE	DontDelete|ReadOnly
# Reverse-engineered from Netscape
  MOUSEDOWN		1				DontDelete|ReadOnly
  MOUSEUP		2				DontDelete|ReadOnly
  MOUSEOVER		4				DontDelete|ReadOnly
  MOUSEOUT		8				DontDelete|ReadOnly
  MOUSEMOVE		16				DontDelete|ReadOnly
  MOUSEDRAG		32				DontDelete|ReadOnly
  CLICK			64				DontDelete|ReadOnly
  DBLCLICK		128				DontDelete|ReadOnly
  KEYDOWN		256				DontDelete|ReadOnly
  KEYUP			512				DontDelete|ReadOnly
  KEYPRESS		1024				DontDelete|ReadOnly
  DRAGDROP		2048				DontDelete|ReadOnly
  FOCUS			4096				DontDelete|ReadOnly
  BLUR			8192				DontDelete|ReadOnly
  SELECT		16384				DontDelete|ReadOnly
  CHANGE		32768				DontDelete|ReadOnly
@end
*/

EventConstructor::EventConstructor(ExecState *exec)
  : DOMObject(exec->interpreter()->builtinObjectPrototype())
{
}

Value EventConstructor::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<EventConstructor, DOMObject>(exec,p,&EventConstructorTable,this);
}

Value EventConstructor::getValueProperty(ExecState *, int token) const
{
  // We use the token as the value to return directly
  return Number(token);
}

Value KJS::getEventConstructor(ExecState *exec)
{
  return cacheGlobalObject<EventConstructor>(exec, "[[event.constructor]]");
}

// -------------------------------------------------------------------------

const ClassInfo DOMEvent::info = { "Event", 0, &DOMEventTable, 0 };
/*
@begin DOMEventTable 7
  type		DOMEvent::Type		DontDelete|ReadOnly
  target	DOMEvent::Target	DontDelete|ReadOnly
  currentTarget	DOMEvent::CurrentTarget	DontDelete|ReadOnly
  srcElement	DOMEvent::SrcElement	DontDelete|ReadOnly
  eventPhase	DOMEvent::EventPhase	DontDelete|ReadOnly
  bubbles	DOMEvent::Bubbles	DontDelete|ReadOnly
  cancelable	DOMEvent::Cancelable	DontDelete|ReadOnly
  timeStamp	DOMEvent::TimeStamp	DontDelete|ReadOnly
  returnValue   DOMEvent::ReturnValue   DontDelete
  cancelBubble  DOMEvent::CancelBubble  DontDelete
@end
@begin DOMEventProtoTable 3
  stopPropagation 	DOMEvent::StopPropagation	DontDelete|Function 0
  preventDefault 	DOMEvent::PreventDefault	DontDelete|Function 0
  initEvent		DOMEvent::InitEvent		DontDelete|Function 3
@end
*/
DEFINE_PROTOTYPE("DOMEvent", DOMEventProto)
IMPLEMENT_PROTOFUNC_DOM(DOMEventProtoFunc)
IMPLEMENT_PROTOTYPE(DOMEventProto, DOMEventProtoFunc)

DOMEvent::DOMEvent(ExecState *exec, DOM::Event e)
  : DOMObject(DOMEventProto::self(exec)), event(e) { }

DOMEvent::DOMEvent(const Object &proto, DOM::Event e)
  : DOMObject(proto), event(e) { }

DOMEvent::~DOMEvent()
{
  ScriptInterpreter::forgetDOMObject(event.handle());
}

Value DOMEvent::tryGet(ExecState *exec, const Identifier &p) const
{
#ifdef KJS_VERBOSE
  kdDebug() << "KJS::DOMEvent::tryGet " << p.qstring() << endl;
#endif
  return DOMObjectLookupGetValue<DOMEvent,DOMObject>(exec, p, &DOMEventTable, this );
}

Value DOMEvent::getValueProperty(ExecState *exec, int token) const
{
  switch (token) {
  case Type:
    return String(event.type());
  case Target:
  case SrcElement: /*MSIE extension - "the object that fired the event"*/
    return getDOMNode(exec,event.target());
  case CurrentTarget:
    return getDOMNode(exec,event.currentTarget());
  case EventPhase:
    return Number((unsigned int)event.eventPhase());
  case Bubbles:
    return Boolean(event.bubbles());
  case Cancelable:
    return Boolean(event.cancelable());
  case TimeStamp:
    return Number((long unsigned int)event.timeStamp()); // ### long long ?
  case ReturnValue: // MSIE extension
    return Boolean(event.handle()->defaultPrevented());
  case CancelBubble: // MSIE extension
    return Boolean(event.handle()->propagationStopped());
  default:
    kdDebug(6070) << "WARNING: Unhandled token in DOMEvent::getValueProperty : " << token << endl;
    return Value();
  }
}

Value DOMEvent::defaultValue(ExecState *exec, KJS::Type hint) const
{
  if (event.handle()->id() == EventImpl::ERROR_EVENT && !event.handle()->message().isNull()) {
    return String(event.handle()->message());
  }
  else
    return DOMObject::defaultValue(exec,hint);
}

void DOMEvent::tryPut(ExecState *exec, const Identifier &propertyName,
                      const Value& value, int attr)
{
  DOMObjectLookupPut<DOMEvent, DOMObject>(exec, propertyName, value, attr,
                                          &DOMEventTable, this);
}

void DOMEvent::putValueProperty(ExecState *exec, int token, const Value& value, int)
{
  switch (token) {
  case ReturnValue: // MSIE equivalent for "preventDefault" (but with a way to reset it)
    // returnValue=false means "default action of the event on the source object is canceled",
    // which means preventDefault(true). Hence the '!'.
    event.handle()->preventDefault(!value.toBoolean(exec));
    break;
  case CancelBubble: // MSIE equivalent for "stopPropagation" (but with a way to reset it)
    event.handle()->stopPropagation(value.toBoolean(exec));
    break;
  default:
    break;
  }
}

Value DOMEventProtoFunc::tryCall(ExecState *exec, Object & thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMEvent, thisObj );
  DOM::Event event = static_cast<DOMEvent *>( thisObj.imp() )->toEvent();
  switch (id) {
    case DOMEvent::StopPropagation:
      event.stopPropagation();
      return Undefined();
    case DOMEvent::PreventDefault:
      event.preventDefault();
      return Undefined();
    case DOMEvent::InitEvent:
      event.initEvent(args[0].toString(exec).string(),args[1].toBoolean(exec),args[2].toBoolean(exec));
      return Undefined();
  };
  return Undefined();
}

Value KJS::getDOMEvent(ExecState *exec, DOM::Event e)
{
  DOM::EventImpl *ei = e.handle();
  if (!ei)
    return Null();
  ScriptInterpreter* interp = static_cast<ScriptInterpreter *>(exec->interpreter());
  DOMObject *ret = interp->getDOMObject(ei);
  if (!ret) {
    if (ei->isTextInputEvent())
      ret = new DOMTextEvent(exec, e);
    else if (ei->isKeyboardEvent())
      ret = new DOMKeyboardEvent(exec, e);
    else if (ei->isMouseEvent())
      ret = new DOMMouseEvent(exec, e);
    else if (ei->isUIEvent())
      ret = new DOMUIEvent(exec, e);
    else if (ei->isMutationEvent())
      ret = new DOMMutationEvent(exec, e);
    else
      ret = new DOMEvent(exec, e);

    interp->putDOMObject(ei, ret);
  }

  return Value(ret);
}

DOM::Event KJS::toEvent(const Value& val)
{
  Object obj = Object::dynamicCast(val);
  if (!obj.isValid() || !obj.inherits(&DOMEvent::info))
    return DOM::Event();

  const DOMEvent *dobj = static_cast<const DOMEvent*>(obj.imp());
  return dobj->toEvent();
}

// -------------------------------------------------------------------------


const ClassInfo EventExceptionConstructor::info = { "EventExceptionConstructor", 0, &EventExceptionConstructorTable, 0 };
/*
@begin EventExceptionConstructorTable 1
  UNSPECIFIED_EVENT_TYPE_ERR    DOM::EventException::UNSPECIFIED_EVENT_TYPE_ERR DontDelete|ReadOnly
@end
*/
EventExceptionConstructor::EventExceptionConstructor(ExecState *exec)
  : DOMObject(exec->interpreter()->builtinObjectPrototype())
{
}

Value EventExceptionConstructor::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<EventExceptionConstructor, DOMObject>(exec,p,&EventExceptionConstructorTable,this);
}

Value EventExceptionConstructor::getValueProperty(ExecState *, int token) const
{
  // We use the token as the value to return directly
  return Number(token);
}

Value KJS::getEventExceptionConstructor(ExecState *exec)
{
  return cacheGlobalObject<EventExceptionConstructor>(exec, "[[eventException.constructor]]");
}

// -------------------------------------------------------------------------

const ClassInfo DOMUIEvent::info = { "UIEvent", &DOMEvent::info, &DOMUIEventTable, 0 };
/*
@begin DOMUIEventTable 7
  view		DOMUIEvent::View	DontDelete|ReadOnly
  detail	DOMUIEvent::Detail	DontDelete|ReadOnly
  keyCode	DOMUIEvent::KeyCode	DontDelete|ReadOnly
  charCode	DOMUIEvent::CharCode	DontDelete|ReadOnly
  layerX	DOMUIEvent::LayerX	DontDelete|ReadOnly
  layerY	DOMUIEvent::LayerY	DontDelete|ReadOnly
  pageX		DOMUIEvent::PageX	DontDelete|ReadOnly
  pageY		DOMUIEvent::PageY	DontDelete|ReadOnly
  which		DOMUIEvent::Which	DontDelete|ReadOnly
@end
@begin DOMUIEventProtoTable 1
  initUIEvent	DOMUIEvent::InitUIEvent	DontDelete|Function 5
@end
*/
DEFINE_PROTOTYPE("DOMUIEvent",DOMUIEventProto)
IMPLEMENT_PROTOFUNC_DOM(DOMUIEventProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMUIEventProto,DOMUIEventProtoFunc,DOMEventProto)

DOMUIEvent::DOMUIEvent(ExecState *exec, DOM::UIEvent ue) :
  DOMEvent(DOMUIEventProto::self(exec), ue) {}

DOMUIEvent::DOMUIEvent(const Object &proto, DOM::UIEvent ue) :
  DOMEvent(proto, ue) {}

DOMUIEvent::~DOMUIEvent()
{
}

Value DOMUIEvent::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<DOMUIEvent,DOMEvent>(exec,p,&DOMUIEventTable,this);
}

Value DOMUIEvent::getValueProperty(ExecState *exec, int token) const
{
  switch (token) {
  case View:
    return getDOMAbstractView(exec,static_cast<DOM::UIEvent>(event).view());
  case Detail:
    return Number(static_cast<DOM::UIEvent>(event).detail());
  case KeyCode:
    // IE-compatibility
    return Number(static_cast<DOM::UIEvent>(event).keyCode());
  case CharCode:
    // IE-compatibility
    return Number(static_cast<DOM::UIEvent>(event).charCode());
  case LayerX:
    // NS-compatibility
    return Number(static_cast<DOM::UIEvent>(event).layerX());
  case LayerY:
    // NS-compatibility
    return Number(static_cast<DOM::UIEvent>(event).layerY());
  case PageX:
    // NS-compatibility
    return Number(static_cast<DOM::UIEvent>(event).pageX());
  case PageY:
    // NS-compatibility
    return Number(static_cast<DOM::UIEvent>(event).pageY());
  case Which:
    // NS-compatibility
    return Number(static_cast<DOM::UIEvent>(event).which());
  default:
    kdDebug(6070) << "WARNING: Unhandled token in DOMUIEvent::getValueProperty : " << token << endl;
    return Undefined();
  }
}

Value DOMUIEventProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMUIEvent, thisObj );
  DOM::UIEvent uiEvent = static_cast<DOMUIEvent *>(thisObj.imp())->toUIEvent();
  switch (id) {
    case DOMUIEvent::InitUIEvent: {
      DOM::AbstractView v = toAbstractView(args[3]);
      static_cast<DOM::UIEvent>(uiEvent).initUIEvent(args[0].toString(exec).string(),
                                                     args[1].toBoolean(exec),
                                                     args[2].toBoolean(exec),
                                                     v,
                                                     args[4].toInteger(exec));
      }
      return Undefined();
  }
  return Undefined();
}

// -------------------------------------------------------------------------

const ClassInfo DOMMouseEvent::info = { "MouseEvent", &DOMUIEvent::info, &DOMMouseEventTable, 0 };

/*
@begin DOMMouseEventTable 2
  screenX	DOMMouseEvent::ScreenX	DontDelete|ReadOnly
  screenY	DOMMouseEvent::ScreenY	DontDelete|ReadOnly
  clientX	DOMMouseEvent::ClientX	DontDelete|ReadOnly
  x		DOMMouseEvent::X	DontDelete|ReadOnly
  clientY	DOMMouseEvent::ClientY	DontDelete|ReadOnly
  y		DOMMouseEvent::Y	DontDelete|ReadOnly
  offsetX	DOMMouseEvent::OffsetX	DontDelete|ReadOnly
  offsetY	DOMMouseEvent::OffsetY	DontDelete|ReadOnly
  ctrlKey	DOMMouseEvent::CtrlKey	DontDelete|ReadOnly
  shiftKey	DOMMouseEvent::ShiftKey	DontDelete|ReadOnly
  altKey	DOMMouseEvent::AltKey	DontDelete|ReadOnly
  metaKey	DOMMouseEvent::MetaKey	DontDelete|ReadOnly
  button	DOMMouseEvent::Button	DontDelete|ReadOnly
  relatedTarget	DOMMouseEvent::RelatedTarget DontDelete|ReadOnly
  fromElement	DOMMouseEvent::FromElement DontDelete|ReadOnly
  toElement	DOMMouseEvent::ToElement	DontDelete|ReadOnly
@end
@begin DOMMouseEventProtoTable 1
  initMouseEvent	DOMMouseEvent::InitMouseEvent	DontDelete|Function 15
@end
*/
DEFINE_PROTOTYPE("DOMMouseEvent",DOMMouseEventProto)
IMPLEMENT_PROTOFUNC_DOM(DOMMouseEventProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMMouseEventProto,DOMMouseEventProtoFunc,DOMUIEventProto)

DOMMouseEvent::DOMMouseEvent(ExecState *exec, DOM::MouseEvent me) :
  DOMUIEvent(DOMMouseEventProto::self(exec), me) {}

DOMMouseEvent::~DOMMouseEvent()
{
}

Value DOMMouseEvent::tryGet(ExecState *exec, const Identifier &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMMouseEvent::tryGet " << p.qstring() << endl;
#endif
  return DOMObjectLookupGetValue<DOMMouseEvent,DOMUIEvent>(exec,p,&DOMMouseEventTable,this);
}

Value DOMMouseEvent::getValueProperty(ExecState *exec, int token) const
{
  switch (token) {
  case ScreenX:
    return Number(static_cast<DOM::MouseEvent>(event).screenX());
  case ScreenY:
    return Number(static_cast<DOM::MouseEvent>(event).screenY());
  case ClientX:
  case X:
    return Number(static_cast<DOM::MouseEvent>(event).clientX());
  case ClientY:
  case Y:
    return Number(static_cast<DOM::MouseEvent>(event).clientY());
  case OffsetX:
  case OffsetY: // MSIE extension
  {
    DOM::Node node = event.target();
    node.handle()->getDocument()->updateRendering();
    khtml::RenderObject *rend = node.handle() ? node.handle()->renderer() : 0L;
    int x = static_cast<DOM::MouseEvent>(event).clientX();
    int y = static_cast<DOM::MouseEvent>(event).clientY();
    if ( rend ) {
      int xPos, yPos;
      if ( rend->absolutePosition( xPos, yPos ) ) {
        kdDebug() << "DOMMouseEvent::getValueProperty rend=" << rend << "  xPos=" << xPos << "  yPos=" << yPos << endl;
        x -= xPos;
        y -= yPos;
      }
      if ( rend->canvas() ) {
        int cYPos, cXPos;
        rend->canvas()->absolutePosition( cXPos,  cYPos,  true );
        x += cXPos;
        y += cYPos;
      }
    }
    return Number( token == OffsetX ? x : y );
  }
  case CtrlKey:
    return Boolean(static_cast<DOM::MouseEvent>(event).ctrlKey());
  case ShiftKey:
    return Boolean(static_cast<DOM::MouseEvent>(event).shiftKey());
  case AltKey:
    return Boolean(static_cast<DOM::MouseEvent>(event).altKey());
  case MetaKey:
    return Boolean(static_cast<DOM::MouseEvent>(event).metaKey());
  case Button:
  {
    if ( exec->interpreter()->compatMode() == Interpreter::NetscapeCompat ) {
        return Number(static_cast<DOM::MouseEvent>(event).button());
    }
    // Tricky. The DOM (and khtml) use 0 for LMB, 1 for MMB and 2 for RMB
    // but MSIE uses 1=LMB, 2=RMB, 4=MMB, as a bitfield
    int domButton = static_cast<DOM::MouseEvent>(event).button();
    int button = domButton==0 ? 1 : domButton==1 ? 4 : domButton==2 ? 2 : 0;
    return Number( (unsigned int)button );
  }
  case ToElement:
    // MSIE extension - "the object toward which the user is moving the mouse pointer"
    if (event.handle()->id() == DOM::EventImpl::MOUSEOUT_EVENT)
      return getDOMNode(exec,static_cast<DOM::MouseEvent>(event).relatedTarget());
    return getDOMNode(exec,static_cast<DOM::MouseEvent>(event).target());
  case FromElement:
    // MSIE extension - "object from which activation
    // or the mouse pointer is exiting during the event" (huh?)
    if (event.handle()->id() == DOM::EventImpl::MOUSEOUT_EVENT)
      return getDOMNode(exec,static_cast<DOM::MouseEvent>(event).target());
    /* fall through */
  case RelatedTarget:
    return getDOMNode(exec,static_cast<DOM::MouseEvent>(event).relatedTarget());
  default:
    kdDebug(6070) << "WARNING: Unhandled token in DOMMouseEvent::getValueProperty : " << token << endl;
    return Value();
  }
}

Value DOMMouseEventProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMMouseEvent, thisObj );
  DOM::MouseEvent mouseEvent = static_cast<DOMMouseEvent *>(thisObj.imp())->toMouseEvent();
  switch (id) {
    case DOMMouseEvent::InitMouseEvent:
      mouseEvent.initMouseEvent(args[0].toString(exec).string(), // typeArg
                                args[1].toBoolean(exec), // canBubbleArg
                                args[2].toBoolean(exec), // cancelableArg
                                toAbstractView(args[3]), // viewArg
                                args[4].toInteger(exec), // detailArg
                                args[5].toInteger(exec), // screenXArg
                                args[6].toInteger(exec), // screenYArg
                                args[7].toInteger(exec), // clientXArg
                                args[8].toInteger(exec), // clientYArg
                                args[9].toBoolean(exec), // ctrlKeyArg
                                args[10].toBoolean(exec), // altKeyArg
                                args[11].toBoolean(exec), // shiftKeyArg
                                args[12].toBoolean(exec), // metaKeyArg
                                args[13].toInteger(exec), // buttonArg
                                toNode(args[14])); // relatedTargetArg
      return Undefined();
  }
  return Undefined();
}

// -------------------------------------------------------------------------

const ClassInfo DOMKeyEventBase::info = { "KeyEventBase", &DOMUIEvent::info, &DOMKeyEventBaseTable, 0 };

/*
@begin DOMKeyEventBaseTable 5
  keyVal   	 DOMKeyEventBase::Key	      DontDelete|ReadOnly
  virtKeyVal	 DOMKeyEventBase::VirtKey     DontDelete|ReadOnly
  ctrlKey        DOMKeyEventBase::CtrlKey     DontDelete|ReadOnly
  altKey         DOMKeyEventBase::AltKey      DontDelete|ReadOnly
  shiftKey       DOMKeyEventBase::ShiftKey    DontDelete|ReadOnly
  altKey         DOMKeyEventBase::AltKey      DontDelete|ReadOnly
@end
*/

DOMKeyEventBase::DOMKeyEventBase(const Object &proto, DOM::TextEvent ke) :
  DOMUIEvent(proto, ke) {}

DOMKeyEventBase::~DOMKeyEventBase()
{}

Value DOMKeyEventBase::tryGet(ExecState *exec,const Identifier &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMKeyEventBase::tryGet " << p.qstring() << endl;
#endif
  return DOMObjectLookupGetValue<DOMKeyEventBase,DOMUIEvent>(exec,p,&DOMKeyEventBaseTable,this);
}

Value DOMKeyEventBase::getValueProperty(ExecState *, int token) const
{
  DOM::KeyEventBaseImpl* tevent = impl();
  switch (token) {
  case Key:
    return Number(tevent->keyVal());
  case VirtKey:
    return Number(tevent->virtKeyVal());
  // these modifier attributes actually belong into a KeyboardEvent interface,
  // but we want them on "keypress" as well.
  case CtrlKey:
    return Boolean(tevent->ctrlKey());
  case ShiftKey:
    return Boolean(tevent->shiftKey());
  case AltKey:
    return Boolean(tevent->altKey());
  case MetaKey:
    return Boolean(tevent->metaKey());
  default:
    kdDebug(6070) << "WARNING: Unhandled token in DOMTextEvent::getValueProperty : " << token << endl;
    return KJS::Undefined();
  }
}

// -------------------------------------------------------------------------
const ClassInfo DOMTextEvent::info = { "TextEvent", &DOMKeyEventBase::info, &DOMTextEventTable, 0 };

/*
@begin DOMTextEventTable 1
  data           DOMTextEvent::Data          DontDelete|ReadOnly
@end
@begin DOMTextEventProtoTable 1
  initTextEvent	DOMTextEvent::InitTextEvent	DontDelete|Function 5
  # Missing: initTextEventNS
@end
*/
DEFINE_PROTOTYPE("DOMTextEvent",DOMTextEventProto)
IMPLEMENT_PROTOFUNC_DOM(DOMTextEventProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMTextEventProto,DOMTextEventProtoFunc,DOMUIEventProto) //Note: no proto in KeyBase

DOMTextEvent::DOMTextEvent(ExecState *exec, DOM::TextEvent ke) :
  DOMKeyEventBase(DOMTextEventProto::self(exec), ke) {}

DOMTextEvent::~DOMTextEvent()
{
}

Value DOMTextEvent::tryGet(ExecState *exec, const Identifier &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMTextEvent::tryGet " << p.qstring() << endl;
#endif
  return DOMObjectLookupGetValue<DOMTextEvent,DOMKeyEventBase>(exec,p,&DOMTextEventTable,this);
}

Value DOMTextEvent::getValueProperty(ExecState *, int token) const
{
  DOM::TextEventImpl* tevent = impl();
  switch (token) {
  case Data:
    return String(tevent->data());
  default:
    kdDebug(6070) << "WARNING: Unhandled token in DOMTextEvent::getValueProperty : " << token << endl;
    return KJS::Undefined();
  }
}

Value DOMTextEventProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMTextEvent, thisObj );
  DOM::TextEventImpl* keyEvent = static_cast<DOMTextEvent *>(thisObj.imp())->impl();
  switch (id) {
    case DOMTextEvent::InitTextEvent:
      keyEvent->initTextEvent(args[0].toString(exec).string(), // typeArg
                            args[1].toBoolean(exec), // canBubbleArg
                            args[2].toBoolean(exec), // cancelableArg
                            toAbstractView(args[3]), // viewArg
                            args[4].toString(exec).string()); // dataArg

      return Undefined();
  }
  return Undefined();
}
// -------------------------------------------------------------------------
const ClassInfo DOMKeyboardEvent::info = { "KeyboardEvent", &DOMKeyEventBase::info, &DOMKeyboardEventTable, 0 };

/*
@begin DOMKeyboardEventTable 2
  keyIdentifier  DOMKeyboardEvent::KeyIdentifier  DontDelete|ReadOnly
  keyLocation    DOMKeyboardEvent::KeyLocation    DontDelete|ReadOnly
@end
@begin DOMKeyboardEventProtoTable 2
  initKeyboardEvent	DOMKeyboardEvent::InitKeyboardEvent	DontDelete|Function 7
  getModifierState      DOMKeyboardEvent::GetModifierState      DontDelete|Function 1
@end
*/
DEFINE_PROTOTYPE("DOMKeyboardEvent",DOMKeyboardEventProto)
IMPLEMENT_PROTOFUNC_DOM(DOMKeyboardEventProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMKeyboardEventProto,DOMKeyboardEventProtoFunc,DOMUIEventProto) //Note: no proto in KeyBase

DOMKeyboardEvent::DOMKeyboardEvent(ExecState *exec, DOM::TextEvent ke) :
  DOMKeyEventBase(DOMKeyboardEventProto::self(exec), ke) {}

DOMKeyboardEvent::~DOMKeyboardEvent()
{
}

Value DOMKeyboardEvent::tryGet(ExecState *exec, const Identifier &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMKeyboardEvent::tryGet " << p.qstring() << endl;
#endif
  return DOMObjectLookupGetValue<DOMKeyboardEvent,DOMKeyEventBase>(exec,p,&DOMKeyboardEventTable,this);
}

Value DOMKeyboardEvent::getValueProperty(ExecState *, int token) const
{
  DOM::KeyboardEventImpl* tevent = impl();
  switch (token) {
  case KeyIdentifier:
    return String(tevent->keyIdentifier());
  case KeyLocation:
    return Number(tevent->keyLocation());
  default:
    kdDebug(6070) << "WARNING: Unhandled token in DOMKeyboardEvent::getValueProperty : " << token << endl;
    return KJS::Undefined();
  }
}

Value DOMKeyboardEventProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMKeyboardEvent, thisObj );
  DOM::KeyboardEventImpl* keyEvent = static_cast<DOMKeyboardEvent *>(thisObj.imp())->impl();
  switch (id) {
    case DOMKeyboardEvent::InitKeyboardEvent:
      keyEvent->initKeyboardEvent(args[0].toString(exec).string(), // typeArg
                            args[1].toBoolean(exec), // canBubbleArg
                            args[2].toBoolean(exec), // cancelableArg
                            toAbstractView(args[3]), // viewArg
                            args[4].toString(exec).string(), // keyIdentifierArg
                            args[5].toInteger(exec),         // keyLocationArg
                            args[6].toString(exec).string()); //modifiersList
      break;
    case DOMKeyboardEvent::GetModifierState:
      return Boolean(keyEvent->getModifierState(args[0].toString(exec).string()));
  }
  return Undefined();
}

// -------------------------------------------------------------------------
const ClassInfo KeyboardEventConstructor::info = { "KeyboardEventConstructor", 0, &KeyboardEventConstructorTable, 0 };
/*
@begin KeyboardEventConstructorTable 4
  DOM_KEY_LOCATION_STANDARD  DOM::KeyboardEventImpl::DOM_KEY_LOCATION_STANDARD DontDelete|ReadOnly
  DOM_KEY_LOCATION_LEFT      DOM::KeyboardEventImpl::DOM_KEY_LOCATION_LEFT     DontDelete|ReadOnly
  DOM_KEY_LOCATION_RIGHT     DOM::KeyboardEventImpl::DOM_KEY_LOCATION_RIGHT    DontDelete|ReadOnly
  DOM_KEY_LOCATION_NUMPAD    DOM::KeyboardEventImpl::DOM_KEY_LOCATION_NUMPAD   DontDelete|ReadOnly
@end
*/
KeyboardEventConstructor::KeyboardEventConstructor(ExecState* exec)
  : DOMObject(exec->interpreter()->builtinObjectPrototype())
{}

Value KeyboardEventConstructor::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<KeyboardEventConstructor,DOMObject>(exec,p,&KeyboardEventConstructorTable,this);
}

Value KeyboardEventConstructor::getValueProperty(ExecState *, int token) const
{
  // We use the token as the value to return directly
  return Number(token);
}

Value KJS::getKeyboardEventConstructor(ExecState *exec)
{
  return cacheGlobalObject<KeyboardEventConstructor>(exec, "[[keyboardEvent.constructor]]");
}


// -------------------------------------------------------------------------
const ClassInfo MutationEventConstructor::info = { "MutationEventConstructor", 0, &MutationEventConstructorTable, 0 };
/*
@begin MutationEventConstructorTable 3
  MODIFICATION	DOM::MutationEvent::MODIFICATION	DontDelete|ReadOnly
  ADDITION	DOM::MutationEvent::ADDITION		DontDelete|ReadOnly
  REMOVAL	DOM::MutationEvent::REMOVAL		DontDelete|ReadOnly
@end
*/
MutationEventConstructor::MutationEventConstructor(ExecState* exec)
  : DOMObject(exec->interpreter()->builtinObjectPrototype())
{
}

Value MutationEventConstructor::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<MutationEventConstructor,DOMObject>(exec,p,&MutationEventConstructorTable,this);
}

Value MutationEventConstructor::getValueProperty(ExecState *, int token) const
{
  // We use the token as the value to return directly
  return Number(token);
}

Value KJS::getMutationEventConstructor(ExecState *exec)
{
  return cacheGlobalObject<MutationEventConstructor>(exec, "[[mutationEvent.constructor]]");
}

// -------------------------------------------------------------------------

const ClassInfo DOMMutationEvent::info = { "MutationEvent", &DOMEvent::info, &DOMMutationEventTable, 0 };
/*
@begin DOMMutationEventTable 5
  relatedNode	DOMMutationEvent::RelatedNode	DontDelete|ReadOnly
  prevValue	DOMMutationEvent::PrevValue	DontDelete|ReadOnly
  newValue	DOMMutationEvent::NewValue	DontDelete|ReadOnly
  attrName	DOMMutationEvent::AttrName	DontDelete|ReadOnly
  attrChange	DOMMutationEvent::AttrChange	DontDelete|ReadOnly
@end
@begin DOMMutationEventProtoTable 1
  initMutationEvent	DOMMutationEvent::InitMutationEvent	DontDelete|Function 8
@end
*/
DEFINE_PROTOTYPE("DOMMutationEvent",DOMMutationEventProto)
IMPLEMENT_PROTOFUNC_DOM(DOMMutationEventProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMMutationEventProto,DOMMutationEventProtoFunc,DOMEventProto)

DOMMutationEvent::DOMMutationEvent(ExecState *exec, DOM::MutationEvent me) :
  DOMEvent(DOMMutationEventProto::self(exec), me) {}

DOMMutationEvent::~DOMMutationEvent()
{
}

Value DOMMutationEvent::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<DOMMutationEvent,DOMEvent>(exec,p,&DOMMutationEventTable,this);
}

Value DOMMutationEvent::getValueProperty(ExecState *exec, int token) const
{
  switch (token) {
  case RelatedNode:
    return getDOMNode(exec,static_cast<DOM::MutationEvent>(event).relatedNode());
  case PrevValue:
    return String(static_cast<DOM::MutationEvent>(event).prevValue());
  case NewValue:
    return String(static_cast<DOM::MutationEvent>(event).newValue());
  case AttrName:
    return String(static_cast<DOM::MutationEvent>(event).attrName());
  case AttrChange:
    return Number((unsigned int)static_cast<DOM::MutationEvent>(event).attrChange());
  default:
    kdDebug(6070) << "WARNING: Unhandled token in DOMMutationEvent::getValueProperty : " << token << endl;
    return Value();
  }
}

Value DOMMutationEventProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMMutationEvent, thisObj );
  DOM::MutationEvent mutationEvent = static_cast<DOMMutationEvent *>(thisObj.imp())->toMutationEvent();
  switch (id) {
    case DOMMutationEvent::InitMutationEvent:
      mutationEvent.initMutationEvent(args[0].toString(exec).string(), // typeArg,
                                      args[1].toBoolean(exec), // canBubbleArg
                                      args[2].toBoolean(exec), // cancelableArg
                                      toNode(args[3]), // relatedNodeArg
                                      args[4].toString(exec).string(), // prevValueArg
                                      args[5].toString(exec).string(), // newValueArg
                                      args[6].toString(exec).string(), // attrNameArg
                                      args[7].toInteger(exec)); // attrChangeArg
      return Undefined();
  }
  return Undefined();
}
