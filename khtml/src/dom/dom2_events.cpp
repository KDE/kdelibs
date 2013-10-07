/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright 2001 Peter Kelly (pmk@post.com)
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

#include "dom/dom2_views.h"
#include "dom/dom_exception.h"
#include "xml/dom2_eventsimpl.h"
#include "xml/dom_nodeimpl.h"

using namespace DOM;

EventListener::EventListener()
{
}

EventListener::~EventListener()
{
}

void EventListener::handleEvent(Event &/*evt*/)
{
}

DOMString EventListener::eventListenerType()
{
    return "";
}

// -----------------------------------------------------------------------------

Event::Event()
{
    impl = 0;
}


Event::Event(const Event &other)
{
    impl = other.impl;
    if (impl) impl->ref();
}

Event::Event(EventImpl *i)
{
    impl = i;
    if (impl) impl->ref();
}

Event::~Event()
{
    if (impl) impl->deref();
}

Event &Event::operator = (const Event &other)
{
    if ( impl != other.impl ) {
        if(impl) impl->deref();
        impl = other.impl;
        if(impl) impl->ref();
    }
    return *this;
}

DOMString Event::type() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return impl->type();
}

Node Event::target() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    if (impl->target()->eventTargetType() == EventTargetImpl::DOM_NODE)
        return static_cast<DOM::NodeImpl*>(impl->target());
    return 0;
}

Node Event::currentTarget() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    if (impl->currentTarget()->eventTargetType() == EventTargetImpl::DOM_NODE)
        return static_cast<DOM::NodeImpl*>(impl->currentTarget());
    return 0;
}

unsigned short Event::eventPhase() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return impl->eventPhase();
}

bool Event::bubbles() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return impl->bubbles();
}

bool Event::cancelable() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return impl->cancelable();
}

DOMTimeStamp Event::timeStamp() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return impl->timeStamp();
}

void Event::stopPropagation()
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    impl->stopPropagation(true);
}

void Event::preventDefault()
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    impl->preventDefault(true);
}

void Event::initEvent(const DOMString &eventTypeArg, bool canBubbleArg, bool cancelableArg)
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    impl->initEvent(eventTypeArg,canBubbleArg,cancelableArg);
}

EventImpl *Event::handle() const
{
    return impl;
}

bool Event::isNull() const
{
    return (impl == 0);
}

// -----------------------------------------------------------------------------

#ifndef SAVE_SPACE

EventException::EventException(unsigned short _code)
{
    code = _code;
}

EventException::EventException(const EventException &other)
{
    code = other.code;
}

EventException & EventException::operator = (const EventException &other)
{
    code = other.code;
    return *this;
}

#endif

DOMString EventException::codeAsString() const
{
    return codeAsString(code);
}

bool EventException::isEventExceptionCode(int exceptioncode)
{
    return exceptioncode >= _EXCEPTION_OFFSET && exceptioncode < _EXCEPTION_MAX;
}

DOMString EventException::codeAsString(int code)
{
    switch ( code ) {
    case UNSPECIFIED_EVENT_TYPE_ERR:
        return DOMString( "UNSPECIFIED_EVENT_TYPE_ERR" );
    default:
        return DOMString( "(unknown exception code)" );
    }
}


// -----------------------------------------------------------------------------

UIEvent::UIEvent() : Event()
{
}

UIEvent::UIEvent(const UIEvent &other) : Event(other)
{
}

UIEvent::UIEvent(const Event &other) : Event()
{
    (*this)=other;
}

UIEvent::UIEvent(UIEventImpl *impl) : Event(impl)
{
}

UIEvent &UIEvent::operator = (const UIEvent &other)
{
    Event::operator = (other);
    return *this;
}

UIEvent &UIEvent::operator = (const Event &other)
{
    Event e;
    e = other;
    if (!e.isNull() && !e.handle()->isUIEvent()) {
	if ( impl ) impl->deref();
	impl = 0;
    } else
	Event::operator = (other);
    return *this;
}

UIEvent::~UIEvent()
{
}

AbstractView UIEvent::view() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<UIEventImpl*>(impl)->view();
}

long UIEvent::detail() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<UIEventImpl*>(impl)->detail();
}

int UIEvent::keyCode() const
{
    if ( !impl ) throw DOMException( DOMException::INVALID_STATE_ERR );

    return static_cast<UIEventImpl*>(impl)->keyCode();
}

int UIEvent::charCode() const
{
    if (!impl)
        throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<UIEventImpl*>(impl)->charCode();
}

