/**
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2000 Dirk Mueller (mueller@kde.org)
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
#include "render_replaced.h"
#include "render_root.h"

#include <assert.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qevent.h>
#include <qapplication.h>

#include "khtml_ext.h"
#include "khtmlview.h"
#include "xml/dom2_eventsimpl.h"
#include "khtml_part.h"
#include "xml/dom_docimpl.h" // ### remove dependency
#include <kdebug.h>

using namespace khtml;
using namespace DOM;


RenderReplaced::RenderReplaced(DOM::NodeImpl* node)
    : RenderBox(node)
{
    // init RenderObject attributes
    setReplaced(true);

    m_intrinsicWidth = 200;
    m_intrinsicHeight = 150;
}

void RenderReplaced::print( QPainter *p, int _x, int _y, int _w, int _h,
                            int _tx, int _ty)
{
    // not visible or nont even once layouted?
    if (style()->visibility() != VISIBLE || m_y <=  -500000)  return;

    _tx += m_x;
    _ty += m_y;

    if((_ty > _y + _h) || (_ty + m_height < _y)) return;

    if(hasSpecialObjects()) printBoxDecorations(p, _x, _y, _w, _h, _tx, _ty);

    // overflow: hidden
    bool clipped = false;
    if (style()->overflow()==OHIDDEN || style()->jsClipMode() ) {
        calcClip(p, _tx, _ty);
	clipped = true;
    }

    printObject(p, _x, _y, _w, _h, _tx, _ty);

    // overflow: hidden
    // restore clip region
    if ( clipped ) {
	p->restore();
    }
}

void RenderReplaced::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown());

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "RenderReplaced::calcMinMaxWidth() known=" << minMaxKnown() << endl;
#endif

    bool isPercent = false;
    int width = calcReplacedWidth(&isPercent);

    if ( isPercent ) {
        m_minWidth = 0;
        m_maxWidth = width;
    }
    else
        m_minWidth = m_maxWidth = width;

    setMinMaxKnown();
}

short RenderReplaced::lineHeight( bool ) const
{
    return height()+marginTop()+marginBottom();
}

short RenderReplaced::baselinePosition( bool ) const
{
    return height()+marginTop()+marginBottom();
}

void RenderReplaced::position(int x, int y, int, int, int, bool, bool, int)
{
    m_x = x + marginLeft();
    m_y = y + marginTop();
}

// -----------------------------------------------------------------------------

RenderWidget::RenderWidget(DOM::NodeImpl* node)
        : RenderReplaced(node)
{
    m_widget = 0;
    // a replaced element doesn't support being anonymous
    assert(node);
    m_view = node->getDocument()->view();

    // this is no real reference counting, its just there
    // to make sure that we're not deleted while we're recursed
    // in an eventFilter of the widget
    ref();
}

void RenderWidget::detach()
{
    remove();

    if ( m_widget ) {
        if ( m_view )
            m_view->removeChild( m_widget );

        m_widget->removeEventFilter( this );
        m_widget->setMouseTracking( false );
    }
    deref();
}

RenderWidget::~RenderWidget()
{
    KHTMLAssert( refCount() <= 0 );

    delete m_widget;
}

void  RenderWidget::resizeWidget( QWidget *widget, int w, int h )
{
    // ugly hack to limit the maximum size of the widget (as X11 has problems i
    h = QMIN( h, 3072 );
    w = QMIN( w, 2000 );

    if (widget->width() != w || widget->height() != h) {
        ref();
        element()->ref();
        widget->resize( w, h );
        element()->deref();
        deref();
    }
}

void RenderWidget::setQWidget(QWidget *widget)
{
    if (widget != m_widget)
    {
        if (m_widget) {
            m_widget->removeEventFilter(this);
            disconnect( m_widget, SIGNAL( destroyed()), this, SLOT( slotWidgetDestructed()));
            delete m_widget;
            m_widget = 0;
        }
        m_widget = widget;
        if (m_widget) {
            connect( m_widget, SIGNAL( destroyed()), this, SLOT( slotWidgetDestructed()));
            m_widget->installEventFilter(this);
            // if we're already layouted, apply the calculated space to the
            // widget immediately
            if (layouted()) {
		// ugly hack to limit the maximum size of the widget (as X11 has problems if it's bigger)
		resizeWidget( m_widget,
			      m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
			      m_height-borderLeft()-borderRight()-paddingLeft()-paddingRight() );
            }
            else
                setPos(xPos(), -500000);
        }
	m_view->addChild( m_widget, -500000, 0 );
    }
}

void RenderWidget::layout( )
{
    KHTMLAssert( !layouted() );
    KHTMLAssert( minMaxKnown() );
    if ( m_widget ) {
	resizeWidget( m_widget,
		      m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
		      m_height-borderLeft()-borderRight()-paddingLeft()-paddingRight() );
    }

    setLayouted();
}

void RenderWidget::slotWidgetDestructed()
{
    m_widget = 0;
}

void RenderWidget::setStyle(RenderStyle *_style)
{
    RenderReplaced::setStyle(_style);
    if(m_widget)
    {
        m_widget->setFont(style()->font());
        if (style()->visibility() != VISIBLE) {
            m_widget->hide();
        }
    }

    // do not paint background or borders for widgets
    setSpecialObjects(false);
}

void RenderWidget::printObject(QPainter* /*p*/, int, int, int, int, int _tx, int _ty)
{
    if (!m_widget || !m_view)
	return;

    if (style()->visibility() != VISIBLE) {
	m_widget->hide();
	return;
    }

    // add offset for relative positioning
    if(isRelPositioned())
	relativePositionOffset(_tx, _ty);

    int xPos = _tx+borderLeft()+paddingLeft();
    int yPos = _ty+borderTop()+paddingTop();

    int childw = m_widget->width();
    int childh = m_widget->height();
    if ( (childw == 2000 || childh == 3072) && m_widget->inherits( "KHTMLView" ) ) {
	KHTMLView *vw = static_cast<KHTMLView *>(m_widget);
	int cy = m_view->contentsY();
	int ch = m_view->visibleHeight();


	int childx = m_view->childX( m_widget );
	int childy = m_view->childY( m_widget );

	int xNew = xPos;
	int yNew = childy;

	// 	qDebug("cy=%d, ch=%d, childy=%d, childh=%d", cy, ch, childy, childh );
	if ( childh == 3072 ) {
	    if ( cy + ch > childy + childh ) {
		yNew = cy + ( ch - childh )/2;
	    } else if ( cy < childy ) {
		yNew = cy + ( ch - childh )/2;
	    }
// 	    qDebug("calculated yNew=%d", yNew);
	}
	yNew = QMIN( yNew, yPos + m_height - childh );
	yNew = QMAX( yNew, yPos );
	if ( yNew != childy || xNew != childx ) {
	    if ( vw->contentsHeight() < yNew - yPos + childh )
		vw->resizeContents( vw->contentsWidth(), yNew - yPos + childh );
	    vw->setContentsPos( xNew - xPos, yNew - yPos );
	}
	xPos = xNew;
	yPos = yNew;
    }
    m_view->addChild(m_widget, xPos, yPos );
    m_widget->show();
}

