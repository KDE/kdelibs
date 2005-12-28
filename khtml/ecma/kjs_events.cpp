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

JSEventListener::JSEventListener(ObjectImp *_listener, ObjectImp *_compareListenerImp, ObjectImp *_win, bool _html)
  : listener( _listener ), compareListenerImp( _compareListenerImp ), html( _html ), win( _win )
{
    //fprintf(stderr,"JSEventListener::JSEventListener this=%p listener=%p\n",this,listener.imp());
  if (compareListenerImp) {
    static_cast<Window*>(win.get())->jsEventListeners.insert(compareListenerImp, this);
  }
}

JSEventListener::~JSEventListener()
{
  if (compareListenerImp) {
    static_cast<Window*>(win.get())->jsEventListeners.remove(compareListenerImp);
  }
  //fprintf(stderr,"JSEventListener::~JSEventListener this=%p listener=%p\n",this,listener.imp());
}

void JSEventListener::handleEvent(DOM::Event &evt)
{
#ifdef KJS_DEBUGGER
  if (KJSDebugWin::debugWindow() && KJSDebugWin::debugWindow()->inSession())
    return;
#endif
  KHTMLPart *part = qobject_cast<KHTMLPart*>(static_cast<Window*>(win.get())->part());
  KJSProxy *proxy = 0L;
  if (part)
    proxy = part->jScript();

  if (proxy && listener && listener->implementsCall()) {
    ref();

    KJS::ScriptInterpreter *interpreter = static_cast<KJS::ScriptInterpreter *>(proxy->interpreter());
    ExecState *exec = interpreter->globalExec();

    List args;
    args.append(getDOMEvent(exec,evt.handle()));

    // Set "this" to the event's current target
    ObjectImp *thisObj = getDOMNode(exec,evt.currentTarget().handle())->getObject();
    if ( !thisObj ) {
      // Window events (window.onload/window.onresize etc.) must have 'this' set to the window.
      // DocumentImpl::defaultEventHandler sets currentTarget to 0 to mean 'window'.
      thisObj = win;
    }

    Window *window = static_cast<Window*>(win.get());
    // Set the event we're handling in the Window object
    window->setCurrentEvent( evt.handle() );
    // ... and in the interpreter
    interpreter->setCurrentEvent( &evt );

    KJSCPUGuard guard;
    guard.start();
    ValueImp *retval = listener->call(exec, thisObj, args);
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

ObjectImp *JSEventListener::listenerObj() const
{
  return listener;
}

JSLazyEventListener::JSLazyEventListener(const QString &_code, const QString &_name, ObjectImp *_win, DOM::NodeImpl* _originalNode)
  : JSEventListener(0, 0, _win, true), code(_code), name(_name), parsed(false)
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
  if (listener) {
    static_cast<Window*>(win.get())->jsEventListeners.remove(listener);
  }
}

void JSLazyEventListener::handleEvent(DOM::Event &evt)
{
  parseCode();
  if (listener) {
    JSEventListener::handleEvent(evt);
  }
}


ObjectImp *JSLazyEventListener::listenerObj() const
{
  parseCode();
  return listener;
}

void JSLazyEventListener::parseCode() const
{
  if (!parsed) {
    KHTMLPart *part = qobject_cast<KHTMLPart*>(static_cast<Window*>(win.get())->part());
    KJSProxy *proxy = 0L;
    if (part)
      proxy = part->jScript();

    if (proxy) {
      KJS::ScriptInterpreter *interpreter = static_cast<KJS::ScriptInterpreter *>(proxy->interpreter());
      ExecState *exec = interpreter->globalExec();

      //KJS::Constructor constr(KJS::Global::current().get("Function").imp());
      KJS::ObjectImp *constr = interpreter->builtinFunction();
      KJS::List args;

      static KJS::UString eventString("event");

      args.append(KJS::String(eventString));
      args.append(KJS::String(code));
      listener = constr->construct(exec, args); // ### is globalExec ok ?

      if (exec->hadException()) {
        exec->clearException();

        // failed to parse, so let's just make this listener a no-op
        listener = 0;
      } else if (!listener->inherits(&DeclaredFunctionImp::info)) {
        listener = 0;// Error creating function
      } else {
        DeclaredFunctionImp *declFunc = static_cast<DeclaredFunctionImp*>(listener.get());
	// what happened to this?
        declFunc->setFunctionName(Identifier(UString(name)));

        if (originalNode)
        {
          // Add the event's home element to the scope
          // (and the document, and the form - see KJS::HTMLElement::eventHandlerScope)
          ScopeChain scope = listener->scope();

          ObjectImp *thisObj = getDOMNode(exec, originalNode)->getObject();

          if (thisObj) {
            static_cast<DOMNode*>(thisObj)->pushEventHandlerScope(exec, scope);
            listener->setScope(scope);
          }
        }
      }
    }

    // no more need to keep the unparsed code around
    code = QString();

    if (listener) {
      static_cast<Window*>(win.get())->jsEventListeners.insert(listener,
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
  : DOMObject(exec->lexicalInterpreter()->builtinObjectPrototype())
{
}

bool EventConstructor::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<EventConstructor, DOMObject>(exec,&EventConstructorTable,this,propertyName,slot);
}

ValueImp *EventConstructor::getValueProperty(ExecState *, int token) const
{
  // We use the token as the value to return directly
  return Number(token);
}

ValueImp *KJS::getEventConstructor(ExecState *exec)
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
IMPLEMENT_PROTOFUNC(DOMEventProtoFunc)
IMPLEMENT_PROTOTYPE(DOMEventProto, DOMEventProtoFunc)

DOMEvent::DOMEvent(ExecState *exec, DOM::EventImpl* e)
  : m_impl(e) {
  setPrototype(DOMEventProto::self(exec));
}

DOMEvent::DOMEvent(ObjectImp *proto, DOM::EventImpl* e):
  m_impl(e) {
  setPrototype(proto);
}

DOMEvent::~DOMEvent()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
}


bool DOMEvent::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kdDebug() << "KJS::DOMEvent::getOwnPropertySlot " << p.qstring() << endl;
#endif

  return getStaticValueSlot<DOMEvent, DOMObject>(exec,&DOMEventTable,this,propertyName,slot);
}

ValueImp *DOMEvent::getValueProperty(ExecState *exec, int token) const
{
  DOM::EventImpl& event = *impl();
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
    return Boolean(event.defaultPrevented());
  case CancelBubble: // MSIE extension
    return Boolean(event.propagationStopped());
  default:
    kdDebug(6070) << "WARNING: Unhandled token in DOMEvent::getValueProperty : " << token << endl;
    return 0;
  }
}

ValueImp *DOMEvent::defaultValue(ExecState *exec, KJS::Type hint) const
{
  if (m_impl->id() == EventImpl::ERROR_EVENT && !m_impl->message().isNull()) {
    return String(m_impl->message());
  }
  else
    return DOMObject::defaultValue(exec,hint);
}

void DOMEvent::put(ExecState *exec, const Identifier &propertyName,
                      ValueImp *value, int attr)
{
  lookupPut<DOMEvent, DOMObject>(exec, propertyName, value, attr,
                                          &DOMEventTable, this);
}

void DOMEvent::putValueProperty(ExecState *exec, int token, ValueImp *value, int)
{
  switch (token) {
  case ReturnValue: // MSIE equivalent for "preventDefault" (but with a way to reset it)
    // returnValue=false means "default action of the event on the source object is canceled",
    // which means preventDefault(true). Hence the '!'.
    m_impl->preventDefault(!value->toBoolean(exec));
    break;
  case CancelBubble: // MSIE equivalent for "stopPropagation" (but with a way to reset it)
    m_impl->stopPropagation(value->toBoolean(exec));
    break;
  default:
    break;
  }
}

ValueImp *DOMEventProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMEvent, thisObj );
  DOM::EventImpl& event = *static_cast<DOMEvent *>( thisObj )->impl();
  switch (id) {
    case DOMEvent::StopPropagation:
      event.stopPropagation(true);
      return Undefined();
    case DOMEvent::PreventDefault:
      event.preventDefault(true);
      return Undefined();
    case DOMEvent::InitEvent:
      event.initEvent(args[0]->toString(exec).domString(),args[1]->toBoolean(exec),args[2]->toBoolean(exec));
      return Undefined();
  };
  return Undefined();
}

