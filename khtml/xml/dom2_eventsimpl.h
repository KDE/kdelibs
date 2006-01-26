/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *           (C) 2001 Tobias Anton (anton@stud.fbi.fh-darmstadt.de)
 *           (C) 2002 Apple Computer, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef _DOM_EventsImpl_h_
#define _DOM_EventsImpl_h_

#include "dom/dom2_events.h"
#include "xml/dom2_viewsimpl.h"

class KHTMLPart;
class QMouseEvent;

namespace DOM {

class AbstractViewImpl;
class DOMStringImpl;
class NodeImpl;

// ### support user-defined events

class EventImpl : public khtml::Shared<EventImpl>
{
public:
    enum EventId {
	UNKNOWN_EVENT = 0,
	// UI events
        DOMFOCUSIN_EVENT,
        DOMFOCUSOUT_EVENT,
        DOMACTIVATE_EVENT,
        // Mouse events
        CLICK_EVENT,
        MOUSEDOWN_EVENT,
        MOUSEUP_EVENT,
        MOUSEOVER_EVENT,
        MOUSEMOVE_EVENT,
        MOUSEOUT_EVENT,
        // Mutation events
        DOMSUBTREEMODIFIED_EVENT,
        DOMNODEINSERTED_EVENT,
        DOMNODEREMOVED_EVENT,
        DOMNODEREMOVEDFROMDOCUMENT_EVENT,
        DOMNODEINSERTEDINTODOCUMENT_EVENT,
        DOMATTRMODIFIED_EVENT,
        DOMCHARACTERDATAMODIFIED_EVENT,
	// HTML events
	LOAD_EVENT,
	UNLOAD_EVENT,
	ABORT_EVENT,
	ERROR_EVENT,
	SELECT_EVENT,
	CHANGE_EVENT,
	SUBMIT_EVENT,
	RESET_EVENT,
	FOCUS_EVENT,
	BLUR_EVENT,
	RESIZE_EVENT,
	SCROLL_EVENT,
        // keyboard events
	KEYDOWN_EVENT,
	KEYUP_EVENT,
	KEYPRESS_EVENT, //Mostly corresponds to DOM3 textInput event.
	// khtml events (not part of DOM)
	KHTML_ECMA_DBLCLICK_EVENT, // for html ondblclick
	KHTML_ECMA_CLICK_EVENT, // for html onclick
	KHTML_DRAGDROP_EVENT,
	KHTML_MOVE_EVENT,
        // XMLHttpRequest events
        KHTML_READYSTATECHANGE_EVENT
    };

    EventImpl();
    EventImpl(EventId _id, bool canBubbleArg, bool cancelableArg);
    virtual ~EventImpl();

    EventId id() const { return m_id; }

    DOMString type() const { return m_type; }
    NodeImpl *target() const { return m_target; }
    void setTarget(NodeImpl *_target);
    NodeImpl *currentTarget() const { return m_currentTarget; }
    void setCurrentTarget(NodeImpl *_currentTarget) { m_currentTarget = _currentTarget; }
    unsigned short eventPhase() const { return m_eventPhase; }
    void setEventPhase(unsigned short _eventPhase) { m_eventPhase = _eventPhase; }
    bool bubbles() const { return m_canBubble; }
    bool cancelable() const { return m_cancelable; }
    DOMTimeStamp timeStamp();
    void stopPropagation(bool stop) { m_propagationStopped = stop; }
    void preventDefault(bool prevent) { if ( m_cancelable ) m_defaultPrevented = prevent; }

    void initEvent(const DOMString &eventTypeArg, bool canBubbleArg, bool cancelableArg);

    virtual bool isUIEvent() const;
    virtual bool isMouseEvent() const;
    virtual bool isMutationEvent() const;
    virtual bool isTextInputEvent() const;
    virtual bool isKeyboardEvent() const;
    bool isKeyRelatedEvent() const { return isTextInputEvent() || isKeyboardEvent(); }

    bool propagationStopped() const { return m_propagationStopped; }
    bool defaultPrevented() const { return m_defaultPrevented; }

    static EventId typeToId(DOMString type);
    static DOMString idToType(EventId id);

    void setDefaultHandled() { m_defaultHandled = true; }
    bool defaultHandled() const { return m_defaultHandled; }

    DOMString message() const { return m_message; }
    void setMessage(const DOMString &_message) { m_message = _message; }

protected:
    DOMStringImpl *m_type;
    bool m_canBubble;
    bool m_cancelable;