int UIEvent::pageX() const
{
    if (!impl)
        throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<UIEventImpl*>(impl)->pageX();
}

int UIEvent::pageY() const
{
    if (!impl)
        throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<UIEventImpl*>(impl)->pageY();
}

int UIEvent::layerX() const
{
    if( !impl )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    return static_cast<UIEventImpl*>(impl)->layerX();
}

int UIEvent::layerY() const
{
    if( !impl )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    return static_cast<UIEventImpl*>(impl)->layerY();
}

int UIEvent::which() const
{
    if( !impl ) throw DOMException( DOMException::INVALID_STATE_ERR );
    return static_cast<UIEventImpl*>(impl)->which();
}

void UIEvent::initUIEvent(const DOMString &typeArg,
                                 bool canBubbleArg,
                                 bool cancelableArg,
                                 const AbstractView &viewArg,
                                 long detailArg)
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    static_cast<UIEventImpl*>(impl)->initUIEvent(typeArg,canBubbleArg,cancelableArg,
						 viewArg.handle(),detailArg);
}

// -----------------------------------------------------------------------------

MouseEvent::MouseEvent() : UIEvent()
{
}

MouseEvent::MouseEvent(const MouseEvent &other) : UIEvent(other)
{
}

MouseEvent::MouseEvent(const Event &other) : UIEvent()
{
    (*this)=other;
}

MouseEvent::MouseEvent(MouseEventImpl *impl) : UIEvent(impl)
{
}

MouseEvent &MouseEvent::operator = (const MouseEvent &other)
{
    UIEvent::operator = (other);
    return *this;
}

MouseEvent &MouseEvent::operator = (const Event &other)
{
    Event e;
    e = other;
    if (!e.isNull() && !e.handle()->isMouseEvent()) {
	if ( impl ) impl->deref();
	impl = 0;
    } else
	UIEvent::operator = (other);
    return *this;
}

MouseEvent::~MouseEvent()
{
}

long MouseEvent::screenX() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MouseEventImpl*>(impl)->screenX();
}

long MouseEvent::screenY() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MouseEventImpl*>(impl)->screenY();
}

long MouseEvent::clientX() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MouseEventImpl*>(impl)->clientX();
}

long MouseEvent::clientY() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MouseEventImpl*>(impl)->clientY();
}

bool MouseEvent::ctrlKey() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MouseEventImpl*>(impl)->ctrlKey();
}

bool MouseEvent::shiftKey() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MouseEventImpl*>(impl)->shiftKey();
}

bool MouseEvent::altKey() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MouseEventImpl*>(impl)->altKey();
}

bool MouseEvent::metaKey() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MouseEventImpl*>(impl)->metaKey();
}

unsigned short MouseEvent::button() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MouseEventImpl*>(impl)->button();
}

Node MouseEvent::relatedTarget() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MouseEventImpl*>(impl)->relatedTarget();
}

void MouseEvent::initMouseEvent(const DOMString &typeArg,
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
                                    const Node &relatedTargetArg)
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    static_cast<MouseEventImpl*>(impl)->initMouseEvent(typeArg,canBubbleArg,
	cancelableArg,viewArg.handle(),detailArg,screenXArg,screenYArg,clientXArg,
        clientYArg,ctrlKeyArg,altKeyArg,shiftKeyArg,metaKeyArg,buttonArg,
	relatedTargetArg);
}

// -----------------------------------------------------------------------------

TextEvent::TextEvent() : UIEvent()
{
}

TextEvent::TextEvent(const TextEvent &other) : UIEvent(other)
{
}

TextEvent::TextEvent(const Event &other) : UIEvent()
{
    (*this)=other;
}

TextEvent &TextEvent::operator = (const TextEvent &other)
{
    UIEvent::operator = (other);
    return *this;
}

TextEvent &TextEvent::operator = (const Event &other)
{
    Event e;
    e = other;
    if (!e.isNull() && !e.handle()->isTextInputEvent()) {
	if ( impl ) impl->deref();
	impl = 0;
    } else
	UIEvent::operator = (other);
    return *this;
}

TextEvent::~TextEvent()
{
}

void TextEvent::initTextEvent(const DOMString &typeArg,
                      bool canBubbleArg,
                      bool cancelableArg,
                      const AbstractView &viewArg,
                      const DOMString &dataArg)
{
    static_cast<TextEventImpl*>(impl)->initTextEvent(
        typeArg, canBubbleArg, cancelableArg, viewArg.handle(), dataArg);
}
// -----------------------------------------------------------------------------