ValueImp *KJS::getDOMEvent(ExecState *exec, DOM::EventImpl* ei)
{
  if (!ei)
    return Null();
  ScriptInterpreter* interp = static_cast<ScriptInterpreter *>(exec->interpreter());
  DOMObject *ret = interp->getDOMObject(ei);
  if (!ret) {
    if (ei->isTextEvent())
      ret = new DOMTextEvent(exec, static_cast<DOM::TextEventImpl*>(ei));
    else if (ei->isMouseEvent())
      ret = new DOMMouseEvent(exec, static_cast<DOM::MouseEventImpl*>(ei));
    else if (ei->isUIEvent())
      ret = new DOMUIEvent(exec, static_cast<DOM::UIEventImpl*>(ei));
    else if (ei->isMutationEvent())
      ret = new DOMMutationEvent(exec, static_cast<DOM::MutationEventImpl*>(ei));
    else
      ret = new DOMEvent(exec, ei);

    interp->putDOMObject(ei, ret);
  }

  return ret;
}

DOM::EventImpl* KJS::toEvent(ValueImp *val)
{
  ObjectImp *obj = val->getObject();
  if (!obj || !obj->inherits(&DOMEvent::info))
    return 0;

  const DOMEvent *dobj = static_cast<const DOMEvent*>(obj);
  return dobj->impl();
}

