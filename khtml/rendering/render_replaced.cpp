/**
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2000-2003 Dirk Mueller (mueller@kde.org)
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

#include "render_arena.h"

#include <assert.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qevent.h>
#include <qapplication.h>
#include <kglobalsettings.h>

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

void RenderReplaced::paint( QPainter *p, int _x, int _y, int _w, int _h,
                            int _tx, int _ty, PaintAction paintPhase)
{
    if (paintPhase != PaintActionForeground)
        return;

    // not visible or nont even once layouted?
    if (style()->visibility() != VISIBLE || m_y <=  -500000)  return;

    _tx += m_x;
    _ty += m_y;

    if((_ty > _y + _h) || (_ty + m_height < _y)) return;

    if(shouldPaintBackgroundOrBorder())
        paintBoxDecorations(p, _x, _y, _w, _h, _tx, _ty);

    paintObject(p, _x, _y, _w, _h, _tx, _ty, paintPhase);
}

void RenderReplaced::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown());

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "RenderReplaced::calcMinMaxWidth() known=" << minMaxKnown() << endl;
#endif

    int width = calcReplacedWidth();

    if (!isWidget())
        width += paddingLeft() + paddingRight() + borderLeft() + borderRight();

    if ( style()->width().isPercent() || style()->height().isPercent() ) {
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

void RenderWidget::detach(RenderArena* renderArena)
{
    kdDebug( 6040 ) << "RenderWidget::detach( " << this << " )" << endl;
    remove();

    if ( m_widget ) {
        if ( m_view ) {
            m_view->setWidgetVisible(this, false);
            m_view->removeChild( m_widget );
        }

        m_widget->removeEventFilter( this );
        m_widget->setMouseTracking( false );
    }

    deref(renderArena);
}

RenderWidget::~RenderWidget()
{
    KHTMLAssert( refCount() <= 0 );

    if(m_widget) {
        m_widget->hide();
        m_widget->deleteLater();
    }
}

class QWidgetResizeEvent : public QEvent
{
public:
    enum { Type = 0xfeedabee };
    QWidgetResizeEvent( int _w,  int _h ) :
	QEvent( (QEvent::Type)Type ),  w( _w ), h( _h ) {}
    int w;
    int h;
};

void  RenderWidget::resizeWidget( int w, int h )
{
    // ugly hack to limit the maximum size of the widget (as X11 has problems i
    h = QMIN( h, 3072 );
    w = QMIN( w, 2000 );

    if (m_widget->width() != w || m_widget->height() != h) {
        RenderArena *arena = ref();
        element()->ref();
	QApplication::postEvent( this, new QWidgetResizeEvent( w, h ) );
        element()->deref();
        deref( arena );
    }
}

bool RenderWidget::event( QEvent *e )
{
    if ( m_widget && (e->type() == (int)QWidgetResizeEvent::Type) ) {
	QWidgetResizeEvent *re = static_cast<QWidgetResizeEvent *>(e);
	m_widget->resize( re->w,  re->h );
    }
    return true;
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
            if (m_widget->focusPolicy() > QWidget::StrongFocus)
                m_widget->setFocusPolicy(QWidget::StrongFocus);
            // if we're already layouted, apply the calculated space to the
            // widget immediately
            if (layouted()) {
		// ugly hack to limit the maximum size of the widget (as X11 has problems if it's bigger)
		resizeWidget( m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
			      m_height-borderTop()-borderBottom()-paddingTop()-paddingBottom() );
            }
            else
                setPos(xPos(), -500000);
        }
	if (m_view) {
	    m_view->setWidgetVisible(this, false);
	    m_view->addChild( m_widget, 0, -500000);
	}
    }
}

void RenderWidget::layout( )
{
    KHTMLAssert( !layouted() );
    KHTMLAssert( minMaxKnown() );
    if ( m_widget ) {
	resizeWidget( m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
		      m_height-borderTop()-borderBottom()-paddingTop()-paddingBottom() );
    }

    setLayouted();
}

void RenderWidget::updateFromElement()
{
    if (m_widget) {
        QColor color = style()->color();
        QColor backgroundColor = style()->backgroundColor();

        if ( color.isValid() || backgroundColor.isValid() ) {
            QPalette pal(QApplication::palette(m_widget));

            int contrast_ = KGlobalSettings::contrast();
            int highlightVal = 100 + (2*contrast_+4)*16/10;
            int lowlightVal = 100 + (2*contrast_+4)*10;

            if (backgroundColor.isValid()) {
                for ( int i = 0; i < QPalette::NColorGroups; i++ ) {
                    pal.setColor( (QPalette::ColorGroup)i, QColorGroup::Background, backgroundColor );
                    pal.setColor( (QPalette::ColorGroup)i, QColorGroup::Light, backgroundColor.light(highlightVal) );
                    pal.setColor( (QPalette::ColorGroup)i, QColorGroup::Dark, backgroundColor.dark(lowlightVal) );
                    pal.setColor( (QPalette::ColorGroup)i, QColorGroup::Mid, backgroundColor.dark(120) );
                    pal.setColor( (QPalette::ColorGroup)i, QColorGroup::Midlight, backgroundColor.light(110) );
                    pal.setColor( (QPalette::ColorGroup)i, QColorGroup::Button, backgroundColor );
                    pal.setColor( (QPalette::ColorGroup)i, QColorGroup::Base, backgroundColor );
	    }
            }
            if ( color.isValid() ) {
                struct ColorSet {
                    QPalette::ColorGroup cg;
                    QColorGroup::ColorRole cr;
                };
                const struct ColorSet toSet [] = {
                    { QPalette::Active, QColorGroup::Foreground },
                    { QPalette::Active, QColorGroup::ButtonText },
                    { QPalette::Active, QColorGroup::Text },
                    { QPalette::Inactive, QColorGroup::Foreground },
                    { QPalette::Inactive, QColorGroup::ButtonText },
                    { QPalette::Inactive, QColorGroup::Text },
                    { QPalette::Disabled,QColorGroup::ButtonText },
                    { QPalette::NColorGroups, QColorGroup::NColorRoles },
                };
                const ColorSet *set = toSet;
                while( set->cg != QPalette::NColorGroups ) {
                    pal.setColor( set->cg, set->cr, color );
                    ++set;
                }

                QColor disfg = color;
                int h, s, v;
                disfg.hsv( &h, &s, &v );
                if (v > 128)
                    // dark bg, light fg - need a darker disabled fg
                    disfg = disfg.dark(lowlightVal);
                else if (disfg != Qt::black)
                    // light bg, dark fg - need a lighter disabled fg - but only if !black
                    disfg = disfg.light(highlightVal);
                else
                    // black fg - use darkgrey disabled fg
                    disfg = Qt::darkGray;
                pal.setColor(QPalette::Disabled,QColorGroup::Foreground,disfg);
            }

            m_widget->setPalette(pal);
        }
        else
            m_widget->unsetPalette();
    }

    RenderReplaced::updateFromElement();
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
    setShouldPaintBackgroundOrBorder(false);
}

void RenderWidget::paintObject(QPainter* /*p*/, int, int, int, int, int _tx, int _ty,
			       PaintAction paintPhase)
{
    if (!m_widget || !m_view || paintPhase != PaintActionForeground)
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
    m_view->setWidgetVisible(this, true);
    m_view->addChild(m_widget, xPos, yPos );
    m_widget->show();
}

