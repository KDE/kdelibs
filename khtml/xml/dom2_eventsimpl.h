/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 2001 Peter Kelly (pmk@post.com)
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

#include "dom2_events.h"
#include "dom2_viewsimpl.h"
#include "dom/dom_misc.h"

namespace DOM {

/*

// Introduced in DOM Level 2: - user inherit
class EventListener {
public:
    EventListener();
    virtual ~EventListener();
    virtual void handleEvent(const Event &evt);
};

*/

class EventImpl : public DomShared
{
public:
    EventImpl();
    virtual ~EventImpl();

    DOMString type() const;
    NodeImpl *target() const;
    NodeImpl *currentTarget() const;
    unsigned short eventPhase() const;
    bool bubbles() const;
    bool cancelable() const;
    DOMTimeStamp timeStamp();
    void stopPropagation();
    void preventDefault();
    void initEvent(const DOMString &eventTypeArg, bool canBubbleArg, bool cancelableArg);

    virtual bool isUIEvent() { return false; }
    virtual bool isMouseEvent() { return false; }
    virtual bool isMutationEvent() { return false; }
    virtual DOMString eventModuleName() { return ""; }

protected:
    DOMStringImpl *m_type;
    bool m_canBubble;
    bool m_cancelable;
};



class UIEventImpl : public EventImpl
{
public:
    UIEventImpl();
    virtual ~UIEventImpl();
    AbstractViewImpl *view() const;
    long detail() const;
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
    virtual ~MouseEventImpl();
    long screenX() const;
    long screenY() const;
    long clientX() const;
    long clientY() const;
    bool ctrlKey() const;
    bool shiftKey() const;
    bool altKey() const;
    bool metaKey() const;
    unsigned short button() const;
    NodeImpl *relatedTarget() const;
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



class MutationEventImpl : public EventImpl {
public:
    MutationEventImpl();
    ~MutationEventImpl();

    Node relatedNode() const;
    DOMString prevValue() const;
    DOMString newValue() const;
    DOMString attrName() const;
    unsigned short attrChange() const;
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
    RegisteredEventListener(DOMString _type, EventListener *_listener, bool _useCapture);

    bool operator==(const RegisteredEventListener &other);

    DOMString type;
    EventListener *listener;
    bool useCapture;
};


}; //namespace
#endif