// -------------------------------------------------------------------------


const ClassInfo EventExceptionConstructor::info = { "EventExceptionConstructor", 0, &EventExceptionConstructorTable, 0 };
/*
@begin EventExceptionConstructorTable 1
  UNSPECIFIED_EVENT_TYPE_ERR    DOM::EventException::UNSPECIFIED_EVENT_TYPE_ERR DontDelete|ReadOnly
@end
*/
EventExceptionConstructor::EventExceptionConstructor(ExecState *exec)
  : DOMObject(exec->lexicalInterpreter()->builtinObjectPrototype())
{
}

bool EventExceptionConstructor::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<EventExceptionConstructor, DOMObject>(exec,&EventExceptionConstructorTable,this,propertyName,slot);
}

ValueImp *EventExceptionConstructor::getValueProperty(ExecState *, int token) const
{
  // We use the token as the value to return directly
  return Number(token);
}

ValueImp *KJS::getEventExceptionConstructor(ExecState *exec)
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
IMPLEMENT_PROTOFUNC(DOMUIEventProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMUIEventProto,DOMUIEventProtoFunc,DOMEventProto)

DOMUIEvent::DOMUIEvent(ExecState *exec, DOM::UIEventImpl* ue) :
  DOMEvent(DOMUIEventProto::self(exec), ue) {}

DOMUIEvent::DOMUIEvent(ObjectImp *proto, DOM::UIEventImpl* ue) :
  DOMEvent(proto, ue) {}

DOMUIEvent::~DOMUIEvent()
{
}

bool DOMUIEvent::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<DOMUIEvent, DOMEvent>(exec,&DOMUIEventTable,this,propertyName,slot);
}

ValueImp *DOMUIEvent::getValueProperty(ExecState *exec, int token) const
{
  DOM::UIEventImpl& event = *impl();
  switch (token) {
  case View:
    return getDOMAbstractView(exec,event.view());
  case Detail:
    return Number(event.detail());
  case KeyCode:
    // IE-compatibility
    return Number(event.keyCode());
  case CharCode:
    // IE-compatibility
    return Number(event.charCode());
  case LayerX:
    // NS-compatibility
    return Number(event.layerX());
  case LayerY:
    // NS-compatibility
    return Number(event.layerY());
  case PageX:
    // NS-compatibility
    return Number(event.pageX());
  case PageY:
    // NS-compatibility
    return Number(event.pageY());
  case Which:
    // NS-compatibility
    return Number(event.which());
  default:
    kdDebug(6070) << "WARNING: Unhandled token in DOMUIEvent::getValueProperty : " << token << endl;
    return Undefined();
  }
}

