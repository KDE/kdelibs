/*
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 2001 Peter Kelly (pmk@post.com)
 * (C) 2001 Tobias Anton (anton@stud.fbi.fh-darmstadt.de)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */

#ifndef _DOM_EventsImpl_h_
#define _DOM_EventsImpl_h_

#include "dom/dom2_events.h"
#include "misc/shared.h"
#include "xml/dom2_viewsimpl.h"
#include <qdatetime.h>
#include <qevent.h>

class KHTMLPart;

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
	// khtml events (not part of DOM)
	KHTML_ECMA_DBLCLICK_EVENT, // for html ondblclick
	KHTML_ECMA_CLICK_EVENT, // for html onclick
	KHTML_DRAGDROP_EVENT,
	KHTML_ERROR_EVENT,
	KHTML_KEYDOWN_EVENT,
	KHTML_KEYPRESS_EVENT,
	KHTML_KEYUP_EVENT,
	KHTML_MOVE_EVENT,
	KHTML_ORIGCLICK_MOUSEUP_EVENT
    };

    EventImpl();
    EventImpl(EventId _id, bool canBubbleArg, bool cancelableArg);
    virtual ~EventImpl();

    EventId id() { return m_id; }

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

    void preventDefault() { m_defaultPrevented = m_cancelable; }
    void initEvent(const DOMString &eventTypeArg, bool canBubbleArg, bool cancelableArg);

    virtual bool isUIEvent() { return false; }
    virtual bool isMouseEvent() { return false; }
    virtual bool isTextEvent() { return false; }
    virtual bool isMutationEvent() { return false; }
    virtual DOMString eventModuleName() { return ""; }

    virtual bool propagationStopped() { return m_propagationStopped; }
    virtual bool defaultPrevented() { return m_defaultPrevented; }

    static EventId typeToId(DOMString type);
    static DOMString idToType(EventId id);

    void setDefaultHandled() { m_defaultHandled = true; }
    bool defaultHandled() const { return m_defaultHandled; }

protected:
    DOMStringImpl *m_type;
    bool m_canBubble;
    bool m_cancelable;

    bool m_propagationStopped;
    bool m_defaultPrevented;
    bool m_defaultHandled;
    EventId m_id;
    NodeImpl *m_currentTarget; // ref > 0 maintained externally
    unsigned short m_eventPhase;
    NodeImpl *m_target;
    QDateTime m_createTime;
};



class UIEventImpl : public EventImpl
{
public:
    UIEventImpl() : m_view(0), m_detail(0) {};
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
    virtual bool isUIEvent() { return true; }
    virtual DOMString eventModuleName() { return "UIEvents"; }
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
		   bool ctrlKeyArg,
		   bool altKeyArg,
		   bool shiftKeyArg,
		   bool metaKeyArg,
		   unsigned short buttonArg,
		   NodeImpl *relatedTargetArg);
    virtual ~MouseEventImpl();
    long screenX() const { return m_screenX; }
    long screenY() const { return m_screenY; }
    long clientX() const { return m_clientX; }
    long clientY() const { return m_clientY; }
    bool ctrlKey() const { return m_ctrlKey; }
    bool shiftKey() const { return m_shiftKey; }
    bool altKey() const { return m_altKey; }
    bool metaKey() const { return m_metaKey; }
    unsigned short button() const { return m_button; }
    NodeImpl *relatedTarget() const { return m_relatedTarget; }


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
    virtual bool isMouseEvent() { return true; }
    virtual DOMString eventModuleName() { return "MouseEvents"; }
protected:
    long m_screenX;
    long m_screenY;
    long m_clientX;
    long m_clientY;
    bool m_ctrlKey;
    bool m_altKey;
    bool m_shiftKey;
    bool m_metaKey;
    unsigned short m_button;
    NodeImpl *m_relatedTarget;
};


class TextEventImpl : public UIEventImpl {
public:
  TextEventImpl();
  TextEventImpl(EventId _id,
	       bool canBubbleArg,
	       bool cancelableArg,
	       AbstractViewImpl *viewArg,
	       unsigned short detailArg,
	       DOMString &outputStringArg,
	       unsigned long keyValArg,
	       unsigned long virtKeyValArg,
	       bool inputGeneratedArg,
	       bool numPadArg);

  TextEventImpl(QKeyEvent *key, AbstractViewImpl *view);

  virtual ~TextEventImpl();

