// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _KJS_EVENTS_H_
#define _KJS_EVENTS_H_

#include "ecma/kjs_dom.h"
#include "dom/dom2_events.h"
#include "dom/dom_misc.h"

namespace KJS {

  class Window;

  class JSEventListener : public DOM::EventListener {
  public:
    /**
     * @param _listener the function object, that will be called when the event is emitted
     * @param _win Window object, for memory management and caching.
     * Never create a JSEventListener directly, use Window::getJSEventListener.
     */
    JSEventListener(Object _listener, const Object &_win, bool _html = false);
    virtual ~JSEventListener();
    void hackSetThisObj( Object& thisObj ) { m_hackThisObj = thisObj; }
    void hackUnsetThisObj() { m_hackThisObj = Object(0L); }
    virtual void handleEvent(DOM::Event &evt);
    virtual DOM::DOMString eventListenerType();
    Object listenerObj() { return listener; }
  protected:
    Object listener;
    bool html;
    Object win, m_hackThisObj;
  };

  Value getNodeEventListener(DOM::Node n, int eventId);

  // Constructor for Event - currently only used for some global vars
  class EventConstructor : public DOMObject {
  public:
    EventConstructor(ExecState *);
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    Value getValueProperty(ExecState *, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  Value getEventConstructor(ExecState *exec);

  class DOMEvent : public DOMObject {
  public:
    DOMEvent(ExecState *exec, DOM::Event e);
    ~DOMEvent();
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    Value getValueProperty(ExecState *, int token) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName,
			const Value& value, int attr = None);
    void putValueProperty(ExecState *exec, int token, const Value& value, int);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Type, Target, CurrentTarget, EventPhase, Bubbles,
           Cancelable, TimeStamp, StopPropagation, PreventDefault, InitEvent,
	   // MS IE equivalents
	   SrcElement, ReturnValue, CancelBubble };
    DOM::Event toEvent() const { return event; }
  protected:
    DOM::Event event;
  };

  Value getDOMEvent(ExecState *exec, DOM::Event e);

  /**
   * Convert an object to an Event. Returns a null Event if not possible.
   */
  DOM::Event toEvent(const Value&);

  // Constructor object EventException
  class EventExceptionConstructor : public DOMObject {
  public:
    EventExceptionConstructor(ExecState *);
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    Value getValueProperty(ExecState *, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  Value getEventExceptionConstructor(ExecState *exec);

  class DOMUIEvent : public DOMEvent {
  public:
    DOMUIEvent(ExecState *exec, DOM::UIEvent ue) : DOMEvent(exec, ue) {}
    ~DOMUIEvent();
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    Value getValueProperty(ExecState *, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { View, Detail, InitUIEvent };
    DOM::UIEvent toUIEvent() const { return static_cast<DOM::UIEvent>(event); }
  };

  class DOMMouseEvent : public DOMUIEvent {
  public:
    DOMMouseEvent(ExecState *exec, DOM::MouseEvent me) : DOMUIEvent(exec, me) {}
    ~DOMMouseEvent();
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    Value getValueProperty(ExecState *, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { ScreenX, ScreenY, ClientX, X, ClientY, Y, OffsetX, OffsetY,
           CtrlKey, ShiftKey, AltKey,
           MetaKey, Button, RelatedTarget, FromElement, ToElement,
           PageX, PageY, LayerX, LayerY, InitMouseEvent
    };
    DOM::MouseEvent toMouseEvent() const { return static_cast<DOM::MouseEvent>(event); }
  };

  class DOMTextEvent : public DOMUIEvent {
  public:
    DOMTextEvent(ExecState *exec, DOM::TextEvent ke) : DOMUIEvent(exec, ke) {}
    ~DOMTextEvent();
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    Value getValueProperty(ExecState *, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Key, VirtKey, OutputString, InitTextEvent, InputGenerated, NumPad };
    DOM::TextEvent toTextEvent() const { return static_cast<DOM::TextEvent>(event); }
  };

  // Constructor object MutationEvent
  class MutationEventConstructor : public DOMObject {
  public:
    MutationEventConstructor(ExecState *);
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    Value getValueProperty(ExecState *, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  Value getMutationEventConstructor(ExecState *exec);

  class DOMMutationEvent : public DOMEvent {
  public:
    DOMMutationEvent(ExecState *exec, DOM::MutationEvent me) : DOMEvent(exec, me) {}
    ~DOMMutationEvent();
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    Value getValueProperty(ExecState *, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { AttrChange, RelatedNode, AttrName, PrevValue, NewValue,
           InitMutationEvent };
    DOM::MutationEvent toMutationEvent() const { return static_cast<DOM::MutationEvent>(event); }
  };

}; // namespace

#endif