ValueImp *DOMUIEventProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMUIEvent, thisObj );
  DOM::UIEventImpl& uiEvent = *static_cast<DOMUIEvent *>(thisObj)->impl();
  switch (id) {
    case DOMUIEvent::InitUIEvent: {
      DOM::AbstractViewImpl* v = toAbstractView(args[3]);
      uiEvent.initUIEvent(args[0]->toString(exec).domString(),
                                                     args[1]->toBoolean(exec),
                                                     args[2]->toBoolean(exec),
                                                     v,
                                                     args[4]->toInteger(exec));
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
IMPLEMENT_PROTOFUNC(DOMMouseEventProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMMouseEventProto,DOMMouseEventProtoFunc,DOMUIEventProto)

DOMMouseEvent::DOMMouseEvent(ExecState *exec, DOM::MouseEventImpl* me) :
  DOMUIEvent(DOMMouseEventProto::self(exec), me) {}

DOMMouseEvent::~DOMMouseEvent()
{
}

bool DOMMouseEvent::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMMouseEvent::getOwnPropertySlot " << p.qstring() << endl;
#endif

  return getStaticValueSlot<DOMMouseEvent, DOMUIEvent>(exec,&DOMMouseEventTable,this,propertyName,slot);
}

ValueImp *DOMMouseEvent::getValueProperty(ExecState *exec, int token) const
{
  DOM::MouseEventImpl& event = *impl();
  switch (token) {
  case ScreenX:
    return Number(event.screenX());
  case ScreenY:
    return Number(event.screenY());
  case ClientX:
  case X:
    return Number(event.clientX());
  case ClientY:
  case Y:
    return Number(event.clientY());
  case OffsetX:
  case OffsetY: // MSIE extension
  {
    DOM::Node node = event.target();
    node.handle()->getDocument()->updateRendering();
    khtml::RenderObject *rend = node.handle() ? node.handle()->renderer() : 0L;
    int x = event.clientX();
    int y = event.clientY();
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
    return Boolean(event.ctrlKey());
  case ShiftKey:
    return Boolean(event.shiftKey());
  case AltKey:
    return Boolean(event.altKey());
  case MetaKey:
    return Boolean(event.metaKey());
  case Button:
  {
    if ( exec->interpreter()->compatMode() == Interpreter::NetscapeCompat ) {
        return Number(event.button());
    }
    // Tricky. The DOM (and khtml) use 0 for LMB, 1 for MMB and 2 for RMB
    // but MSIE uses 1=LMB, 2=RMB, 4=MMB, as a bitfield
    int domButton = event.button();
    int button = domButton==0 ? 1 : domButton==1 ? 4 : domButton==2 ? 2 : 0;
    return Number( (unsigned int)button );
  }
  case ToElement:
    // MSIE extension - "the object toward which the user is moving the mouse pointer"
    if (event.id() == DOM::EventImpl::MOUSEOUT_EVENT)
      return getDOMNode(exec,event.relatedTarget());
    return getDOMNode(exec,event.target());
  case FromElement:
    // MSIE extension - "object from which activation
    // or the mouse pointer is exiting during the event" (huh?)
    if (event.id() == DOM::EventImpl::MOUSEOUT_EVENT)
      return getDOMNode(exec,event.target());
    /* fall through */
  case RelatedTarget:
    return getDOMNode(exec,event.relatedTarget());
  default:
    kdDebug(6070) << "WARNING: Unhandled token in DOMMouseEvent::getValueProperty : " << token << endl;
    return 0;
  }
}

ValueImp *DOMMouseEventProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMMouseEvent, thisObj );
  DOM::MouseEventImpl& mouseEvent = *static_cast<DOMMouseEvent *>(thisObj)->impl();
  switch (id) {
    case DOMMouseEvent::InitMouseEvent:
      mouseEvent.initMouseEvent(args[0]->toString(exec).domString(), // typeArg
                                args[1]->toBoolean(exec), // canBubbleArg
                                args[2]->toBoolean(exec), // cancelableArg
                                toAbstractView(args[3]), // viewArg
                                args[4]->toInteger(exec), // detailArg
                                args[5]->toInteger(exec), // screenXArg
                                args[6]->toInteger(exec), // screenYArg
                                args[7]->toInteger(exec), // clientXArg
                                args[8]->toInteger(exec), // clientYArg
                                args[9]->toBoolean(exec), // ctrlKeyArg
                                args[10]->toBoolean(exec), // altKeyArg
                                args[11]->toBoolean(exec), // shiftKeyArg
                                args[12]->toBoolean(exec), // metaKeyArg
                                args[13]->toInteger(exec), // buttonArg
                                toNode(args[14])); // relatedTargetArg
      return Undefined();
  }
  return Undefined();
}

