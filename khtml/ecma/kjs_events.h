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
 */

#ifndef _KJS_EVENTS_H_
#define _KJS_EVENTS_H_

#include "kjs_dom.h"
#include <dom2_events.h>
#include <dom_misc.h>

namespace KJS {

  class Window;

  class JSEventListener : public DOM::EventListener {
  public:
    JSEventListener(Object _listener, const Object &_win, bool _html = false);
    virtual ~JSEventListener();
    virtual void handleEvent(DOM::Event &evt);
    virtual DOM::DOMString eventListenerType();
    Object listenerObj() { return listener; }
  protected:
    Object listener;
    bool html;
    Object win;
  };

  Value getNodeEventListener(DOM::Node n, int eventId);

  // Prototype object Event
  class EventPrototype : public DOMObject {
  public:
    EventPrototype() { }
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  Value getEventPrototype(ExecState *exec);

  class DOMEvent : public DOMObject {
  public:
    DOMEvent(DOM::Event e) : event(e) {}
    ~DOMEvent();
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    virtual DOM::Event toEvent() const { return event; }
  protected:
    DOM::Event event;
  };

  class DOMEventFunc : public DOMFunction {
    friend class DOMNode;
  public:
    DOMEventFunc(DOM::Event e, int i) : DOMFunction(), event(e), id(i) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { StopPropagation, PreventDefault, InitEvent };
  private:
    DOM::Event event;
    int id;
  };

  Value getDOMEvent(DOM::Event e);

  /**
   * Convert an object to an Event. Returns a null Event if not possible.
   */
  DOM::Event toEvent(const Value&);

  // Prototype object EventException
  class EventExceptionPrototype : public DOMObject {
  public:
    EventExceptionPrototype() { }
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  Value getEventExceptionPrototype(ExecState *exec);

  class DOMUIEvent : public DOMEvent {
  public:
    DOMUIEvent(DOM::UIEvent ue) : DOMEvent(ue) {}
    ~DOMUIEvent();
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMUIEventFunc : public DOMFunction {
  public:
    DOMUIEventFunc(DOM::UIEvent ue, int i)
        : DOMFunction(), uiEvent(ue), id(i) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { InitUIEvent };
  private:
    DOM::UIEvent uiEvent;
    int id;
  };

  class DOMMouseEvent : public DOMUIEvent {
  public:
    DOMMouseEvent(DOM::MouseEvent me) : DOMUIEvent(me) {}
    ~DOMMouseEvent();
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMMouseEventFunc : public DOMFunction {
  public:
    DOMMouseEventFunc(DOM::MouseEvent me, int i)
        : DOMFunction(), mouseEvent(me), id(i) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { InitMouseEvent };
  private:
    DOM::MouseEvent mouseEvent;
    int id;
  };

  // Prototype object MutationEvent
  class MutationEventPrototype : public DOMObject {
  public:
    MutationEventPrototype() { }
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  Value getMutationEventPrototype(ExecState *exec);

  class DOMMutationEvent : public DOMEvent {
  public:
    DOMMutationEvent(DOM::MutationEvent me) : DOMEvent(me) {}
    ~DOMMutationEvent();
    virtual Value tryGet(ExecState *exec,const UString &p) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMMutationEventFunc : public DOMFunction {
  public:
    DOMMutationEventFunc(DOM::MutationEvent me, int i)
        : DOMFunction(), mutationEvent(me), id(i) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { InitMutationEvent };
  private:
    DOM::MutationEvent mutationEvent;
    int id;
  };

}; // namespace

#endif
