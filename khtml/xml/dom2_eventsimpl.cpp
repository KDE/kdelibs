/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *           (C) 2001 Tobias Anton (anton@stud.fbi.fh-darmstadt.de)
 *           (C) 2003 Apple Computer, Inc.
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
 */

#include "dom/dom2_views.h"

#include "xml/dom2_eventsimpl.h"
#include "xml/dom_stringimpl.h"
#include "xml/dom_nodeimpl.h"
#include "xml/dom_docimpl.h"
#include "rendering/render_layer.h"
#include "khtmlview.h"

#include <kdebug.h>

using namespace DOM;
using namespace khtml;

EventImpl::EventImpl()
{
    m_type = 0;
    m_canBubble = false;
    m_cancelable = false;

    m_propagationStopped = false;
    m_defaultPrevented = false;
    m_id = UNKNOWN_EVENT;
    m_currentTarget = 0;
    m_eventPhase = 0;
    m_target = 0;
    m_createTime = QDateTime::currentDateTime();
    m_defaultHandled = false;
}

EventImpl::EventImpl(EventId _id, bool canBubbleArg, bool cancelableArg)
{
    DOMString t = EventImpl::idToType(_id);
    m_type = t.implementation();
    if (m_type)
	m_type->ref();
    m_canBubble = canBubbleArg;
    m_cancelable = cancelableArg;

    m_propagationStopped = false;
    m_defaultPrevented = false;
    m_id = _id;
    m_currentTarget = 0;
    m_eventPhase = 0;
    m_target = 0;
    m_createTime = QDateTime::currentDateTime();
    m_defaultHandled = false;
}

EventImpl::~EventImpl()
{
    if (m_type)
        m_type->deref();
    if (m_target)
        m_target->deref();
}

void EventImpl::setTarget(NodeImpl *_target)
{
    if (m_target)
        m_target->deref();
    m_target = _target;
    if (m_target)
        m_target->ref();
}

DOMTimeStamp EventImpl::timeStamp()
{
    QDateTime epoch(QDate(1970,1,1),QTime(0,0));
    // ### kjs does not yet support long long (?) so the value wraps around
    return epoch.secsTo(m_createTime)*1000+m_createTime.time().msec();
}

void EventImpl::initEvent(const DOMString &eventTypeArg, bool canBubbleArg, bool cancelableArg)
{
    // ### ensure this is not called after we have been dispatched (also for subclasses)

    if (m_type)
	m_type->deref();

    m_type = eventTypeArg.implementation();
    if (m_type)
	m_type->ref();

    m_id = typeToId(eventTypeArg);

    m_canBubble = canBubbleArg;
    m_cancelable = cancelableArg;
}

EventImpl::EventId EventImpl::typeToId(DOMString type)
{
    if (type == "DOMFocusIn")
	return DOMFOCUSIN_EVENT;
    else if (type == "DOMFocusOut")
	return DOMFOCUSOUT_EVENT;
    else if (type == "DOMActivate")
	return DOMACTIVATE_EVENT;
    else if (type == "click")
	return CLICK_EVENT;
    else if (type == "mousedown")
	return MOUSEDOWN_EVENT;
    else if (type == "mouseup")
	return MOUSEUP_EVENT;
    else if (type == "mouseover")
	return MOUSEOVER_EVENT;
    else if (type == "mousemove")
	return MOUSEMOVE_EVENT;
    else if (type == "mouseout")
	return MOUSEOUT_EVENT;
    else if (type == "DOMSubtreeModified")
	return DOMSUBTREEMODIFIED_EVENT;
    else if (type == "DOMNodeInserted")
	return DOMNODEINSERTED_EVENT;
    else if (type == "DOMNodeRemoved")
	return DOMNODEREMOVED_EVENT;
    else if (type == "DOMNodeRemovedFromDocument")
	return DOMNODEREMOVEDFROMDOCUMENT_EVENT;
    else if (type == "DOMNodeInsertedIntoDocument")
	return DOMNODEINSERTEDINTODOCUMENT_EVENT;
    else if (type == "DOMAttrModified")
	return DOMATTRMODIFIED_EVENT;
    else if (type == "DOMCharacterDataModified")
	return DOMCHARACTERDATAMODIFIED_EVENT;
    else if (type == "load")
	return LOAD_EVENT;
    else if (type == "unload")
	return UNLOAD_EVENT;
    else if (type == "abort")
	return ABORT_EVENT;
    else if (type == "error")
	return ERROR_EVENT;
    else if (type == "select")
	return SELECT_EVENT;
    else if (type == "change")
	return CHANGE_EVENT;
    else if (type == "submit")
	return SUBMIT_EVENT;
    else if (type == "reset")
	return RESET_EVENT;
    else if (type == "focus")
	return FOCUS_EVENT;
    else if (type == "blur")
	return BLUR_EVENT;
    else if (type == "resize")
	return RESIZE_EVENT;
    else if (type == "scroll")
	return SCROLL_EVENT;
    else if ( type == "keydown" )
        return KEYDOWN_EVENT;
    else if ( type == "keyup" )
        return KEYUP_EVENT;
    else if ( type == "keypress" )
        return KEYPRESS_EVENT;
    else if ( type == "readystatechange" )
        return KHTML_READYSTATECHANGE_EVENT;
    else if ( type == "dblclick" )
        return KHTML_ECMA_DBLCLICK_EVENT;

    // ignore: KHTML_CLICK_EVENT
    return UNKNOWN_EVENT;
}