KeyboardEvent::KeyboardEvent() : UIEvent()
{
}

KeyboardEvent::KeyboardEvent(const KeyboardEvent &other) : UIEvent(other)
{
}

KeyboardEvent::KeyboardEvent(const Event &other) : UIEvent()
{
    (*this)=other;
}

KeyboardEvent &KeyboardEvent::operator = (const KeyboardEvent &other)
{
    UIEvent::operator = (other);
    return *this;
}

KeyboardEvent &KeyboardEvent::operator = (const Event &other)
{
    Event e;
    e = other;
    if (!e.isNull() && !e.handle()->isKeyboardEvent()) {
	if ( impl ) impl->deref();
	impl = 0;
    } else
	UIEvent::operator = (other);
    return *this;
}

KeyboardEvent::~KeyboardEvent()
{
}

DOMString KeyboardEvent::keyIdentifier() const
{
    return static_cast<const KeyboardEventImpl*>(impl)->keyIdentifier();
}

unsigned long KeyboardEvent::keyLocation() const
{
    return static_cast<const KeyboardEventImpl*>(impl)->keyLocation();
}

bool KeyboardEvent::ctrlKey() const
{
    return static_cast<const KeyboardEventImpl*>(impl)->ctrlKey();
}

bool KeyboardEvent::shiftKey() const
{
    return static_cast<const KeyboardEventImpl*>(impl)->shiftKey();
}

bool KeyboardEvent::altKey() const
{
    return static_cast<const KeyboardEventImpl*>(impl)->altKey();
}

bool KeyboardEvent::metaKey() const
{
    return static_cast<const KeyboardEventImpl*>(impl)->metaKey();
}

bool KeyboardEvent::getModifierState(DOMString keyIdentifierArg) const
{
    return static_cast<const KeyboardEventImpl*>(impl)->getModifierState(keyIdentifierArg);
}

void KeyboardEvent::initKeyboardEvent(DOMString typeArg,
                            bool canBubbleArg,
                            bool cancelableArg,
                            AbstractView viewArg,
                            DOMString keyIdentifierArg,
                            unsigned long keyLocationArg,
                            DOMString modifiersList)
{
    static_cast<KeyboardEventImpl*>(impl)->initKeyboardEvent(typeArg,
        canBubbleArg, cancelableArg, viewArg.handle(), keyIdentifierArg, keyLocationArg, modifiersList);
}


// -----------------------------------------------------------------------------

MutationEvent::MutationEvent() : Event()
{
}

MutationEvent::MutationEvent(const MutationEvent &other) : Event(other)
{
}

MutationEvent::MutationEvent(const Event &other) : Event()
{
    (*this)=other;
}

MutationEvent::MutationEvent(MutationEventImpl *impl) : Event(impl)
{
}

MutationEvent &MutationEvent::operator = (const MutationEvent &other)
{
    Event::operator = (other);
    return *this;
}

MutationEvent &MutationEvent::operator = (const Event &other)
{
    Event e;
    e = other;
    if (!e.isNull() && !e.handle()->isMutationEvent()) {
	if ( impl ) impl->deref();
	impl = 0;
    } else
	Event::operator = (other);
    return *this;
}

MutationEvent::~MutationEvent()
{
}

Node MutationEvent::relatedNode() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MutationEventImpl*>(impl)->relatedNode();
}

DOMString MutationEvent::prevValue() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MutationEventImpl*>(impl)->prevValue();
}

DOMString MutationEvent::newValue() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MutationEventImpl*>(impl)->newValue();
}

DOMString MutationEvent::attrName() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MutationEventImpl*>(impl)->attrName();
}

unsigned short MutationEvent::attrChange() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return static_cast<MutationEventImpl*>(impl)->attrChange();
}

void MutationEvent::initMutationEvent(const DOMString &typeArg,
                                       bool canBubbleArg,
                                       bool cancelableArg,
                                       const Node &relatedNodeArg,
                                       const DOMString &prevValueArg,
                                       const DOMString &newValueArg,
                                       const DOMString &attrNameArg,
                                       unsigned short attrChangeArg)
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    static_cast<MutationEventImpl*>(impl)->initMutationEvent(typeArg,
	canBubbleArg,cancelableArg,relatedNodeArg,prevValueArg,
	newValueArg,attrNameArg,attrChangeArg);
}