  // VirtualKeyCode
  enum KeyCodes  {
         DOM_VK_UNDEFINED               = 0x0,
         DOM_VK_RIGHT_ALT               = 0x01,
         DOM_VK_LEFT_ALT                = 0x02,
         DOM_VK_LEFT_CONTROL            = 0x03,
         DOM_VK_RIGHT_CONTROL           = 0x04,
         DOM_VK_LEFT_SHIFT              = 0x05,
         DOM_VK_RIGHT_SHIFT             = 0x06,
         DOM_VK_LEFT_META               = 0x07,
         DOM_VK_RIGHT_META              = 0x08,
         DOM_VK_CAPS_LOCK               = 0x09,
         DOM_VK_DELETE                  = 0x0A,
         DOM_VK_END                     = 0x0B,
         DOM_VK_ENTER                   = 0x0C,
         DOM_VK_ESCAPE                  = 0x0D,
         DOM_VK_HOME                    = 0x0E,
         DOM_VK_INSERT                  = 0x0F,
         DOM_VK_NUM_LOCK                = 0x10,
         DOM_VK_PAUSE                   = 0x11,
         DOM_VK_PRINTSCREEN             = 0x12,
         DOM_VK_SCROLL_LOCK             = 0x13,
         DOM_VK_LEFT                    = 0x14,
         DOM_VK_RIGHT                   = 0x15,
         DOM_VK_UP                      = 0x16,
         DOM_VK_DOWN                    = 0x17,
         DOM_VK_PAGE_DOWN               = 0x18,
         DOM_VK_PAGE_UP                 = 0x19,
         DOM_VK_F1                      = 0x1A,
         DOM_VK_F2                      = 0x1B,
         DOM_VK_F3                      = 0x1C,
         DOM_VK_F4                      = 0x1D,
         DOM_VK_F5                      = 0x1E,
         DOM_VK_F6                      = 0x1F,
         DOM_VK_F7                      = 0x20,
         DOM_VK_F8                      = 0x21,
         DOM_VK_F9                      = 0x22,
         DOM_VK_F10                     = 0x23,
         DOM_VK_F11                     = 0x24,
         DOM_VK_F12                     = 0x25,
         DOM_VK_F13                     = 0x26,
         DOM_VK_F14                     = 0x27,
         DOM_VK_F15                     = 0x28,
         DOM_VK_F16                     = 0x29,
         DOM_VK_F17                     = 0x2A,
         DOM_VK_F18                     = 0x2B,
         DOM_VK_F19                     = 0x2C,
         DOM_VK_F20                     = 0x2D,
         DOM_VK_F21                     = 0x2E,
         DOM_VK_F22                     = 0x2F,
         DOM_VK_F23                     = 0x30,
         DOM_VK_F24                     = 0x31
  };

  void initTextEvent(const DOMString &typeArg,
                    bool canBubbleArg,
                    bool cancelableArg,
                    const AbstractView &viewArg,
                    long detailArg,
                    const DOMString &outputStringArg,
                    unsigned long keyValArg,
                    unsigned long virtKeyValArg,
                    bool inputGeneratedArg,
                    bool numPadArg);
  void initModifier(unsigned long modifierArg, bool valueArg);

  bool checkModifier(unsigned long modiferArg);

 //Attributes:
    bool             inputGenerated() const { return m_inputGenerated; }
    unsigned long    keyVal() const { return m_keyVal; }
    unsigned long    virtKeyVal() const { return m_virtKeyVal; }
    bool             numPad() const { return m_numPad; }
    DOMString        outputString() const { return m_outputString; }

  virtual DOMString eventModuleName() { return "TextEvents"; }
  virtual bool isTextEvent() { return true; }

 QKeyEvent *qKeyEvent;

private:
  unsigned long m_keyVal;
  unsigned long m_virtKeyVal;
  bool m_inputGenerated;
  DOMString m_outputString;
  bool m_numPad;
  // bitfield containing state of modifiers. not part of the dom.
  unsigned long    m_modifier;
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
    virtual bool isMutationEvent() { return true; }
    virtual DOMString eventModuleName() { return "MutationEvents"; }
protected:
    NodeImpl *m_relatedNode;
    DOMStringImpl *m_prevValue;
    DOMStringImpl *m_newValue;
    DOMStringImpl *m_attrName;
    unsigned short m_attrChange;
};


class RegisteredEventListener {
public:
    RegisteredEventListener(EventImpl::EventId _id, EventListener *_listener, bool _useCapture)
        : id(_id), listener(_listener), useCapture(_useCapture) {};

    ~RegisteredEventListener() { listener->deref(); }

    bool operator==(const RegisteredEventListener &other)
    { return id == other.id && listener == other.listener && useCapture == other.useCapture; }


    EventImpl::EventId id;
    EventListener *listener;
    bool useCapture;
private:
    RegisteredEventListener( const RegisteredEventListener & );
    RegisteredEventListener & operator=( const RegisteredEventListener & );
};

}; //namespace
#endif