    bool m_propagationStopped;
    bool m_defaultPrevented;
    bool m_defaultHandled;
    EventId m_id : 6;
    unsigned short m_eventPhase : 2;
    NodeImpl *m_currentTarget; // ref > 0 maintained externally
    NodeImpl *m_target;
    QDateTime m_createTime;
    DOMString m_message;
};



class UIEventImpl : public EventImpl
{
public:
    UIEventImpl() : m_view(0), m_detail(0) {}
    UIEventImpl(EventId _id,
		bool canBubbleArg,
		bool cancelableArg,
		AbstractViewImpl *viewArg,
		long detailArg);
    virtual ~UIEventImpl();
    AbstractViewImpl *view() const { return m_view; }
    long detail() const { return m_detail; }
    void initUIEvent(const DOMString &typeArg,
		     bool canBubbleArg,
		     bool cancelableArg,
		     const AbstractView &viewArg,
		     long detailArg);
    virtual bool isUIEvent() const;

protected:
    AbstractViewImpl *m_view;
    long m_detail;

};

// Introduced in DOM Level 2: - internal
class MouseEventImpl : public UIEventImpl {
public:
    MouseEventImpl();
    MouseEventImpl(EventId _id,
		   bool canBubbleArg,
		   bool cancelableArg,
		   AbstractViewImpl *viewArg,
		   long detailArg,
		   long screenXArg,
		   long screenYArg,
		   long clientXArg,
		   long clientYArg,
                   long pageXArg,
                   long pageYArg,
		   bool ctrlKeyArg,
		   bool altKeyArg,
		   bool shiftKeyArg,
		   bool metaKeyArg,
		   unsigned short buttonArg,
		   NodeImpl *relatedTargetArg,
		   QMouseEvent *qe = 0,
                   bool isDoubleClick = false);
    virtual ~MouseEventImpl();
    long screenX() const { return m_screenX; }
    long screenY() const { return m_screenY; }
    long clientX() const { return m_clientX; }
    long clientY() const { return m_clientY; }
    long layerX() const { return m_layerX; } // non-DOM extension
    long layerY() const { return m_layerY; } // non-DOM extension
    long pageX() const { return m_pageX; } // non-DOM extension
    long pageY() const { return m_pageY; } // non-DOM extension
    bool isDoubleClick() const { return m_isDoubleClick; } // non-DOM extension
    bool ctrlKey() const { return m_ctrlKey; }
    bool shiftKey() const { return m_shiftKey; }
    bool altKey() const { return m_altKey; }
    bool metaKey() const { return m_metaKey; }
    unsigned short button() const { return m_button; }
    NodeImpl *relatedTarget() const { return m_relatedTarget; }

    void computeLayerPos();

    void initMouseEvent(const DOMString &typeArg,
			bool canBubbleArg,
			bool cancelableArg,
			const AbstractView &viewArg,
			long detailArg,
			long screenXArg,
			long screenYArg,
			long clientXArg,
			long clientYArg,
			bool ctrlKeyArg,
			bool altKeyArg,
			bool shiftKeyArg,
			bool metaKeyArg,
			unsigned short buttonArg,
			const Node &relatedTargetArg);
    virtual bool isMouseEvent() const;