// -------------------------------------------------------------------------

const ClassInfo DOMTextEvent::info = { "TextEvent", &DOMUIEvent::info, &DOMTextEventTable, 0 };

/*
@begin DOMTextEventTable 5
  keyVal   	 DOMTextEvent::Key	     DontDelete|ReadOnly
  virtKeyVal	 DOMTextEvent::VirtKey        DontDelete|ReadOnly
  outputString	 DOMTextEvent::OutputString   DontDelete|ReadOnly
  inputGenerated DOMTextEvent::InputGenerated DontDelete|ReadOnly
  numPad         DOMTextEvent::NumPad         DontDelete|ReadOnly
  # actually belonging to KeyboardEvent
  ctrlKey        DOMTextEvent::CtrlKey     DontDelete|ReadOnly
  altKey         DOMTextEvent::AltKey      DontDelete|ReadOnly
  shiftKey       DOMTextEvent::ShiftKey    DontDelete|ReadOnly
  altKey         DOMTextEvent::AltKey      DontDelete|ReadOnly
@end
@begin DOMTextEventProtoTable 1
  initTextEvent	DOMTextEvent::InitTextEvent	DontDelete|Function 10
  # Missing: initTextEventNS, initModifier
@end
*/
DEFINE_PROTOTYPE("DOMTextEvent",DOMTextEventProto)
IMPLEMENT_PROTOFUNC(DOMTextEventProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMTextEventProto,DOMTextEventProtoFunc,DOMUIEventProto)

DOMTextEvent::DOMTextEvent(ExecState *exec, DOM::TextEventImpl* ke) :
  DOMUIEvent(DOMTextEventProto::self(exec), ke) {}

DOMTextEvent::~DOMTextEvent()
{
}

bool DOMTextEvent::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMTextEvent::getOwnPropertySlot " << p.qstring() << endl;
#endif

  return getStaticValueSlot<DOMTextEvent, DOMUIEvent>(exec,&DOMTextEventTable,this,propertyName,slot);
}


ValueImp *DOMTextEvent::getValueProperty(ExecState *, int token) const
{
  // ### KDE 4: use const reference
  DOM::TextEventImpl& tevent = *impl();
  switch (token) {
  case Key:
    return Number(tevent.keyVal());
  case VirtKey:
    return Number(tevent.virtKeyVal());
  case OutputString:
    return String(tevent.outputString());
  case InputGenerated:
    return Boolean(tevent.inputGenerated());
  case NumPad:
    return Boolean(tevent.numPad());
  // these modifier attributes actually belong into a KeyboardEvent interface
  case CtrlKey:
    return Boolean(tevent.checkModifier(Qt::ControlModifier));
  case ShiftKey:
    return Boolean(tevent.checkModifier(Qt::ShiftModifier));
  case AltKey:
    return Boolean(tevent.checkModifier(Qt::AltModifier));
  case MetaKey:
    return Boolean(tevent.checkModifier(Qt::MetaModifier));
  default:
    kdDebug(6070) << "WARNING: Unhandled token in DOMTextEvent::getValueProperty : " << token << endl;
    return KJS::Undefined();
  }
}