DOMString EventImpl::idToType(EventImpl::EventId id)
{
    switch (id) {
    case DOMFOCUSIN_EVENT:
        return "DOMFocusIn";
    case DOMFOCUSOUT_EVENT:
        return "DOMFocusOut";
    case DOMACTIVATE_EVENT:
        return "DOMActivate";
    case CLICK_EVENT:
        return "click";
    case MOUSEDOWN_EVENT:
        return "mousedown";
    case MOUSEUP_EVENT:
        return "mouseup";
    case MOUSEOVER_EVENT:
        return "mouseover";
    case MOUSEMOVE_EVENT:
        return "mousemove";
    case MOUSEOUT_EVENT:
        return "mouseout";
    case DOMSUBTREEMODIFIED_EVENT:
        return "DOMSubtreeModified";
    case DOMNODEINSERTED_EVENT:
        return "DOMNodeInserted";
    case DOMNODEREMOVED_EVENT:
        return "DOMNodeRemoved";
    case DOMNODEREMOVEDFROMDOCUMENT_EVENT:
        return "DOMNodeRemovedFromDocument";
    case DOMNODEINSERTEDINTODOCUMENT_EVENT:
        return "DOMNodeInsertedIntoDocument";
    case DOMATTRMODIFIED_EVENT:
        return "DOMAttrModified";
    case DOMCHARACTERDATAMODIFIED_EVENT:
        return "DOMCharacterDataModified";
    case LOAD_EVENT:
        return "load";
    case UNLOAD_EVENT:
        return "unload";
    case ABORT_EVENT:
        return "abort";
    case ERROR_EVENT:
        return "error";
    case SELECT_EVENT:
        return "select";
    case CHANGE_EVENT:
        return "change";
    case SUBMIT_EVENT:
        return "submit";
    case RESET_EVENT:
        return "reset";
    case FOCUS_EVENT:
        return "focus";
    case BLUR_EVENT:
        return "blur";
    case RESIZE_EVENT:
        return "resize";
    case SCROLL_EVENT:
        return "scroll";
    case KEYDOWN_EVENT:
        return "keydown";
    case KEYUP_EVENT:
        return "keyup";
    case KEYPRESS_EVENT:
        return "keypress";

    //khtml extensions
    case KHTML_ECMA_DBLCLICK_EVENT:
        return "dblclick";
    case KHTML_ECMA_CLICK_EVENT:
        return "click";
    case KHTML_DRAGDROP_EVENT:
        return "khtml_dragdrop";
    case KHTML_ERROR_EVENT:
        return "khtml_error";
    case KHTML_MOVE_EVENT:
        return "khtml_move";
    case KHTML_READYSTATECHANGE_EVENT:
        return "readystatechange";

    default:
        return DOMString();
        break;
    }
}

bool EventImpl::isUIEvent() const
{
    return false;
}

bool EventImpl::isMouseEvent() const
{
    return false;
}

bool EventImpl::isMutationEvent() const
{
    return false;
}

bool EventImpl::isTextEvent() const
{
    return false;
}

// -----------------------------------------------------------------------------

UIEventImpl::UIEventImpl(EventId _id, bool canBubbleArg, bool cancelableArg,
		AbstractViewImpl *viewArg, long detailArg)
		: EventImpl(_id,canBubbleArg,cancelableArg)
{
    m_view = viewArg;
    if (m_view)
        m_view->ref();
    m_detail = detailArg;
}

UIEventImpl::~UIEventImpl()
{
    if (m_view)
        m_view->deref();
}