bool RenderWidget::eventFilter(QObject* /*o*/, QEvent* e)
{
    if ( !element() ) return true;

    ref();
    element()->ref();

    bool filtered = false;

    //kdDebug() << "RenderWidget::eventFilter type=" << e->type() << endl;
    switch(e->type()) {
    case QEvent::FocusOut:
       //static const char* const r[] = {"Mouse", "Tab", "Backtab", "ActiveWindow", "Popup", "Shortcut", "Other" };
        //kdDebug() << "RenderFormElement::eventFilter FocusOut widget=" << m_widget << " reason:" << r[QFocusEvent::reason()] << endl;
        // Don't count popup as a valid reason for losing the focus
        // (example: opening the options of a select combobox shouldn't emit onblur)
        if ( QFocusEvent::reason() != QFocusEvent::Popup )
       {
           //kdDebug(6000) << "RenderWidget::eventFilter captures FocusOut" << endl;
            element()->dispatchHTMLEvent(EventImpl::BLUR_EVENT,false,false);
//             if (  element()->isEditable() ) {
//                 KHTMLPartBrowserExtension *ext = static_cast<KHTMLPartBrowserExtension *>( element()->view->part()->browserExtension() );
//                 if ( ext )  ext->editableWidgetBlurred( m_widget );
//             }
//             handleFocusOut();
        }
        break;
    case QEvent::FocusIn:
        //kdDebug(6000) << "RenderWidget::eventFilter captures FocusIn" << endl;
        element()->getDocument()->setFocusNode(element());
//         if ( isEditable() ) {
//             KHTMLPartBrowserExtension *ext = static_cast<KHTMLPartBrowserExtension *>( element()->view->part()->browserExtension() );
//             if ( ext )  ext->editableWidgetFocused( m_widget );
//         }
        break;
    case QEvent::MouseButtonPress:
//       handleMousePressed(static_cast<QMouseEvent*>(e));
        break;
    case QEvent::MouseButtonRelease:
//    {
//         int absX, absY;
//         absolutePosition(absX,absY);
//         QMouseEvent* _e = static_cast<QMouseEvent*>(e);
//         m_button = _e->button();
//         m_state  = _e->state();
//         QMouseEvent e2(e->type(),QPoint(absX,absY)+_e->pos(),_e->button(),_e->state());

//         element()->dispatchMouseEvent(&e2,EventImpl::MOUSEUP_EVENT,m_clickCount);

//         if((m_mousePos - e2.pos()).manhattanLength() <= QApplication::startDragDistance()) {
//             // DOM2 Events section 1.6.2 says that a click is if the mouse was pressed
//             // and released in the "same screen location"
//             // As people usually can't click on the same pixel, we're a bit tolerant here
//             element()->dispatchMouseEvent(&e2,EventImpl::CLICK_EVENT,m_clickCount);
//         }

//         if(!isRenderButton()) {
//             // ### DOMActivate is also dispatched for thigs like selects & textareas -
//             // not sure if this is correct
//             element()->dispatchUIEvent(EventImpl::DOMACTIVATE_EVENT,m_isDoubleClick ? 2 : 1);
//             element()->dispatchMouseEvent(&e2, m_isDoubleClick ? EventImpl::KHTML_DBLCLICK_EVENT : EventImpl::KHTML_CLICK_EVENT, m_clickCount);
//             m_isDoubleClick = false;
//         }
//         else
//             // save position for slotClicked - see below -
//             m_mousePos = e2.pos();
//     }
    break;
    case QEvent::MouseButtonDblClick:
//     {
//         m_isDoubleClick = true;
//         handleMousePressed(static_cast<QMouseEvent*>(e));
//     }
    break;
    case QEvent::MouseMove:
//     {
//         int absX, absY;
//         absolutePosition(absX,absY);
//         QMouseEvent* _e = static_cast<QMouseEvent*>(e);
//         QMouseEvent e2(e->type(),QPoint(absX,absY)+_e->pos(),_e->button(),_e->state());
//         element()->dispatchMouseEvent(&e2);
//         // ### change cursor like in KHTMLView?
//     }
    break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    {
        if (!element()->dispatchKeyEvent(static_cast<QKeyEvent*>(e)))
            filtered = true;
        break;
    }
    default: break;
    };

    element()->deref();

    // stop processing if the widget gets deleted, but continue in all other cases
    if (hasOneRef())
        filtered = true;
    deref();

    return filtered;
}


#include "render_replaced.moc"