    QMouseEvent *qEvent() const { return m_qevent; }
protected:
    long m_screenX;
    long m_screenY;
    long m_clientX;
    long m_clientY;
    long m_layerX;
    long m_layerY;
    long m_pageX;
    long m_pageY;
    bool m_ctrlKey : 1;
    bool m_altKey  : 1;
    bool m_shiftKey : 1;
    bool m_metaKey : 1;
    bool m_isDoubleClick : 1;
    unsigned short m_button;
    NodeImpl *m_relatedTarget;
    QMouseEvent *m_qevent;
};


class KeyEventBaseImpl : public UIEventImpl {
public:
  // VirtualKeyCode
  enum KeyCodes  {
      DOM_VK_UNDEFINED		     = 0x0,
      DOM_VK_RIGHT_ALT		     = 0x12,
      DOM_VK_LEFT_ALT		     = 0x12,
      DOM_VK_LEFT_CONTROL	     = 0x11,
      DOM_VK_RIGHT_CONTROL	     = 0x11,
      DOM_VK_LEFT_SHIFT		     = 0x10,
      DOM_VK_RIGHT_SHIFT	     = 0x10,
      DOM_VK_META		     = 0x9D,
      DOM_VK_BACK_SPACE		     = 0x08,
      DOM_VK_CAPS_LOCK		     = 0x14,
      DOM_VK_DELETE		     = 0x7F,
      DOM_VK_END		     = 0x23,
      DOM_VK_ENTER		     = 0x0D,
      DOM_VK_ESCAPE		     = 0x1B,
      DOM_VK_HOME		     = 0x24,
      DOM_VK_NUM_LOCK		     = 0x90,
      DOM_VK_PAUSE		     = 0x13,
      DOM_VK_PRINTSCREEN	     = 0x9A,
      DOM_VK_SCROLL_LOCK	     = 0x91,
      DOM_VK_SPACE		     = 0x20,
      DOM_VK_TAB		     = 0x09,
      DOM_VK_LEFT		     = 0x25,
      DOM_VK_RIGHT		     = 0x27,
      DOM_VK_UP			     = 0x26,
      DOM_VK_DOWN		     = 0x28,
      DOM_VK_PAGE_DOWN		     = 0x22,
      DOM_VK_PAGE_UP		     = 0x21,
      DOM_VK_F1			     = 0x70,
      DOM_VK_F2			     = 0x71,
      DOM_VK_F3			     = 0x72,
      DOM_VK_F4			     = 0x73,
      DOM_VK_F5			     = 0x74,
      DOM_VK_F6			     = 0x75,
      DOM_VK_F7			     = 0x76,
      DOM_VK_F8			     = 0x77,
      DOM_VK_F9			     = 0x78,
      DOM_VK_F10		     = 0x79,
      DOM_VK_F11		     = 0x7A,
      DOM_VK_F12		     = 0x7B,
      DOM_VK_F13		     = 0xF000,
      DOM_VK_F14		     = 0xF001,
      DOM_VK_F15		     = 0xF002,
      DOM_VK_F16		     = 0xF003,
      DOM_VK_F17		     = 0xF004,
      DOM_VK_F18		     = 0xF005,
      DOM_VK_F19		     = 0xF006,
      DOM_VK_F20		     = 0xF007,
      DOM_VK_F21		     = 0xF008,
      DOM_VK_F22		     = 0xF009,
      DOM_VK_F23		     = 0xF00A,
      DOM_VK_F24		     = 0xF00B
  };

  void initKeyBaseEvent(const DOMString &typeArg,
                         bool canBubbleArg,
                         bool cancelableArg,
                         const AbstractView &viewArg,
                         unsigned long keyVal,
                         unsigned long virtKeyVal,
                         unsigned long modifiers);

  bool ctrlKey()  const { return m_modifier & Qt::ControlButton; }
  bool shiftKey() const { return m_modifier & Qt::ShiftButton; }
  bool altKey()   const { return m_modifier & Qt::AltButton; }
  bool metaKey()  const { return m_modifier & Qt::MetaButton; }

  bool             inputGenerated() const { return m_virtKeyVal == 0; }
  unsigned long    keyVal() const     { return m_keyVal; }
  unsigned long    virtKeyVal() const { return m_virtKeyVal; }

  QKeyEvent *qKeyEvent() const { if (!m_keyEvent) buildQKeyEvent(); return m_keyEvent; }

  //Legacy key stuff...
  virtual int keyCode() const  = 0;
  virtual int charCode() const = 0;

  //### KDE4: remove these 2
  void initModifier(unsigned long modifierArg, bool valueArg);
  bool checkModifier(unsigned long modifierArg);

  ~KeyEventBaseImpl();

  //Returns true if the event was synthesized by client use of DOM
  bool isSynthetic() const { return m_synthetic; }
protected:
  KeyEventBaseImpl(): m_keyEvent(0), m_keyVal(0), m_virtKeyVal(0), m_modifier(0), m_synthetic(false)
  {  m_detail = 0; }

  KeyEventBaseImpl(EventId id,
                   bool canBubbleArg,
                   bool cancelableArg,
                   AbstractViewImpl *viewArg,
                   QKeyEvent *key);


  mutable QKeyEvent *m_keyEvent;
  unsigned long m_keyVal;     //Unicode key value
  unsigned long m_virtKeyVal; //Virtual key value for keys like arrows, Fn, etc.

  // bitfield containing state of modifiers. not part of the dom.
  unsigned long    m_modifier;

  bool             m_synthetic;

  void buildQKeyEvent() const; //Construct a Qt key event from m_keyVal/m_virtKeyVal
};

class TextEventImpl : public KeyEventBaseImpl {
public:
    TextEventImpl();

    TextEventImpl(QKeyEvent* key, DOM::AbstractViewImpl* view);
    
