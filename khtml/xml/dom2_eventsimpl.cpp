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


#include "dom2_eventsimpl.h"
#include "dom2_views.h"
#include "dom2_viewsimpl.h"
#include "dom_string.h"
#include "dom_stringimpl.h"

using namespace DOM;

EventImpl::EventImpl()
{
    m_type = 0;
    m_canBubble = false;
    m_cancelable = false;
}

EventImpl::~EventImpl()
{
    if (m_type)
	m_type->deref();
}

DOMString EventImpl::type() const
{
    return m_type;
}

NodeImpl *EventImpl::target() const
{
    // ###
    return 0;
}

NodeImpl *EventImpl::currentTarget() const
{
    // ###
    return 0;
}

unsigned short EventImpl::eventPhase() const
{
    // ###
    return 0;
}

bool EventImpl::bubbles() const
{
    return m_canBubble; // ### is this the same as bubbles?
}

bool EventImpl::cancelable() const
{
    return m_cancelable;
}

DOMTimeStamp EventImpl::timeStamp()
{
    // ###
    return 0;
}

void EventImpl::stopPropagation()
{
    // ###
}

void EventImpl::preventDefault()
{
    // ###
}

void EventImpl::initEvent(const DOMString &eventTypeArg, bool canBubbleArg, bool cancelableArg)
{
    // ### ensure this is not called after we have been dispatched (also for subclasses)
    m_type = eventTypeArg.implementation();
    if (m_type)
	m_type->ref();
    m_canBubble = canBubbleArg;
    m_cancelable = cancelableArg;
}

// -----------------------------------------------------------------------------

UIEventImpl::UIEventImpl()
{
    m_view = 0;
    m_detail = 0;
}

UIEventImpl::~UIEventImpl()
{
    if (m_view)
	m_view->deref();
}

AbstractViewImpl *UIEventImpl::view() const
{
    return m_view;
}

long UIEventImpl::detail() const
{
    return m_detail;
}

void UIEventImpl::initUIEvent(const DOMString &typeArg,
                                 bool canBubbleArg,
                                 bool cancelableArg,
                                 const AbstractView &viewArg,
                                 long detailArg)
{
    EventImpl::initEvent(typeArg,canBubbleArg,cancelableArg);

    m_view = viewArg.handle();
    if (m_view)
	m_view->ref();
    m_detail = detailArg;
}

// -----------------------------------------------------------------------------

MouseEventImpl::MouseEventImpl()
{
    m_screenX = 0;
    m_screenY = 0;
    m_clientX = 0;
    m_clientY = 0;
    m_ctrlKey = false;
    m_altKey = false;
    m_shiftKey = false;
    m_metaKey = false;
    m_button = 0;
    m_relatedTarget = 0;
}

MouseEventImpl::~MouseEventImpl()
{
    if (m_relatedTarget)
	m_relatedTarget->deref();
}

long MouseEventImpl::screenX() const
{
    return m_screenX;
}

long MouseEventImpl::screenY() const
{
    return m_screenY;
}

long MouseEventImpl::clientX() const
{
    return m_clientX;
}

long MouseEventImpl::clientY() const
{
    return m_clientY;
}

bool MouseEventImpl::ctrlKey() const
{
    return m_ctrlKey;
}

bool MouseEventImpl::shiftKey() const
{
    return m_shiftKey;
}

bool MouseEventImpl::altKey() const
{
    return m_altKey;
}

bool MouseEventImpl::metaKey() const
{
    return m_metaKey;
}

unsigned short MouseEventImpl::button() const
{
    return m_button;
}

NodeImpl *MouseEventImpl::relatedTarget() const
{
    return m_relatedTarget;
}

void MouseEventImpl::initMouseEvent(const DOMString &typeArg,
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
    UIEventImpl::initUIEvent(typeArg,canBubbleArg,cancelableArg,viewArg,detailArg);

    m_screenX = screenXArg;
    m_screenY = screenYArg;
    m_clientX = clientXArg;
    m_clientY = clientYArg;
    m_ctrlKey = ctrlKeyArg;
    m_altKey = altKeyArg;
    m_shiftKey = shiftKeyArg;
    m_metaKey = metaKeyArg;
    m_button = buttonArg;
    m_relatedTarget = relatedTargetArg.handle();
    if (m_relatedTarget)
	m_relatedTarget->ref();
}

// -----------------------------------------------------------------------------

MutationEventImpl::MutationEventImpl()
{
    m_relatedNode = 0;
    m_prevValue = 0;
    m_newValue = 0;
    m_attrName = 0;
    m_attrChange = 0;
}

MutationEventImpl::~MutationEventImpl()
{
    if (m_relatedNode)
	m_relatedNode->deref();
    if (m_prevValue)
	m_prevValue->deref();
    if (m_newValue)
	m_newValue->deref();
    if (m_attrName)
	m_attrName->deref();
}

Node MutationEventImpl::relatedNode() const
{
    return m_relatedNode;
}

DOMString MutationEventImpl::prevValue() const
{
    return m_prevValue;
}

DOMString MutationEventImpl::newValue() const
{
    return m_newValue;
}

DOMString MutationEventImpl::attrName() const
{
    return m_attrName;
}

unsigned short MutationEventImpl::attrChange() const
{
    return m_attrChange;
}

void MutationEventImpl::initMutationEvent(const DOMString &typeArg,
                                       bool canBubbleArg,
                                       bool cancelableArg,
                                       const Node &relatedNodeArg,
                                       const DOMString &prevValueArg,
                                       const DOMString &newValueArg,
                                       const DOMString &attrNameArg,
                                       unsigned short attrChangeArg)
{
    EventImpl::initEvent(typeArg,canBubbleArg,cancelableArg);

    m_relatedNode = relatedNodeArg.handle();
    if (m_relatedNode)
	m_relatedNode->ref();
    m_prevValue = prevValueArg.implementation();
    if (m_prevValue)
	m_prevValue->ref();
    m_newValue = newValueArg.implementation();
    if (m_newValue)
	m_newValue->ref();
    m_attrName = attrNameArg.implementation();
    if (m_newValue)
	m_newValue->ref();
    m_attrChange = attrChangeArg;
}

// -----------------------------------------------------------------------------

RegisteredEventListener::RegisteredEventListener(DOMString _type, EventListener *_listener, bool _useCapture)
{
    type = _type;
    listener = _listener;
    useCapture = _useCapture;
}

bool RegisteredEventListener::operator==(const RegisteredEventListener &other)
{
    return (type == other.type &&
	    listener == other.listener &&
	    useCapture == other.useCapture);
}

