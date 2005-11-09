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
    JSEventListener(ObjectImp* _listener, ObjectImp *_compareListenerImp, ObjectImp* _win, bool _html = false);
    virtual ~JSEventListener();
    virtual void handleEvent(DOM::Event &evt);
    virtual DOM::DOMString eventListenerType();
    // Return the KJS function object executed when this event is emitted
    virtual ObjectImp* listenerObj() const;
    ObjectImp *listenerObjImp() const { return listenerObj().imp(); }
    // for Window::clear(). This is a bad hack though. The JSEventListener might not get deleted
    // if it was added to a DOM node in another frame (#61467). But calling removeEventListener on
    // all nodes we're listening to is quite difficult.
    void clear() { listener = Object(); }
    bool isHTMLEventListener() const { return html; }

  protected:
    mutable ObjectImp* listener;
    // Storing a different ObjectImp ptr is needed to support addEventListener(.. [Object] ..) calls
    // In the real-life case (where a 'function' is passed to addEventListener) we can directly call
    // the 'listener' object and can cache the 'listener.imp()'. If the event listener should be removed
    // the implementation will call removeEventListener(.. [Function] ..), and we can lookup the event
    // listener by the passed function's imp() ptr.
    // In the only dom-approved way (passing an Object to add/removeEventListener), the 'listener'
    // variable stores the function object 'passedListener.handleEvent'. But we need to cache
    // the imp() ptr of the 'passedListener' function _object_, as the implementation will
    // call removeEventListener(.. [Object ..] on removal, and now we can successfully lookup
    // the correct event listener, as well as the 'listener.handleEvent' function, we need to call.
    mutable ObjectImp *compareListenerImp;
    bool html;
    ObjectImp* win;
  };

  class JSLazyEventListener : public JSEventListener {
  public:
    JSLazyEventListener(const QString &_code, const QString &_name, ObjectImp* _win, DOM::NodeImpl* node);
    ~JSLazyEventListener();
    virtual void handleEvent(DOM::Event &evt);
    ObjectImp* listenerObj() const;
  private:
    void parseCode() const;

    mutable QString code;
    mutable QString name;
    mutable bool parsed;
    DOM::NodeImpl *originalNode;
  };

  // Constructor for Event - currently only used for some global vars
  class EventConstructor : public DOMObject {
  public:
    EventConstructor(ExecState *);
    virtual ValueImp* tryGet(ExecState *exec,const Identifier &p) const;
    ValueImp* getValueProperty(ExecState *, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  ValueImp* getEventConstructor(ExecState *exec);

  class DOMEvent : public DOMObject {
  public:
    // Build a DOMEvent
    DOMEvent(ExecState *exec, DOM::Event e);
    ~DOMEvent();
    virtual ValueImp* tryGet(ExecState *exec,const Identifier &p) const;
    ValueImp* getValueProperty(ExecState *, int token) const;
    virtual void tryPut(ExecState *exec, const Identifier &propertyName,
			ValueImp* value, int attr = None);
    virtual ValueImp* defaultValue(ExecState *exec, KJS::Type hint) const;
    void putValueProperty(ExecState *exec, int token, ValueImp* value, int);
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

  ValueImp* getDOMEvent(ExecState *exec, DOM::Event e);

  /**
   * Convert an object to an Event. Returns a null Event if not possible.
   */
  DOM::Event toEvent(ValueImp*);

  // Constructor object EventException
  class EventExceptionConstructor : public DOMObject {
  public:
    EventExceptionConstructor(ExecState *);
    virtual ValueImp* tryGet(ExecState *exec,const Identifier &p) const;
    ValueImp* getValueProperty(ExecState *, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  ValueImp* getEventExceptionConstructor(ExecState *exec);

  class DOMUIEvent : public DOMEvent {
  public:
    // Build a DOMUIEvent
    DOMUIEvent(ExecState *exec, DOM::UIEvent ue);
    ~DOMUIEvent();
    virtual ValueImp* tryGet(ExecState *exec,const Identifier &p) const;
    ValueImp* getValueProperty(ExecState *, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { View, Detail, KeyCode, CharCode, LayerX, LayerY, PageX, PageY, Which, InitUIEvent };
    DOM::UIEvent toUIEvent() const { return static_cast<DOM::UIEvent>(event); }
  };

  class DOMMouseEvent : public DOMUIEvent {
  public:
    DOMMouseEvent(ExecState *exec, DOM::MouseEvent me);
    ~DOMMouseEvent();
    virtual ValueImp* tryGet(ExecState *exec,const Identifier &p) const;
    ValueImp* getValueProperty(ExecState *, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { ScreenX, ScreenY, ClientX, X, ClientY, Y, OffsetX, OffsetY,
           CtrlKey, ShiftKey, AltKey,
           MetaKey, Button, RelatedTarget, FromElement, ToElement,
           InitMouseEvent
    };
    DOM::MouseEvent toMouseEvent() const { return static_cast<DOM::MouseEvent>(event); }
  };

  class DOMTextEvent : public DOMUIEvent {
  public:
    DOMTextEvent(ExecState *exec, DOM::TextEvent ke);
    ~DOMTextEvent();
    virtual ValueImp* tryGet(ExecState *exec,const Identifier &p) const;
    ValueImp* getValueProperty(ExecState *, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Key, VirtKey, OutputString, InitTextEvent, InputGenerated, NumPad,
           CtrlKey, ShiftKey, AltKey, MetaKey };
    DOM::TextEvent toTextEvent() const { return static_cast<DOM::TextEvent>(event); }
  };

  // Constructor object MutationEvent
  class MutationEventConstructor : public DOMObject {
  public:
    MutationEventConstructor(ExecState *);
    virtual ValueImp* tryGet(ExecState *exec,const Identifier &p) const;
    ValueImp* getValueProperty(ExecState *, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  ValueImp* getMutationEventConstructor(ExecState *exec);

  class DOMMutationEvent : public DOMEvent {
  public:
    DOMMutationEvent(ExecState *exec, DOM::MutationEvent me);
    ~DOMMutationEvent();
    virtual ValueImp* tryGet(ExecState *exec,const Identifier &p) const;
    ValueImp* getValueProperty(ExecState *, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { AttrChange, RelatedNode, AttrName, PrevValue, NewValue,
           InitMutationEvent };
    DOM::MutationEvent toMutationEvent() const { return static_cast<DOM::MutationEvent>(event); }
  };

} // namespace

#endif
