/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "khtml_events.h"
#include "rendering/render_object.h"
#include "xml/dom_nodeimpl.h"
#include "xml/dom_position.h"

using namespace khtml;
using namespace DOM;

class khtml::MouseEvent::MouseEventPrivate
{
};

khtml::MouseEvent::MouseEvent( const char *name, QMouseEvent *qmouseEvent, int x, int y,
                               const DOM::DOMString &url, const DOM::DOMString& target,
	                const DOM::Node &innerNode )
: KParts::Event( name ), m_qmouseEvent( qmouseEvent ), m_x( x ), m_y( y ),
  m_url( url ), m_target(target), m_innerNode( innerNode )
{
  d = 0;
  if (innerNode.handle() && innerNode.handle()->renderer())
      innerNode.handle()->renderer()->absolutePosition(m_nodeAbsX, m_nodeAbsY);
}

khtml::MouseEvent::~MouseEvent()
{
  delete d;
}

long khtml::MouseEvent::offset() const
{
    Position pos;
    if (innerNode().handle()) {
        // FIXME: Shouldn't be necessary to skip text nodes.
        DOM::Node inner = innerNode();
        if (inner.nodeType() == Node::TEXT_NODE)
            inner = inner.parentNode();
        pos = inner.handle()->positionForCoordinates(m_x, m_y).position();
    }
    return pos.offset();
}

const char *khtml::MousePressEvent::s_strMousePressEvent = "khtml/Events/MousePressEvent";

const char *khtml::MouseDoubleClickEvent::s_strMouseDoubleClickEvent = "khtml/Events/MouseDoubleClickEvent";

const char *khtml::MouseMoveEvent::s_strMouseMoveEvent = "khtml/Events/MouseMoveEvent";

const char *khtml::MouseReleaseEvent::s_strMouseReleaseEvent = "khtml/Events/MouseReleaseEvent";

const char *khtml::DrawContentsEvent::s_strDrawContentsEvent = "khtml/Events/DrawContentsEvent";

class khtml::DrawContentsEvent::DrawContentsEventPrivate
{
public:
  DrawContentsEventPrivate()
  {
  }
  ~DrawContentsEventPrivate()
  {
  }
};

khtml::DrawContentsEvent::DrawContentsEvent( QPainter *painter, int clipx, int clipy, int clipw, int cliph )
  : KParts::Event( s_strDrawContentsEvent ), m_painter( painter ), m_clipx( clipx ), m_clipy( clipy ),
    m_clipw( clipw ), m_cliph( cliph )
{
  d = new DrawContentsEventPrivate;
}

khtml::DrawContentsEvent::~DrawContentsEvent()
{
  delete d;
}