void UIEventImpl::initUIEvent(const DOMString &typeArg,
			      bool canBubbleArg,
			      bool cancelableArg,
			      const AbstractView &viewArg,
			      long detailArg)
{
    EventImpl::initEvent(typeArg,canBubbleArg,cancelableArg);

    if (m_view)
	m_view->deref();

    m_view = viewArg.handle();
    if (m_view)
	m_view->ref();
    m_detail = detailArg;
}

bool UIEventImpl::isUIEvent() const
{
    return true;
}

// -----------------------------------------------------------------------------

MouseEventImpl::MouseEventImpl()
{
    m_screenX = 0;
    m_screenY = 0;
    m_clientX = 0;
    m_clientY = 0;
    m_pageX = 0;
    m_pageY = 0;
    m_ctrlKey = false;
    m_altKey = false;
    m_shiftKey = false;
    m_metaKey = false;
    m_button = 0;
    m_relatedTarget = 0;
    m_qevent = 0;
    m_isDoubleClick = false;
}

MouseEventImpl::MouseEventImpl(EventId _id,
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
			       QMouseEvent *qe,
                               bool isDoubleClick)
		   : UIEventImpl(_id,canBubbleArg,cancelableArg,viewArg,detailArg)
{
    m_screenX = screenXArg;
    m_screenY = screenYArg;
    m_clientX = clientXArg;
    m_clientY = clientYArg;
    m_pageX = pageXArg;
    m_pageY = pageYArg;
    m_ctrlKey = ctrlKeyArg;
    m_altKey = altKeyArg;
    m_shiftKey = shiftKeyArg;
    m_metaKey = metaKeyArg;
    m_button = buttonArg;
    m_relatedTarget = relatedTargetArg;
    if (m_relatedTarget)
	m_relatedTarget->ref();
    computeLayerPos();
    m_qevent = qe;
    m_isDoubleClick = isDoubleClick;
}

MouseEventImpl::~MouseEventImpl()
{
    if (m_relatedTarget)
	m_relatedTarget->deref();
}

void MouseEventImpl::computeLayerPos()
{
    m_layerX = m_pageX;
    m_layerY = m_pageY;

    DocumentImpl* doc = view() ? view()->document() : 0;
    if (doc) {
        khtml::RenderObject::NodeInfo renderInfo(true, false);
        doc->renderer()->layer()->nodeAtPoint(renderInfo, m_pageX, m_pageY);

        NodeImpl *node = renderInfo.innerNonSharedNode();
        while (node && !node->renderer())
            node = node->parent();

        if (node) {
            node->renderer()->enclosingLayer()->updateLayerPosition();
            for (RenderLayer* layer = node->renderer()->enclosingLayer(); layer;
                 layer = layer->parent()) {
                m_layerX -= layer->xPos();
                m_layerY -= layer->yPos();
            }
        }
    }
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

    if (m_relatedTarget)
	m_relatedTarget->deref();

    m_screenX = screenXArg;
    m_screenY = screenYArg;
    m_clientX = clientXArg;
    m_clientY = clientYArg;
    m_pageX   = clientXArg;
    m_pageY   = clientYArg;
    KHTMLView* v;
    if ( view() && view()->document() && ( v = view()->document()->view() ) ) {
        m_pageX += v->contentsX();
        m_pageY += v->contentsY();
    }
    m_ctrlKey = ctrlKeyArg;
    m_altKey = altKeyArg;
    m_shiftKey = shiftKeyArg;
    m_metaKey = metaKeyArg;
    m_button = buttonArg;
    m_relatedTarget = relatedTargetArg.handle();
    if (m_relatedTarget)
	m_relatedTarget->ref();


    // ### make this on-demand. its soo sloooow
    computeLayerPos();
    m_qevent = 0;
}

bool MouseEventImpl::isMouseEvent() const
{
    return true;
}

//---------------------------------------------------------------------------------------------

TextEventImpl::TextEventImpl()
{
    m_keyEvent = 0;
}