bool RenderWidget::eventFilter(QObject* /*o*/, QEvent* e)
{
    if ( !element() ) return true;

    RenderArena *arena = ref();
    element()->ref();

    bool filtered = false;

    //kdDebug() << "RenderWidget::eventFilter type=" << e->type() << endl;
    switch(e->type()) {
    case QEvent::FocusOut:
        // Don't count popup as a valid reason for losing the focus
        // (example: opening the options of a select combobox shouldn't emit onblur)
        if ( QFocusEvent::reason() != QFocusEvent::Popup )
            handleFocusOut();
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
    case QEvent::MouseButtonRelease:
    break;
    case QEvent::MouseButtonDblClick:
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
    case QEvent::Wheel:
        // don't allow the widget to react to wheel event unless its
        // currently focused. this avoids accidentally changing a select box
        // or something while wheeling a webpage.
        if (qApp->focusWidget() != m_widget &&
            m_widget->focusPolicy() <= QWidget::StrongFocus)  {
            static_cast<QWheelEvent*>(e)->ignore();
	    if (m_view)
		QApplication::sendEvent(m_view, e);
            filtered = true;
        }
    break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        if (!element()->dispatchKeyEvent(static_cast<QKeyEvent*>(e)))
            filtered = true;
        break;
    default: break;
    };

    element()->deref();

    // stop processing if the widget gets deleted, but continue in all other cases
    if (hasOneRef())
        filtered = true;
    deref( arena );

    return filtered;
}

void RenderWidget::deref(RenderArena *arena)
{
    if (_ref) _ref--;
//     qDebug( "deref(%p): width get count is %d", this, _ref);
    if (!_ref)
        arenaDelete(arena);
}


#include "render_replaced.moc"