    void initTextEvent(const DOMString &typeArg,
                       bool canBubbleArg,
                       bool cancelableArg,
                       const AbstractView &viewArg,
                       const DOMString& text);

    virtual bool isTextInputEvent() const;

    //Legacy key stuff...
    int keyCode() const;
    int charCode() const;

    DOMString data() const { return m_outputString; }
private:
    DOMString m_outputString;
};

class KeyboardEventImpl : public KeyEventBaseImpl {
public:
  KeyboardEventImpl();
  KeyboardEventImpl(QKeyEvent* key, DOM::AbstractViewImpl* view);

  virtual bool isKeyboardEvent() const;

  enum KeyLocation {
    DOM_KEY_LOCATION_STANDARD      = 0x00,
    DOM_KEY_LOCATION_LEFT          = 0x01,
    DOM_KEY_LOCATION_RIGHT         = 0x02,
    DOM_KEY_LOCATION_NUMPAD        = 0x03
  };

  //Legacy key stuff...
  int keyCode() const;
  int charCode() const;

  DOMString     keyIdentifier() const;
  unsigned long keyLocation() const { return m_keyLocation; }

  bool getModifierState(const DOMString& keyIdentifierArg) const;

  void initKeyboardEvent(const DOMString &typeArg,
                         bool canBubbleArg,
                         bool cancelableArg,
                         const AbstractView &viewArg,
                         const DOMString &keyIdentifierArg,
                         unsigned long keyLocationArg,
                         const DOMString& modifiersList);

  //### KDE4: remove this, it's only for compatibility with
  //the old TextEvent wrapper
  void initKeyboardEvent(const DOMString &typeArg,
                         bool canBubbleArg,
                         bool cancelableArg,
                         const AbstractView &viewArg,
                         unsigned long keyVal,
                         unsigned long virtKeyVal,
                         unsigned long modifiers,
                         unsigned long keyLocationArg) {
     initKeyBaseEvent(typeArg, canBubbleArg, cancelableArg, viewArg,
        keyVal, virtKeyVal, modifiers);
     m_keyLocation = keyLocationArg;
  }
private:
    unsigned long m_keyLocation;
};



class MutationEventImpl : public EventImpl {
// ### fire these during parsing (if necessary)
public:
    MutationEventImpl();
    MutationEventImpl(EventId _id,
		      bool canBubbleArg,
		      bool cancelableArg,
		      const Node &relatedNodeArg,
		      const DOMString &prevValueArg,
		      const DOMString &newValueArg,
		      const DOMString &attrNameArg,
		      unsigned short attrChangeArg);
    ~MutationEventImpl();

    Node relatedNode() const { return m_relatedNode; }
    DOMString prevValue() const { return m_prevValue; }
    DOMString newValue() const { return m_newValue; }
    DOMString attrName() const { return m_attrName; }
    unsigned short attrChange() const { return m_attrChange; }
    void initMutationEvent(const DOMString &typeArg,
			   bool canBubbleArg,
			   bool cancelableArg,
			   const Node &relatedNodeArg,
			   const DOMString &prevValueArg,
			   const DOMString &newValueArg,
			   const DOMString &attrNameArg,
			   unsigned short attrChangeArg);
    virtual bool isMutationEvent() const;
protected:
    NodeImpl *m_relatedNode;
    DOMStringImpl *m_prevValue;
    DOMStringImpl *m_newValue;
    DOMStringImpl *m_attrName;
    unsigned short m_attrChange;
};


class RegisteredEventListener {
public:
    RegisteredEventListener() : id(EventImpl::EventId(0)), useCapture(false), listener(0) {}

    RegisteredEventListener(EventImpl::EventId _id, EventListener *_listener, bool _useCapture)
        : id(_id), useCapture(_useCapture), listener(_listener) { listener->ref(); }

    ~RegisteredEventListener() { if (listener) listener->deref(); listener = 0; }

    bool operator==(const RegisteredEventListener &other) const
    { return id == other.id && listener == other.listener && useCapture == other.useCapture; }


    EventImpl::EventId id : 6;
    bool useCapture;
    EventListener *listener;

    RegisteredEventListener( const RegisteredEventListener &other ) : 
                id(other.id), useCapture(other.useCapture), listener(other.listener) 
    { if (listener) listener->ref(); }

    RegisteredEventListener & operator=( const RegisteredEventListener &other ) {
        id         = other.id;
        useCapture = other.useCapture;
        if (other.listener)
            other.listener->ref();
        if (listener)
            listener->deref();
        listener = other.listener;
        return *this;
    }
};



} //namespace
#endif