TextEventImpl::TextEventImpl(QKeyEvent *key, bool keypress, AbstractViewImpl *view)
  : UIEventImpl(KEYDOWN_EVENT,true,true,view,0)
{
  m_keyEvent = new QKeyEvent(key->type(), key->key(), key->ascii(), key->state(), key->text(), key->isAutoRepeat(), key->count() );
  // Events are supposed to be accepted by default in Qt!
  // This line made QLineEdit's keyevents be ignored, so they were sent to the khtmlview
  // (and e.g. space would make it scroll down)
  //m_keyEvent->ignore();

  if( keypress )
    m_id = KEYPRESS_EVENT;
  else if( key->type() == QEvent::KeyPress )
    m_id = KEYDOWN_EVENT;
  else if( key->type() == QEvent::KeyRelease )
    m_id = KEYUP_EVENT;

  m_detail = key->count();

  m_numPad = false;
  m_keyVal = key->ascii();
  m_virtKeyVal = DOM_VK_UNDEFINED;
  m_inputGenerated = true;

  switch(key->key())
  {
  case Qt::Key_Enter:
      m_numPad = true;
      /* fall through */
  case Qt::Key_Return:
      m_virtKeyVal = DOM_VK_ENTER;
      break;
  case Qt::Key_NumLock:
      m_numPad = true;
      m_virtKeyVal = DOM_VK_NUM_LOCK;
      break;
  case Qt::Key_Alt:
      m_virtKeyVal = DOM_VK_RIGHT_ALT;
      // ### DOM_VK_LEFT_ALT;
      break;
  case Qt::Key_Control:
      m_virtKeyVal = DOM_VK_LEFT_CONTROL;
      // ### DOM_VK_RIGHT_CONTROL
      break;
  case Qt::Key_Shift:
      m_virtKeyVal = DOM_VK_LEFT_SHIFT;
      // ### DOM_VK_RIGHT_SHIFT
      break;
  case Qt::Key_Meta:
      m_virtKeyVal = DOM_VK_META;
      break;
  case Qt::Key_CapsLock:
      m_virtKeyVal = DOM_VK_CAPS_LOCK;
      break;
  case Qt::Key_Delete:
      m_virtKeyVal = DOM_VK_DELETE;
      break;
  case Qt::Key_End:
      m_virtKeyVal = DOM_VK_END;
      break;
  case Qt::Key_Escape:
      m_virtKeyVal = DOM_VK_ESCAPE;
      break;
  case Qt::Key_Home:
      m_virtKeyVal = DOM_VK_HOME;
      break;
//   case Qt::Key_Insert:
//       m_virtKeyVal = DOM_VK_INSERT;
//       break;
  case Qt::Key_Pause:
      m_virtKeyVal = DOM_VK_PAUSE;
      break;
  case Qt::Key_Print:
      m_virtKeyVal = DOM_VK_PRINTSCREEN;
      break;
  case Qt::Key_ScrollLock:
      m_virtKeyVal = DOM_VK_SCROLL_LOCK;
      break;
  case Qt::Key_Left:
      m_virtKeyVal = DOM_VK_LEFT;
      break;
  case Qt::Key_Right:
      m_virtKeyVal = DOM_VK_RIGHT;
      break;
  case Qt::Key_Up:
      m_virtKeyVal = DOM_VK_UP;
      break;
  case Qt::Key_Down:
      m_virtKeyVal = DOM_VK_DOWN;
      break;
  case Qt::Key_Next:
      m_virtKeyVal = DOM_VK_PAGE_DOWN;
      break;
  case Qt::Key_Prior:
      m_virtKeyVal = DOM_VK_PAGE_UP;
      break;
  case Qt::Key_F1:
      m_virtKeyVal = DOM_VK_F1;
      break;
  case Qt::Key_F2:
      m_virtKeyVal = DOM_VK_F2;
      break;
  case Qt::Key_F3:
      m_virtKeyVal = DOM_VK_F3;
      break;
  case Qt::Key_F4:
      m_virtKeyVal = DOM_VK_F4;
      break;
  case Qt::Key_F5:
      m_virtKeyVal = DOM_VK_F5;
      break;
  case Qt::Key_F6:
      m_virtKeyVal = DOM_VK_F6;
      break;
  case Qt::Key_F7:
      m_virtKeyVal = DOM_VK_F7;
      break;
  case Qt::Key_F8:
      m_virtKeyVal = DOM_VK_F8;
      break;
  case Qt::Key_F9:
      m_virtKeyVal = DOM_VK_F9;
      break;
  case Qt::Key_F10:
      m_virtKeyVal = DOM_VK_F10;
      break;
  case Qt::Key_F11:
      m_virtKeyVal = DOM_VK_F11;
      break;
  case Qt::Key_F12:
      m_virtKeyVal = DOM_VK_F12;
      break;
  case Qt::Key_F13:
      m_virtKeyVal = DOM_VK_F13;
      break;
  case Qt::Key_F14:
      m_virtKeyVal = DOM_VK_F14;
      break;
  case Qt::Key_F15:
      m_virtKeyVal = DOM_VK_F15;
      break;
  case Qt::Key_F16:
      m_virtKeyVal = DOM_VK_F16;
      break;
  case Qt::Key_F17:
      m_virtKeyVal = DOM_VK_F17;
      break;
  case Qt::Key_F18:
      m_virtKeyVal = DOM_VK_F18;
      break;
  case Qt::Key_F19:
      m_virtKeyVal = DOM_VK_F19;
      break;
  case Qt::Key_F20:
      m_virtKeyVal = DOM_VK_F20;
      break;
  case Qt::Key_F21:
      m_virtKeyVal = DOM_VK_F21;
      break;
  case Qt::Key_F22:
      m_virtKeyVal = DOM_VK_F22;
      break;
  case Qt::Key_F23:
      m_virtKeyVal = DOM_VK_F23;
      break;
  case Qt::Key_F24:
      m_virtKeyVal = DOM_VK_F24;
      break;
  default:
      m_virtKeyVal = DOM_VK_UNDEFINED;
      break;
  }

  // m_keyVal should contain the unicode value
  // of the pressed key if available.
  if (m_virtKeyVal == DOM_VK_UNDEFINED && !key->text().isEmpty())
      m_keyVal = key->text().unicode()[0];

  //  m_numPad = ???

  // key->state returns enum ButtonState, which is ShiftButton, ControlButton and AltButton or'ed together.
  m_modifier = key->state();

  // key->text() returns the unicode sequence as a QString
  m_outputString = DOMString(key->text());
}