ValueImp *DOMTextEventProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMTextEvent, thisObj );
  DOM::TextEventImpl& keyEvent = *static_cast<DOMTextEvent *>(thisObj)->impl();
  switch (id) {
    case DOMTextEvent::InitTextEvent:
      keyEvent.initTextEvent(args[0]->toString(exec).domString(), // typeArg
                            args[1]->toBoolean(exec), // canBubbleArg
                            args[2]->toBoolean(exec), // cancelableArg
                            toAbstractView(args[3]), // viewArg
                            args[4]->toInteger(exec), // detailArg
                            args[5]->toString(exec).domString(),  // outputStringArg
                            args[6]->toInteger(exec), // keyValArg
                            args[7]->toInteger(exec), // virtKeyValArg
                            args[8]->toBoolean(exec), // inputGeneratedArg
                            args[9]->toBoolean(exec));// numPadArg

      return Undefined();
  }
  return Undefined();
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
  : DOMObject(exec->lexicalInterpreter()->builtinObjectPrototype())
{
}

bool MutationEventConstructor::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<MutationEventConstructor, DOMObject>(exec,&MutationEventConstructorTable,this,propertyName,slot);
}

ValueImp *MutationEventConstructor::getValueProperty(ExecState *, int token) const
{
  // We use the token as the value to return directly
  return Number(token);
}

ValueImp *KJS::getMutationEventConstructor(ExecState *exec)
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
IMPLEMENT_PROTOFUNC(DOMMutationEventProtoFunc)
IMPLEMENT_PROTOTYPE_WITH_PARENT(DOMMutationEventProto,DOMMutationEventProtoFunc,DOMEventProto)

DOMMutationEvent::DOMMutationEvent(ExecState *exec, DOM::MutationEventImpl* me) :
  DOMEvent(DOMMutationEventProto::self(exec), me) {}

DOMMutationEvent::~DOMMutationEvent()
{
}

bool DOMMutationEvent::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<DOMMutationEvent, DOMEvent>(exec,&DOMMutationEventTable,this,propertyName,slot);
}

ValueImp *DOMMutationEvent::getValueProperty(ExecState *exec, int token) const
{
  DOM::MutationEventImpl& event = *impl();
  switch (token) {
  case RelatedNode: {
    DOM::Node relatedNode = event.relatedNode();
    return getDOMNode(exec,relatedNode.handle());
  }
  case PrevValue:
    return String(event.prevValue());
  case NewValue:
    return String(event.newValue());
  case AttrName:
    return String(event.attrName());
  case AttrChange:
    return Number((unsigned int)event.attrChange());
  default:
    kdDebug(6070) << "WARNING: Unhandled token in DOMMutationEvent::getValueProperty : " << token << endl;
    return 0;
  }
}

ValueImp *DOMMutationEventProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMMutationEvent, thisObj );
  DOM::MutationEventImpl& mutationEvent = *static_cast<DOMMutationEvent *>(thisObj)->impl();
  switch (id) {
    case DOMMutationEvent::InitMutationEvent:
      mutationEvent.initMutationEvent(args[0]->toString(exec).domString(), // typeArg,
                                      args[1]->toBoolean(exec), // canBubbleArg
                                      args[2]->toBoolean(exec), // cancelableArg
                                      toNode(args[3]), // relatedNodeArg
                                      args[4]->toString(exec).domString(), // prevValueArg
                                      args[5]->toString(exec).domString(), // newValueArg
                                      args[6]->toString(exec).domString(), // attrNameArg
                                      args[7]->toInteger(exec)); // attrChangeArg
      return Undefined();
  }
  return Undefined();
}