TextEventImpl::TextEventImpl(EventId _id,
			   bool canBubbleArg,
			   bool cancelableArg,
			   AbstractViewImpl *viewArg,
			   unsigned short detailArg,
			   DOMString &outputStringArg,
			   unsigned long keyValArg,
			   unsigned long virtKeyValArg,
			   bool inputGeneratedArg,
			   bool numPadArg)
  : UIEventImpl(_id,canBubbleArg,cancelableArg,viewArg,detailArg)
{
  m_keyEvent = 0;
  m_keyVal = keyValArg;
  m_virtKeyVal = virtKeyValArg;
  m_inputGenerated = inputGeneratedArg;
  m_outputString = outputStringArg;
  m_numPad = numPadArg;
  m_modifier = 0;
}

TextEventImpl::~TextEventImpl()
{
    delete m_keyEvent;
}

bool TextEventImpl::checkModifier(unsigned long modifierArg)
{
  return ((m_modifier & modifierArg) == modifierArg);
}

void TextEventImpl::initTextEvent(const DOMString &typeArg,
				bool canBubbleArg,
				bool cancelableArg,
				const AbstractView &viewArg,
				long detailArg,
				const DOMString &outputStringArg,
				unsigned long keyValArg,
				unsigned long virtKeyValArg,
				bool inputGeneratedArg,
				bool numPadArg)
{
  UIEventImpl::initUIEvent(typeArg, canBubbleArg, cancelableArg, viewArg, detailArg);

  m_outputString = outputStringArg;
  m_keyVal = keyValArg;
  m_virtKeyVal = virtKeyValArg;
  m_inputGenerated = inputGeneratedArg;
  m_numPad = numPadArg;
}

void TextEventImpl::initModifier(unsigned long modifierArg,
				bool valueArg)
{
  if (valueArg)
      m_modifier |= modifierArg;
  else
      m_modifier &= (modifierArg ^ 0xFFFFFFFF);
}

int TextEventImpl::keyCode() const
{
    if (!m_keyEvent)
        return 0;

    if (m_virtKeyVal != DOM_VK_UNDEFINED) {
        return m_virtKeyVal;
    } else {
        int c = charCode();
        if (c != 0) {
            return QChar(c).upper().unicode();
        } else {
            c = m_keyEvent->key();
            if (c == Qt::Key_unknown)
                kdDebug( 6020 ) << "Unknown key" << endl;
            return c;
        }
    }
}

int TextEventImpl::charCode() const
{
    if (!m_keyEvent)
        return 0;

    if (m_outputString.length() != 1)
        return 0;

    return m_outputString[0].unicode();
}


bool TextEventImpl::isTextEvent() const
{
    return true;
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

MutationEventImpl::MutationEventImpl(EventId _id,
				     bool canBubbleArg,
				     bool cancelableArg,
				     const Node &relatedNodeArg,
				     const DOMString &prevValueArg,
				     const DOMString &newValueArg,
				     const DOMString &attrNameArg,
				     unsigned short attrChangeArg)
		      : EventImpl(_id,canBubbleArg,cancelableArg)
{
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
    if (m_attrName)
	m_attrName->ref();
    m_attrChange = attrChangeArg;
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

    if (m_relatedNode)
	m_relatedNode->deref();
    if (m_prevValue)
	m_prevValue->deref();
    if (m_newValue)
	m_newValue->deref();
    if (m_attrName)
	m_attrName->deref();

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

bool MutationEventImpl::isMutationEvent() const
{
    return true;
}

