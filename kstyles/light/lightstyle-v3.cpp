/*
  Copyright (c) 2000-2001 Trolltech AS (info@trolltech.com)

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#include "lightstyle-v3.h"

#include "qmenubar.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qpalette.h"
#include "qframe.h"
#include "qpushbutton.h"
#include "qdrawutil.h"
#include "qscrollbar.h"
#include "qtabbar.h"
#include "qguardedptr.h"
#include "qlayout.h"
#include "qlineedit.h"
#include "qimage.h"
#include "qcombobox.h"
#include "qslider.h"
#include "qstylefactory.h"


// The Light Style, 3rd revision

LightStyleV3::LightStyleV3()
    : QCommonStyle()
{
    basestyle = QStyleFactory::create( "Windows" );
    if ( ! basestyle )
	basestyle = QStyleFactory::create( QStyleFactory::keys().first() );
    if ( ! basestyle )
	qFatal( "LightStyle: couldn't find a basestyle!" );
}

LightStyleV3::~LightStyleV3()
{
    delete basestyle;
}

void LightStyleV3::polishPopupMenu( QPopupMenu * )
{
    // empty to satisy pure virtual requirements
}

/*
  A LightBevel looks like this:

  EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
  ESSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSE
  ESBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBSE
  ESBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBSE
  ESSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSE
  EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE

  where:
      E is the sunken etching ( optional, not drawn by default )
      S is the border (optional, drawn by default )
      B is the bevel (draw with the line width, minus the width of
                      the etching and border )
      F is the fill ( optional, not drawn by default )
*/
static void drawLightEtch( QPainter *p,
			   const QRect &rect,
			   const QColor &color,
			   bool sunken )
{
    QPointArray pts( 4 );

    pts.setPoint( 0, rect.left(),     rect.bottom() - 1);
    pts.setPoint( 1, rect.left(),     rect.top() );
    pts.setPoint( 2, rect.left() + 1, rect.top() );
    pts.setPoint( 3, rect.right(),    rect.top() );
    p->setPen( sunken ? color.dark( 115 ) : color.light( 115 ) );
    p->drawLineSegments( pts );

    pts.setPoint( 0, rect.left(),     rect.bottom() );
    pts.setPoint( 1, rect.right(),    rect.bottom() );
    pts.setPoint( 2, rect.right(),    rect.bottom() - 1 );
    pts.setPoint( 3, rect.right(),    rect.top() + 1 );
    p->setPen( sunken ? color.light( 115 ) : color.dark( 115 ) );
    p->drawLineSegments( pts );
}

static void drawLightBevel( QPainter *p,
			    const QRect &rect,
			    const QColorGroup &cg,
			    QStyle::SFlags flags,
 			    int linewidth,
			    bool etch = false,       // light sunken bevel around border
			    bool border = true,      // rectangle around bevel
			    const QBrush *fill = 0 ) // contents fill
{
    QRect br = rect;
    bool bevel = ( flags & ( QStyle::Style_Down | QStyle::Style_On |
			     QStyle::Style_Sunken | QStyle::Style_Raised ) );
    bool sunken = (flags & (QStyle::Style_Down | QStyle::Style_On |
			    QStyle::Style_Sunken));

    if ( etch && linewidth > 0 ) {
	drawLightEtch( p, br, cg.background(), true );
	linewidth--;
	br.addCoords( 1, 1, -1, -1 );
    }

    if ( ! br.isValid() )
	return;
    if ( border && linewidth > 0 ) {
	p->setPen( cg.dark() );
	p->drawRect( br );
	linewidth--;
	br.addCoords( 1, 1, -1, -1 );
    }

    if ( ! br.isValid() )
	return;
    if ( bevel && linewidth > 0 ) {
	// draw a bevel
	int x, y, w, h;
	br.rect( &x, &y, &w, &h );

	// copied form qDrawShadePanel - just changed the highlight colors...
	QPointArray a( 4*linewidth );
	if ( sunken )
	    p->setPen( border ? cg.mid() : cg.dark() );
	else
	    p->setPen( cg.light() );
	int x1, y1, x2, y2;
	int i;
	int n = 0;
	x1 = x;
	y1 = y2 = y;
	x2 = x+w-2;
	for ( i=0; i<linewidth; i++ ) {		// top shadow
	    a.setPoint( n++, x1, y1++ );
	    a.setPoint( n++, x2--, y2++ );
	}
	x2 = x1;
	y1 = y+h-2;
	for ( i=0; i<linewidth; i++ ) {		// left shadow
	    a.setPoint( n++, x1++, y1 );
	    a.setPoint( n++, x2++, y2-- );
	}
	p->drawLineSegments( a );
	n = 0;
	if ( sunken )
	    p->setPen( cg.light() );
	else
	    p->setPen( border ? cg.mid() : cg.dark() );
	x1 = x;
	y1 = y2 = y+h-1;
	x2 = x+w-1;
	for ( i=0; i<linewidth; i++ ) {		// bottom shadow
	    a.setPoint( n++, x1++, y1-- );
	    a.setPoint( n++, x2, y2-- );
	}
	x1 = x2;
	y1 = y;
	y2 = y+h-linewidth-1;
	for ( i=0; i<linewidth; i++ ) {		// right shadow
	    a.setPoint( n++, x1--, y1++ );
	    a.setPoint( n++, x2--, y2 );
	}
	p->drawLineSegments( a );

	br.addCoords( linewidth, linewidth, -linewidth, -linewidth );
    }

    // fill
    if ( fill )
	p->fillRect( br, *fill );
}

void LightStyleV3::drawPrimitive( PrimitiveElement pe,
				QPainter *p,
				const QRect &r,
				const QColorGroup &cg,
				SFlags flags,
				const QStyleOption &data ) const
{
    QRect br = r;
    const QBrush *fill = 0;

    switch (pe) {
    case PE_HeaderSection:
	// don't draw any headers sunken
	flags = ((flags | Style_Sunken) ^ Style_Sunken) | Style_Raised;

	p->setPen( cg.background() );
	// hard border at the bottom/right of the header
	if ( flags & Style_Horizontal ) {
	    p->drawLine( br.bottomLeft(), br.bottomRight() );
	    br.addCoords( 0, 0, 0, -1 );
	} else {
	    p->drawLine( br.topRight(), br.bottomRight() );
	    br.addCoords( 0, 0, -1, 0 );
	}

	// draw the header ( just an etching )
	if ( ! br.isValid() )
	    break;
	drawLightEtch( p, br, ( ( flags & Style_Down ) ? 
                                cg.midlight() : cg.button() ), 
                       ( flags & Style_Down ) );
	br.addCoords( 1, 1, -1, -1 );

	// fill the header
	if ( ! br.isValid() )
	    break;
	p->fillRect( br, cg.brush( ( flags & Style_Down ) ?
				   QColorGroup::Midlight : QColorGroup::Button ) );

	// the taskbuttons in kicker seem to allow the style to set the pencolor
	// here, which will be used to draw the text for focused window buttons...
	// how utterly silly
	p->setPen( cg.buttonText() );
	break;

    case PE_ButtonCommand:
	{
	    QRect br = r;

	    if (flags & QStyle::Style_Enabled) {
		if (flags & (QStyle::Style_Down |
			     QStyle::Style_On |
			     QStyle::Style_Sunken))
		    fill = &cg.brush(QColorGroup::Midlight);
		else
		    fill = &cg.brush(QColorGroup::Button);
	    } else
		fill = &cg.brush(QColorGroup::Background);

	    bool etch = true;
	    if ( flags & Style_ButtonDefault ) {
		etch = false;
		br.addCoords( 1, 1, -1, -1 );
	    }
	    drawLightBevel( p, br, cg, flags,
			    pixelMetric( PM_DefaultFrameWidth ) + ( etch ? 1 : 0 ),
			    etch, true, fill );
	    break;
	}

    case PE_ButtonBevel:
    case PE_ButtonTool:
	if (flags & QStyle::Style_Enabled) {
	    if (flags & (QStyle::Style_Down |
			 QStyle::Style_On |
			 QStyle::Style_Sunken))
		fill = &cg.brush(QColorGroup::Midlight);
	    else
		fill = &cg.brush(QColorGroup::Button);
	} else
	    fill = &cg.brush(QColorGroup::Background);
	drawLightBevel( p, r, cg, flags, pixelMetric( PM_DefaultFrameWidth ),
			false, true, fill );
	break;

    case PE_ButtonDropDown:
	{
	    QBrush thefill;
	    bool sunken =
		(flags & (QStyle::Style_Down | QStyle::Style_On | QStyle::Style_Sunken));

	    if (flags & QStyle::Style_Enabled) {
		if (sunken)
		    thefill = cg.brush(QColorGroup::Midlight);
		else
		    thefill = cg.brush(QColorGroup::Button);
	    } else
		thefill = cg.brush(QColorGroup::Background);

	    p->setPen( cg.dark() );
	    p->drawLine(r.topLeft(),     r.topRight());
	    p->drawLine(r.topRight(),    r.bottomRight());
	    p->drawLine(r.bottomRight(), r.bottomLeft());

	    if (flags & (QStyle::Style_Down | QStyle::Style_On |
			 QStyle::Style_Sunken | QStyle::Style_Raised)) {
		// button bevel
		if (sunken)
		    p->setPen(cg.mid());
		else
		    p->setPen(cg.light());

		p->drawLine(r.x(), r.y() + 2,
			    r.x(), r.y() + r.height() - 3); // left
		p->drawLine(r.x(), r.y() + 1,
			    r.x() + r.width() - 2, r.y() + 1); // top

		if (sunken)
		    p->setPen(cg.light());
		else
		    p->setPen(cg.mid());

		p->drawLine(r.x() + r.width() - 2, r.y() + 2,
			    r.x() + r.width() - 2, r.y() + r.height() - 3); // right
		p->drawLine(r.x() + 1, r.y() + r.height() - 2,
			    r.x() + r.width() - 2, r.y() + r.height() - 2); // bottom
	    }

	    p->fillRect(r.x() + 1, r.y() + 2, r.width() - 3, r.height() - 4, thefill);
	    break;
	}

    case PE_ButtonDefault:
	p->setPen( cg.shadow() );
	p->drawRect( r );
	break;

    case PE_Indicator:
	const QBrush *fill;
	if (! (flags & Style_Enabled))
	    fill = &cg.brush(QColorGroup::Background);
	else if (flags & Style_Down)
	    fill = &cg.brush(QColorGroup::Mid);
	else
	    fill = &cg.brush(QColorGroup::Base);
	drawLightBevel( p, r, cg, flags | Style_Sunken, 2, true, true, fill );

	p->setPen(cg.text());
	if (flags & Style_NoChange) {
	    p->drawLine(r.x() + 3, r.y() + r.height() / 2,
			r.x() + r.width() - 4, r.y() + r.height() / 2);
	    p->drawLine(r.x() + 3, r.y() + 1 + r.height() / 2,
			r.x() + r.width() - 4, r.y() + 1 + r.height() / 2);
	    p->drawLine(r.x() + 3, r.y() - 1 + r.height() / 2,
			r.x() + r.width() - 4, r.y() - 1 + r.height() / 2);
	} else if (flags & Style_On) {
	    p->drawLine(r.x() + 4, r.y() + 3,
			r.x() + r.width() - 4, r.y() + r.height() - 5);
	    p->drawLine(r.x() + 3, r.y() + 3,
			r.x() + r.width() - 4, r.y() + r.height() - 4);
	    p->drawLine(r.x() + 3, r.y() + 4,
			r.x() + r.width() - 5, r.y() + r.height() - 4);
	    p->drawLine(r.x() + 3, r.y() + r.height() - 5,
			r.x() + r.width() - 5, r.y() + 3);
	    p->drawLine(r.x() + 3, r.y() + r.height() - 4,
			r.x() + r.width() - 4, r.y() + 3);
	    p->drawLine(r.x() + 4, r.y() + r.height() - 4,
			r.x() + r.width() - 4, r.y() + 4);
	}

	break;

    case PE_ExclusiveIndicator:
	{
	    QRect br = r, // bevel rect
		  lr = r, // outline rect
		  cr = r, // contents rect
		  ir = r; // indicator rect
	    lr.addCoords( 1, 1, -1, -1 );
	    cr.addCoords( 2, 2, -2, -2 );
	    ir.addCoords( 3, 3, -3, -3 );

	    p->fillRect( r, cg.brush( QColorGroup::Background ) );

	    p->setPen( flags & Style_Down ? cg.mid() :
		       ( flags & Style_Enabled ? cg.base() : cg.background() ) );
	    p->setBrush( flags & Style_Down ? cg.mid() :
			 ( flags & Style_Enabled ? cg.base() : cg.background() ) );
	    p->drawEllipse( lr );

	    p->setPen( cg.background().dark( 115 ) );
	    p->drawArc( br, 45*16, 180*16 );
	    p->setPen( cg.background().light( 115 ) );
	    p->drawArc( br, 235*16, 180*16 );

	    p->setPen( cg.dark() );
	    p->drawArc( lr, 0, 16*360 );

	    if ( flags & Style_On ) {
		p->setPen( flags & Style_Down ? cg.mid() :
			   ( flags & Style_Enabled ? cg.base() : cg.background() ) );
		p->setBrush( cg.text() );
		p->drawEllipse( ir );
	    }

	    break;
	}

    case PE_DockWindowHandle:
	{
	    QString title;
	    bool drawTitle = FALSE;
	    if ( p && p->device()->devType() == QInternal::Widget ) {
		QWidget *w = (QWidget *) p->device();
		QWidget *p = w->parentWidget();
		if (p->inherits("QDockWindow") && ! p->inherits("QToolBar")) {
		    drawTitle = TRUE;
		    title = p->caption();
		}
	    }

	    flags |= Style_Raised;
	    if (flags & Style_Horizontal) {
		if (drawTitle) {
		    QPixmap pm(r.height(), r.width());
		    QPainter p2(&pm);
		    p2.fillRect(0, 0, pm.width(), pm.height(),
				cg.brush(QColorGroup::Highlight));
		    p2.setPen(cg.highlightedText());
		    p2.drawText(0, 0, pm.width(), pm.height(), AlignCenter, title);
		    p2.end();

		    QWMatrix m;
		    m.rotate(270.0);
		    pm = pm.xForm(m);
		    p->drawPixmap(r.x(), r.y(), pm);
		} else {
		    for ( int i = r.left() - 1; i < r.right(); i += 3 ) {
			p->setPen( cg.midlight() );
			p->drawLine( i, r.top(), i, r.bottom() );
			p->setPen( cg.background() );
			p->drawLine( i + 1, r.top(), i, r.bottom() );
			p->setPen( cg.mid() );
			p->drawLine( i + 2, r.top(), i + 2, r.bottom() );
		    }
		}
	    } else {
		if (drawTitle) {
		    p->fillRect(r, cg.brush(QColorGroup::Highlight));
		    p->setPen(cg.highlightedText());
		    p->drawText(r, AlignCenter, title);
		} else {
		    for ( int i = r.top() - 1; i < r.bottom(); i += 3 ) {
			p->setPen( cg.midlight() );
			p->drawLine( r.left(), i, r.right(), i );
			p->setPen( cg.background() );
			p->drawLine( r.left(), i + 1, r.right(), i );
			p->setPen( cg.mid() );
			p->drawLine( r.left(), i + 2, r.right(), i + 2 );
		    }

		}
	    }
	    break;
	}

    case PE_DockWindowSeparator:
	{
	    if (flags & Style_Horizontal) {
		int hw = r.width() / 2;
		p->setPen( cg.mid() );
		p->drawLine( hw,     r.top() + 6, hw,     r.bottom() - 6 );
		p->setPen( cg.light() );
		p->drawLine( hw + 1, r.top() + 6, hw + 1, r.bottom() - 6 );
	    } else {
		int hh = r.height() / 2;
		p->setPen( cg.mid() );
		p->drawLine( r.left() + 6, hh,     r.right() - 6, hh     );
		p->setPen( cg.light() );
		p->drawLine( r.left() + 6, hh + 1, r.right() - 6, hh + 1 );
	    }
	    break;
	}

    case PE_Splitter:
	if (flags & Style_Horizontal)
	    flags &= ~Style_Horizontal;
	else
	    flags |= Style_Horizontal;
	// fall through intended

    case PE_DockWindowResizeHandle:
	{
	    QRect br = r;

	    p->setPen( cg.shadow() );
	    p->drawRect( br );

	    br.addCoords( 1, 1, -1, -1 );

	    if ( ! br.isValid() )
		break;
	    p->setPen( cg.light() );
	    p->drawLine( br.left(), br.top(), br.right() - 1, br.top() );
	    p->drawLine( br.left(), br.top() + 1, br.left(), br.bottom() );
	    p->setPen( cg.mid() );
	    p->drawLine( br.bottomLeft(), br.bottomRight() );
	    p->drawLine( br.right(), br.top(), br.right(), br.bottom() - 1 );

	    br.addCoords( 1, 1, -1, -1 );

	    if ( ! br.isValid() )
		break;
	    p->fillRect( br, cg.brush( QColorGroup::Button ) );
	    break;
	}

    case PE_PanelPopup:
	drawLightBevel( p, r, cg, flags,
			( data.isDefault() ? pixelMetric(PM_DefaultFrameWidth) :
			  data.lineWidth() ), false, true );
	break;

    case PE_Panel:
    case PE_PanelLineEdit:
    case PE_PanelTabWidget:
    case PE_WindowFrame:
	{
	    QRect br = r;

	    int cover = 0;
	    int reallw = ( data.isDefault() ?
			   pixelMetric( PM_DefaultFrameWidth ) : data.lineWidth() );
	    cover = reallw - 1;

	    if ( ! ( flags & Style_Sunken ) )
		flags |= Style_Raised;
	    drawLightBevel( p, br, cg, flags, 1, false, false );
	    br.addCoords( 1, 1, -1, -1 );

	    while ( cover-- > 0 ) {
		QPointArray pts( 8 );
		pts.setPoint( 0, br.left(),     br.bottom() - 1);
		pts.setPoint( 1, br.left(),     br.top() );
		pts.setPoint( 2, br.left() + 1, br.top() );
		pts.setPoint( 3, br.right(),    br.top() );
		pts.setPoint( 4, br.left(),     br.bottom() );
		pts.setPoint( 5, br.right(),    br.bottom() );
		pts.setPoint( 6, br.right(),    br.bottom() - 1 );
		pts.setPoint( 7, br.right(),    br.top() + 1 );
		p->setPen( cg.background() );
		p->drawLineSegments( pts );

		br.addCoords( 1, 1, -1, -1 );
	    }
	    break;
	}

    case PE_PanelDockWindow:
	drawLightBevel( p, r, cg, flags, ( data.isDefault() ?
					   pixelMetric(PM_DefaultFrameWidth) :
					   data.lineWidth() ), false, false,
			&cg.brush( QColorGroup::Button ) );
	break;

    case PE_PanelMenuBar:
	drawLightBevel( p, r, cg, flags, ( data.isDefault() ?
					   pixelMetric(PM_MenuBarFrameWidth) :
					   data.lineWidth() ), false, false,
			&cg.brush( QColorGroup::Button ) );
	break;

    case PE_ScrollBarSubLine:
	{
	    QRect br = r;
	    PrimitiveElement pe;

	    p->setPen( cg.background() );
	    if (flags & Style_Horizontal) {
		pe = PE_ArrowLeft;
		p->drawLine( br.topLeft(), br.topRight() );
		br.addCoords( 0, 1, 0, 0 );
	    } else {
		pe = PE_ArrowUp;
		p->drawLine( br.topLeft(), br.bottomLeft() );
		br.addCoords( 1, 0, 0, 0 );
	    }

	    if ( ! br.isValid() )
		break;
	    drawLightEtch( p, br, cg.button(), false );
	    br.addCoords( 1, 1, -1, -1 );

	    if ( ! br.isValid() )
		break;
	    p->fillRect( br, cg.brush( ( flags & Style_Down ) ?
				       QColorGroup::Midlight :
				       QColorGroup::Button ) );
	    br.addCoords( 2, 2, -2, -2 );

	    if ( ! br.isValid() )
		break;
	    drawPrimitive( pe, p, br, cg, flags );
	    break;
	}

    case PE_ScrollBarAddLine:
	{
	    QRect br = r;
	    PrimitiveElement pe;

	    p->setPen( cg.background() );
	    if (flags & Style_Horizontal) {
		pe = PE_ArrowRight;
		p->drawLine( br.topLeft(), br.topRight() );
		br.addCoords( 0, 1, 0, 0 );
	    } else {
		pe = PE_ArrowDown;
		p->drawLine( br.topLeft(), br.bottomLeft() );
		br.addCoords( 1, 0, 0, 0 );
	    }

	    if ( ! br.isValid() )
		break;
	    drawLightEtch( p, br, cg.button(), false );
	    br.addCoords( 1, 1, -1, -1 );

	    if ( ! br.isValid() )
		break;
	    p->fillRect( br, cg.brush( ( flags & Style_Down ) ?
				       QColorGroup::Midlight :
				       QColorGroup::Button ) );
	    br.addCoords( 2, 2, -2, -2 );

	    if ( ! br.isValid() )
		break;
	    drawPrimitive( pe, p, br, cg, flags );
	    break;
	}

    case PE_ScrollBarSubPage:
	{
	    QRect br = r;

	    p->setPen( cg.background() );
	    if (flags & Style_Horizontal) {
		p->drawLine( br.topLeft(), br.topRight() );
		br.addCoords( 0, 1, 0, 0 );
	    } else {
		p->drawLine( br.topLeft(), br.bottomLeft() );
		br.addCoords( 1, 0, 0, 0 );
	    }

	    if ( ! br.isValid() )
		break;
	    drawLightEtch( p, br, cg.button(), false );
	    br.addCoords( 1, 1, -1, -1 );

	    if ( ! br.isValid() )
		break;
	    p->fillRect( br, cg.brush( ( flags & Style_Down ) ?
				       QColorGroup::Midlight :
				       QColorGroup::Button ) );
	    break;
	}

    case PE_ScrollBarAddPage:
	{
	    QRect br = r;

	    p->setPen( cg.background() );
	    if (flags & Style_Horizontal) {
		p->drawLine( br.topLeft(), br.topRight() );
		br.addCoords( 0, 1, 0, 0 );
	    } else {
		p->drawLine( br.topLeft(), br.bottomLeft() );
		br.addCoords( 1, 0, 0, 0 );
	    }

	    if ( ! br.isValid() )
		break;
	    drawLightEtch( p, br, cg.button(), false );
	    br.addCoords( 1, 1, -1, -1 );

	    if ( ! br.isValid() )
		break;
	    p->fillRect( br, cg.brush( ( flags & Style_Down ) ?
				       QColorGroup::Midlight :
				       QColorGroup::Button ) );
	    break;
	}

    case PE_ScrollBarSlider:
	{
	    QRect br = r;

	    p->setPen( cg.background() );
	    if (flags & Style_Horizontal) {
		p->drawLine( br.topLeft(), br.topRight() );
		br.addCoords( 0, 1, 0, 0 );
	    } else {
		p->drawLine( br.topLeft(), br.bottomLeft() );
		br.addCoords( 1, 0, 0, 0 );
	    }

	    if ( ! br.isValid() )
		break;
	    p->setPen( cg.highlight().light() );
	    p->drawLine( br.topLeft(), br.topRight() );
	    p->drawLine( br.left(), br.top() + 1, br.left(), br.bottom() - 1 );

	    p->setPen( cg.highlight().dark() );
	    p->drawLine( br.left(), br.bottom(), br.right() - 1, br.bottom() );
	    p->drawLine( br.topRight(), br.bottomRight() );
	    br.addCoords( 1, 1, -1, -1 );

	    p->fillRect( br, cg.brush( QColorGroup::Highlight ) );
	    break;
	}

    case PE_FocusRect:
	p->setBrush( NoBrush );
	if ( flags & Style_FocusAtBorder )
	    p->setPen( cg.shadow() );
	else
	    p->setPen( cg.dark() );
	p->drawRect( r );
	break;

    case PE_ProgressBarChunk:
	p->fillRect(r.x(), r.y() + 2, r.width(), r.height() - 4, cg.highlight());
	break;

    default:
	if (pe >= PE_ArrowUp && pe <= PE_ArrowLeft) {
	    QPointArray a;

	    switch ( pe ) {
	    case PE_ArrowUp:
		a.setPoints( 7, -4,1, 2,1, -3,0, 1,0, -2,-1, 0,-1, -1,-2 );
		break;

	    case PE_ArrowDown:
		a.setPoints( 7, -4,-2, 2,-2, -3,-1, 1,-1, -2,0, 0,0, -1,1 );
		break;

	    case PE_ArrowRight:
		a.setPoints( 7, -2,-3, -2,3, -1,-2, -1,2, 0,-1, 0,1, 1,0 );
		break;

	    case PE_ArrowLeft:
		a.setPoints( 7, 0,-3, 0,3, -1,-2, -1,2, -2,-1, -2,1, -3,0 );
		break;

	    default:
		break;
	    }

	    if (a.isNull())
		return;

	    p->save();
	    if ( flags & Style_Enabled ) {
		a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
		p->setPen( cg.buttonText() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
	    } else {
		a.translate( r.x() + r.width() / 2 + 1, r.y() + r.height() / 2 + 1 );
		p->setPen( cg.light() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
		a.translate( -1, -1 );
		p->setPen( cg.mid() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
	    }
	    p->restore();
	} else
	    QCommonStyle::drawPrimitive(pe, p, r, cg, flags, data);
	break;
    }
}

void LightStyleV3::drawControl( ControlElement control,
			      QPainter *p,
			      const QWidget *widget,
			      const QRect &r,
			      const QColorGroup &cg,
			      SFlags flags,
			      const QStyleOption &data ) const
{
    switch (control) {
    case CE_TabBarTab:
	{
	    const QTabBar *tb = (const QTabBar *) widget;
	    QRect br = r;

	    if ( tb->shape() == QTabBar::RoundedAbove ) {
		if ( ! ( flags & Style_Selected ) ) {
		    p->setPen( cg.background() );
		    p->drawLine( br.left(),  br.bottom(),
				 br.right(), br.bottom() );
		    p->setPen( cg.light() );
		    p->drawLine( br.left(),  br.bottom() - 1,
				 br.right(), br.bottom() - 1);
		    br.addCoords( 0, 2, -1, -2 );
		    if ( br.left() == 0 )
			p->drawPoint( br.left(), br.bottom() + 2 );
		} else {
		    p->setPen( cg.background() );
		    p->drawLine( br.bottomLeft(), br.bottomRight() );
		    if ( br.left() == 0 ) {
			p->setPen( cg.light() );
			p->drawPoint( br.bottomLeft() );
		    }
		    br.addCoords( 0, 0, 0, -1 );
		}

		p->setPen( cg.light() );
		p->drawLine( br.bottomLeft(), br.topLeft() );
		p->drawLine( br.topLeft(), br.topRight() );
		p->setPen( cg.dark() );
		p->drawLine( br.right(), br.top() + 1, br.right(), br.bottom() );
		br.addCoords( 1, 1, -1, 0 );
		p->fillRect( br, cg.brush( QColorGroup::Background ) );
	    } else if ( tb->shape() == QTabBar::RoundedBelow ) {
		if ( ! ( flags & Style_Selected ) ) {
		    p->setPen( cg.background() );
		    p->drawLine( br.left(),  br.top(),
				 br.right(), br.top() );
		    p->setPen( cg.dark() );
		    p->drawLine( br.left(),  br.top() + 1,
				 br.right(), br.top() + 1);
		    br.addCoords( 0, 2, -1, -2 );
		    if ( br.left() == 0 ) {
			p->setPen( cg.light() );
			p->drawPoint( br.left(), br.top() - 2 );
		    }
		} else {
		    p->setPen( cg.background() );
		    p->drawLine( br.topLeft(), br.topRight() );
		    if ( br.left() == 0 ) {
			p->setPen( cg.light() );
			p->drawPoint( br.topLeft() );
		    }
		    br.addCoords( 0, 1, 0, 0 );
		}

		p->setPen( cg.light() );
		p->drawLine( br.topLeft(), br.bottomLeft() );
		p->setPen( cg.dark() );
		p->drawLine( br.bottomLeft(), br.bottomRight() );
		p->drawLine( br.right(), br.top(), br.right(), br.bottom() - 1 );
		br.addCoords( 1, 0, -1, -1 );
		p->fillRect( br, cg.brush( QColorGroup::Background ) );
	    } else
		QCommonStyle::drawControl( control, p, widget, r, cg, flags, data );
	    break;
	}

    case CE_PopupMenuItem:
	{
	    if (! widget || data.isDefault())
		break;

	    const QPopupMenu *popupmenu = (const QPopupMenu *) widget;
	    QMenuItem *mi = data.menuItem();
	    int tab = data.tabWidth();
	    int maxpmw = data.maxIconWidth();

	    if ( mi && mi->isSeparator() ) {
		// draw separator
		p->fillRect(r, cg.brush(QColorGroup::Button));
		p->setPen( cg.mid() );
		p->drawLine(r.left() + 12,  r.top() + 1,
			    r.right() - 12, r.top() + 1);
		p->setPen( cg.light() );
		p->drawLine(r.left() + 12,  r.top() + 2,
			    r.right() - 12, r.top() + 2);
		break;
	    }

	    if (flags & Style_Active)
		qDrawShadePanel(p, r, cg, TRUE, 1,
				&cg.brush(QColorGroup::Midlight));
	    else
		p->fillRect(r, cg.brush(QColorGroup::Button));

	    if ( !mi )
		break;

	    maxpmw = QMAX(maxpmw, 16);

	    QRect cr, ir, tr, sr;
	    // check column
	    cr.setRect(r.left(), r.top(), maxpmw, r.height());
	    // submenu indicator column
	    sr.setCoords(r.right() - maxpmw, r.top(), r.right(), r.bottom());
	    // tab/accelerator column
	    tr.setCoords(sr.left() - tab - 4, r.top(), sr.left(), r.bottom());
	    // item column
	    ir.setCoords(cr.right() + 4, r.top(), tr.right() - 4, r.bottom());

	    bool reverse = QApplication::reverseLayout();
	    if ( reverse ) {
		cr = visualRect( cr, r );
		sr = visualRect( sr, r );
		tr = visualRect( tr, r );
		ir = visualRect( ir, r );
	    }
	    
	    if (mi->isChecked() &&
		! (flags & Style_Active) &
		(flags & Style_Enabled))
		qDrawShadePanel(p, cr, cg, TRUE, 1, &cg.brush(QColorGroup::Midlight));

	    if (mi->iconSet()) {
		QIconSet::Mode mode =
		    (flags & Style_Enabled) ? QIconSet::Normal : QIconSet::Disabled;
		if ((flags & Style_Active) && (flags & Style_Enabled))
		    mode = QIconSet::Active;
		QPixmap pixmap;
		if (popupmenu->isCheckable() && mi->isChecked())
		    pixmap =
			mi->iconSet()->pixmap( QIconSet::Small, mode, QIconSet::On );
		else
		    pixmap =
			mi->iconSet()->pixmap( QIconSet::Small, mode );
		QRect pmr(QPoint(0, 0), pixmap.size());
		pmr.moveCenter(cr.center());
		p->setPen(cg.text());
		p->drawPixmap(pmr.topLeft(), pixmap);
	    } else if (popupmenu->isCheckable() && mi->isChecked())
		drawPrimitive(PE_CheckMark, p, cr, cg,
			      (flags & Style_Enabled) | Style_On);

	    QColor textcolor;
	    QColor embosscolor;
	    if (flags & Style_Active) {
		if (! (flags & Style_Enabled))
		    textcolor = cg.midlight().dark();
		else
		    textcolor = cg.buttonText();
		embosscolor = cg.midlight().light();
	    } else if (! (flags & Style_Enabled)) {
		textcolor = cg.text();
		embosscolor = cg.light();
	    } else
		textcolor = embosscolor = cg.buttonText();
	    p->setPen(textcolor);

	    if (mi->custom()) {
		p->save();
		if (! (flags & Style_Enabled)) {
		    p->setPen(cg.light());
		    mi->custom()->paint(p, cg, flags & Style_Active,
					flags & Style_Enabled,
					ir.x() + 1, ir.y() + 1,
					ir.width() - 1, ir.height() - 1);
		    p->setPen(textcolor);
		}
		mi->custom()->paint(p, cg, flags & Style_Active,
				    flags & Style_Enabled,
				    ir.x(), ir.y(),
				    ir.width(), ir.height());
		p->restore();
	    }

	    QString text = mi->text();
	    if (! text.isNull()) {
		int t = text.find('\t');

		// draw accelerator/tab-text
		if (t >= 0) {
		    int alignFlag = AlignVCenter | ShowPrefix | DontClip | SingleLine;
		    alignFlag |= ( reverse ? AlignLeft : AlignRight );
		    if (! (flags & Style_Enabled)) {
			p->setPen(embosscolor);
			tr.moveBy(1, 1);
			p->drawText(tr, alignFlag, text.mid(t + 1));
			tr.moveBy(-1, -1);
			p->setPen(textcolor);
		    }

		    p->drawText(tr, alignFlag, text.mid(t + 1));
		}

		int alignFlag = AlignVCenter | ShowPrefix | DontClip | SingleLine;
		alignFlag |= ( reverse ? AlignRight : AlignLeft );

		if (! (flags & Style_Enabled)) {
		    p->setPen(embosscolor);
		    ir.moveBy(1, 1);
		    p->drawText(ir, alignFlag, text, t);
		    ir.moveBy(-1, -1);
		    p->setPen(textcolor);
		}

		p->drawText(ir, alignFlag, text, t);
	    } else if (mi->pixmap()) {
		QPixmap pixmap = *mi->pixmap();
		if (pixmap.depth() == 1)
		    p->setBackgroundMode(OpaqueMode);
		p->drawPixmap(ir.x(), (ir.height() - pixmap.height()) / 2, pixmap);
		if (pixmap.depth() == 1)
		    p->setBackgroundMode(TransparentMode);
	    }

	    if (mi->popup())
		drawPrimitive( (QApplication::reverseLayout() ? PE_ArrowLeft : PE_ArrowRight),
			       p, sr, cg, flags);
	    break;
	}

    case CE_MenuBarItem:
	{
	    if ( flags & Style_Active )
		qDrawShadePanel(p, r, cg, TRUE, 1, &cg.brush(QColorGroup::Midlight));
	    else
		p->fillRect( r, cg.brush( QColorGroup::Button ) );

	    if (data.isDefault())
		break;

	    QMenuItem *mi = data.menuItem();
	    drawItem(p, r, AlignCenter | ShowPrefix | DontClip | SingleLine, cg,
		     flags & Style_Enabled, mi->pixmap(), mi->text(), -1,
		     &cg.buttonText());
	    break;
	}

    case CE_ProgressBarGroove:
	drawLightBevel( p, r, cg, Style_Sunken, pixelMetric( PM_DefaultFrameWidth ),
			true, true, &cg.brush( QColorGroup::Background ) );
	break;

    default:
	QCommonStyle::drawControl(control, p, widget, r, cg, flags, data);
	break;
    }
}

void LightStyleV3::drawControlMask( ControlElement control,
				  QPainter *p,
				  const QWidget *widget,
				  const QRect &r,
				  const QStyleOption &data ) const
{
    switch (control) {
    case CE_PushButton:
	p->fillRect(r, color1);
	break;

    default:
	QCommonStyle::drawControlMask(control, p, widget, r, data);
	break;
    }
}

QRect LightStyleV3::subRect(SubRect subrect, const QWidget *widget) const
{
    QRect rect;

    switch (subrect) {
    case SR_PushButtonFocusRect:
	{
	    rect = QCommonStyle::subRect( SR_PushButtonContents, widget );
	    int bm = pixelMetric( PM_ButtonMargin, widget ), hbm = bm / 2;
	    rect.addCoords( hbm, hbm, -hbm, -hbm );
  	    break;
  	}

    case SR_ComboBoxFocusRect:
	{
	    rect = QCommonStyle::subRect( SR_ComboBoxFocusRect, widget );
	    rect.addCoords( -1, -1, 1, 1 );
	    break;
	}

    default:
	rect = QCommonStyle::subRect(subrect, widget);
	break;
    }

    return rect;
}

void LightStyleV3::drawComplexControl( ComplexControl control,
				     QPainter* p,
				     const QWidget* widget,
				     const QRect& r,
				     const QColorGroup& cg,
				     SFlags flags,
				     SCFlags controls,
				     SCFlags active,
				     const QStyleOption &data ) const
{
    switch (control) {
    case CC_ComboBox:
	{
	    const QComboBox *combobox = (const QComboBox *) widget;
	    QRect frame, arrow, field;
	    frame =
		QStyle::visualRect(querySubControlMetrics(CC_ComboBox, widget,
							  SC_ComboBoxFrame, data),
				   widget);
	    arrow =
		QStyle::visualRect(querySubControlMetrics(CC_ComboBox, widget,
							  SC_ComboBoxArrow, data),
				   widget);
	    field =
		QStyle::visualRect(querySubControlMetrics(CC_ComboBox, widget,
							  SC_ComboBoxEditField, data),
				   widget);

	    if ((controls & SC_ComboBoxFrame) && frame.isValid())
		drawPrimitive( PE_Panel, p, frame, cg, flags | Style_Sunken );

	    if ((controls & SC_ComboBoxArrow) && arrow.isValid()) {
		drawLightEtch( p, arrow, cg.button(), ( active == SC_ComboBoxArrow ) );
		arrow.addCoords( 1, 1, -1, -1 );
		p->fillRect( arrow, cg.brush( QColorGroup::Button ) );
		arrow.addCoords(3, 1, -1, -1);
		drawPrimitive(PE_ArrowDown, p, arrow, cg, flags);
	    }

	    if ((controls & SC_ComboBoxEditField) && field.isValid()) {
		if (flags & Style_HasFocus) {
		    if (! combobox->editable()) {
			QRect fr =
			    QStyle::visualRect( subRect( SR_ComboBoxFocusRect, widget ),
						widget );
			p->fillRect( fr, cg.brush( QColorGroup::Highlight ) );
			drawPrimitive( PE_FocusRect, p, fr, cg,
				       flags | Style_FocusAtBorder,
				       QStyleOption(cg.highlight()));
		    }

		    p->setPen(cg.highlightedText());
		} else {
		    p->fillRect( field, ( ( flags & Style_Enabled ) ?
					  cg.brush( QColorGroup::Base ) :
					  cg.brush( QColorGroup::Background ) ) );
		    p->setPen( cg.text() );
		}
	    }

	    break;
	}

    case CC_SpinWidget:
	{
	    const QSpinWidget *spinwidget = (const QSpinWidget *) widget;
	    QRect frame, up, down;

	    frame = querySubControlMetrics(CC_SpinWidget, widget,
					   SC_SpinWidgetFrame, data);
	    up = spinwidget->upRect();
	    down = spinwidget->downRect();
	    
	    if ((controls & SC_SpinWidgetFrame) && frame.isValid())
		drawPrimitive( PE_Panel, p, frame, cg, flags | Style_Sunken );

	    if ((controls & SC_SpinWidgetUp) && up.isValid()) {
		PrimitiveElement pe = PE_SpinWidgetUp;
		if ( spinwidget->buttonSymbols() == QSpinWidget::PlusMinus )
		    pe = PE_SpinWidgetPlus;

		p->setPen( cg.background() );
		p->drawLine( up.topLeft(), up.bottomLeft() );

		up.addCoords( 1, 0, 0, 0 );
		p->fillRect( up, cg.brush( QColorGroup::Button ) );
		drawLightEtch( p, up, cg.button(), ( active == SC_SpinWidgetUp ) );

		up.addCoords( 1, 0, 0, 0 );
		drawPrimitive(pe, p, up, cg, flags |
			      ((active == SC_SpinWidgetUp) ?
			       Style_On | Style_Sunken : Style_Raised));
	    }

	    if ((controls & SC_SpinWidgetDown) && down.isValid()) {
		PrimitiveElement pe = PE_SpinWidgetDown;
		if ( spinwidget->buttonSymbols() == QSpinWidget::PlusMinus )
		    pe = PE_SpinWidgetMinus;

		p->setPen( cg.background() );
		p->drawLine( down.topLeft(), down.bottomLeft() );

		down.addCoords( 1, 0, 0, 0 );
		p->fillRect( down, cg.brush( QColorGroup::Button ) );
		drawLightEtch( p, down, cg.button(), ( active == SC_SpinWidgetDown ) );

		down.addCoords( 1, 0, 0, 0 );
		drawPrimitive(pe, p, down, cg, flags |
			      ((active == SC_SpinWidgetDown) ?
			       Style_On | Style_Sunken : Style_Raised));
	    }

	    break;
	}

    case CC_ScrollBar:
	{
	    const QScrollBar *scrollbar = (const QScrollBar *) widget;
	    QRect addline, subline, subline2, addpage, subpage, slider, first, last;
	    bool maxedOut = (scrollbar->minValue() == scrollbar->maxValue());

	    subline = querySubControlMetrics(control, widget, SC_ScrollBarSubLine, data);
	    addline = querySubControlMetrics(control, widget, SC_ScrollBarAddLine, data);
	    subpage = querySubControlMetrics(control, widget, SC_ScrollBarSubPage, data);
	    addpage = querySubControlMetrics(control, widget, SC_ScrollBarAddPage, data);
	    slider  = querySubControlMetrics(control, widget, SC_ScrollBarSlider,  data);
	    first   = querySubControlMetrics(control, widget, SC_ScrollBarFirst,   data);
	    last    = querySubControlMetrics(control, widget, SC_ScrollBarLast,    data);

	    subline2 = addline;
	    if (scrollbar->orientation() == Qt::Horizontal)
		subline2.moveBy(-addline.width(), 0);
	    else
		subline2.moveBy(0, -addline.height());

       	    if ((controls & SC_ScrollBarSubLine) && subline.isValid()) {
		drawPrimitive(PE_ScrollBarSubLine, p, subline, cg,
			      Style_Enabled | ((active == SC_ScrollBarSubLine) ?
					       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));

		if (subline2.isValid())
		    drawPrimitive(PE_ScrollBarSubLine, p, subline2, cg,
				  Style_Enabled | ((active == SC_ScrollBarSubLine) ?
						   Style_Down : Style_Default) |
				  ((scrollbar->orientation() == Qt::Horizontal) ?
				   Style_Horizontal : 0));
	    }
	    if ((controls & SC_ScrollBarAddLine) && addline.isValid())
		drawPrimitive(PE_ScrollBarAddLine, p, addline, cg,
			      Style_Enabled | ((active == SC_ScrollBarAddLine) ?
					       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarSubPage) && subpage.isValid())
		drawPrimitive(PE_ScrollBarSubPage, p, subpage, cg,
			      Style_Enabled | ((active == SC_ScrollBarSubPage) ?
					       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarAddPage) && addpage.isValid())
		drawPrimitive(PE_ScrollBarAddPage, p, addpage, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarAddPage) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
       	    if ((controls & SC_ScrollBarFirst) && first.isValid())
		drawPrimitive(PE_ScrollBarFirst, p, first, cg,
			      Style_Enabled | ((active == SC_ScrollBarFirst) ?
					       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarLast) && last.isValid())
		drawPrimitive(PE_ScrollBarLast, p, last, cg,
			      Style_Enabled | ((active == SC_ScrollBarLast) ?
					       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarSlider) && slider.isValid()) {
		drawPrimitive(PE_ScrollBarSlider, p, slider, cg,
			      Style_Enabled | ((active == SC_ScrollBarSlider) ?
					       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));

		// ### perhaps this should not be able to accept focus if maxedOut?
		if (scrollbar->hasFocus()) {
		    QRect fr(slider.x() + 2, slider.y() + 2,
			     slider.width() - 5, slider.height() - 5);
		    drawPrimitive(PE_FocusRect, p, fr, cg, Style_Default);
		}
	    }

	    break;
	}

    case CC_Slider:
	{
	    const QSlider *slider = (const QSlider *) widget;
	    QRect groove = querySubControlMetrics(CC_Slider, widget, SC_SliderGroove,
						  data),
		  handle = querySubControlMetrics(CC_Slider, widget, SC_SliderHandle,
						  data);

	    if ((controls & SC_SliderGroove) && groove.isValid()) {
		drawLightBevel( p, groove, cg,
				( ( flags | Style_Raised ) ^ Style_Raised ) |
				( ( flags & Style_Enabled ) ? Style_Sunken :
				  Style_Default ), 2, true, true,
				&cg.brush( QColorGroup::Midlight ) );
		groove.addCoords( 2, 2, -2, -2 );
		drawLightEtch( p, groove, cg.midlight(), false );

		if (flags & Style_HasFocus) {
		    groove.addCoords( -2, -2, 2, 2 );
		    drawPrimitive( PE_FocusRect, p, groove, cg, flags );
		}
	    }

	    if ((controls & SC_SliderHandle) && handle.isValid()) {
		p->setPen( cg.highlight().light() );
		p->drawLine( handle.topLeft(), handle.topRight() );
		p->drawLine( handle.left(), handle.top() + 1,
			     handle.left(), handle.bottom() - 1 );
		p->setPen( cg.highlight().dark() );
		p->drawLine( handle.left(), handle.bottom(),
			     handle.right() - 1, handle.bottom() );
		p->drawLine( handle.topRight(), handle.bottomRight() );
		handle.addCoords( 1, 1, -1, -1 );
		p->fillRect( handle, cg.brush( QColorGroup::Highlight ) );
		p->setPen( cg.midlight() );

		if ( slider->orientation() == Qt::Horizontal )
		    p->drawLine( handle.left() + handle.width() / 2,
				 handle.top() + 1,
				 handle.left() + handle.width() / 2,
				 handle.bottom() - 1 );
		else
		    p->drawLine( handle.left() + 1,
				 handle.top() + handle.height() / 2,
				 handle.right() - 1,
				 handle.top() + handle.height() / 2 );
	    }

	    if (controls & SC_SliderTickmarks)
		QCommonStyle::drawComplexControl(control, p, widget, r, cg, flags,
						 SC_SliderTickmarks, active, data );
	    break;
	}

    case CC_ListView:
	// use the base style for CC_ListView
	basestyle->drawComplexControl(control, p, widget, r, cg, flags,
				      controls, active, data);
	break;

    default:
	QCommonStyle::drawComplexControl(control, p, widget, r, cg, flags,
				      controls, active, data);
	break;
    }
}

QRect LightStyleV3::querySubControlMetrics( ComplexControl control,
					  const QWidget *widget,
					  SubControl sc,
					  const QStyleOption &data ) const
{
    QRect ret;

    switch (control) {
    case CC_ComboBox:
	{
	    int fw = pixelMetric( PM_DefaultFrameWidth, widget );
	    int sb = pixelMetric( PM_ScrollBarExtent ); // width of the arrow

	    switch ( sc ) {
	    case SC_ComboBoxFrame:
		ret = widget->rect();
		break;
	    case SC_ComboBoxArrow:
		ret.setRect( widget->width() - fw - sb, fw,
			     sb, widget->height() - fw*2 );
		break;
	    case SC_ComboBoxEditField:
		ret.setRect( fw, fw, widget->width() - fw*2 - sb - 1,
			     widget->height() - fw*2 );
		break;
	    default:
		break;
	    }

	    break;
	}

    case CC_ScrollBar:
	{
	    const QScrollBar *scrollbar = (const QScrollBar *) widget;
	    int sliderstart = scrollbar->sliderStart();
	    int sbextent = pixelMetric(PM_ScrollBarExtent, widget);
	    int maxlen = ((scrollbar->orientation() == Qt::Horizontal) ?
			  scrollbar->width() : scrollbar->height()) - (sbextent * 3);
	    int sliderlen;

	    // calculate slider length
	    if (scrollbar->maxValue() != scrollbar->minValue()) {
		uint range = scrollbar->maxValue() - scrollbar->minValue();
		sliderlen = (scrollbar->pageStep() * maxlen) /
			    (range + scrollbar->pageStep());

		int slidermin = pixelMetric( PM_ScrollBarSliderMin, widget );
		if ( sliderlen < slidermin || range > INT_MAX / 2 )
		    sliderlen = slidermin;
		if ( sliderlen > maxlen )
		    sliderlen = maxlen;
	    } else
		sliderlen = maxlen;

	    switch (sc) {
	    case SC_ScrollBarSubLine:
		// top/left button
		ret.setRect(0, 0, sbextent, sbextent);
		break;

	    case SC_ScrollBarAddLine:
		// bottom/right button
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(scrollbar->width() - sbextent, 0, sbextent, sbextent);
		else
		    ret.setRect(0, scrollbar->height() - sbextent, sbextent, sbextent);
		break;

	    case SC_ScrollBarSubPage:
		// between top/left button and slider
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(sbextent, 0, sliderstart - sbextent, sbextent);
		else
		    ret.setRect(0, sbextent, sbextent, sliderstart - sbextent);
		break;

	    case SC_ScrollBarAddPage:
		// between bottom/right button and slider
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(sliderstart + sliderlen, 0, maxlen - sliderstart -
				sliderlen + sbextent, sbextent);
		else
		    ret.setRect(0, sliderstart + sliderlen, sbextent, maxlen -
				sliderstart - sliderlen + sbextent);
		break;

	    case SC_ScrollBarGroove:
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(sbextent, 0, maxlen, sbextent );
		else
		    ret.setRect(0, sbextent, sbextent, maxlen );
		break;

	    case SC_ScrollBarSlider:
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(sliderstart, 0, sliderlen, sbextent);
		else
		    ret.setRect(0, sliderstart, sbextent, sliderlen);
		break;

	    default:
		break;
	    }

	    break;
	}

    case CC_Slider:
	{
	    const QSlider *slider = (const QSlider *) widget;
	    int tickOffset = pixelMetric( PM_SliderTickmarkOffset, widget );
	    int thickness = pixelMetric( PM_SliderControlThickness, widget );

	    switch ( sc ) {
	    case SC_SliderGroove:
		if ( slider->orientation() == Horizontal )
		    ret.setRect( 0, tickOffset, slider->width(), thickness );
		else
		    ret.setRect( tickOffset, 0, thickness, slider->height() );
		break;

	    case SC_SliderHandle:
		{
		    int pos = slider->sliderStart();
		    int len = pixelMetric( PM_SliderLength, widget );

		    if ( slider->orientation() == Horizontal )
			ret.setRect( pos + 2, tickOffset + 2, len - 4, thickness - 4 );
		    else
			ret.setRect( tickOffset + 2, pos + 2, thickness - 4, len - 4 );
		    break;
		}

	    default:
		ret = QCommonStyle::querySubControlMetrics(control, widget, sc, data);
		break;
	    }

	    break;
	}

    default:
	ret = QCommonStyle::querySubControlMetrics(control, widget, sc, data);
	break;
    }

    return ret;
}

QStyle::SubControl LightStyleV3::querySubControl( ComplexControl control,
						const QWidget *widget,
						const QPoint &pos,
						const QStyleOption &data ) const
{
    QStyle::SubControl ret =
	QCommonStyle::querySubControl(control, widget, pos, data);

    // this is an ugly hack, but i really don't care, it's the quickest way to
    // enabled the third button
    if (control == CC_ScrollBar &&
	ret == SC_None)
	ret = SC_ScrollBarSubLine;

    return ret;
}

int LightStyleV3::pixelMetric( PixelMetric metric,
			     const QWidget *widget ) const
{
    int ret;

    switch (metric) {
    case PM_ButtonMargin:
	ret = 6;
	break;

    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 0;
	break;

    case PM_ButtonDefaultIndicator:
	ret = 0;
	break;

    case PM_DefaultFrameWidth:
	ret = 2;
	break;

    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
	ret = 13;
	break;

    case PM_TabBarTabOverlap:
    case PM_TabBarBaseOverlap:
	ret = 0;
	break;

    case PM_ScrollBarExtent:
    case PM_ScrollBarSliderMin:
	ret = 15;
	break;

    case PM_MenuBarFrameWidth:
	ret = 1;
	break;

    case PM_ProgressBarChunkWidth:
	ret = 2;
	break;

    case PM_DockWindowHandleExtent:
	ret = 8;
	break;

    case PM_DockWindowSeparatorExtent:
	ret = 8;
	break;

    case PM_SplitterWidth:
	ret = 8;
	break;

    case PM_SliderLength:
	ret = 25;
	break;

    case PM_SliderThickness:
	ret = 11;
	break;

    case PM_SliderControlThickness:
	{
	    const QSlider * sl = (const QSlider *) widget;
	    int space = (sl->orientation() == Horizontal) ? sl->height()
			: sl->width();
	    int ticks = sl->tickmarks();
	    int n = 0;
	    if ( ticks & QSlider::Above ) n++;
	    if ( ticks & QSlider::Below ) n++;
	    if ( !n ) {
		ret = space;
		break;
	    }

	    int thick = 6;	// Magic constant to get 5 + 16 + 5

	    space -= thick;
	    //### the two sides may be unequal in size
	    if ( space > 0 )
		thick += (space * 2) / (n + 2);
	    ret = thick;
	    break;
	}

   case PM_MaximumDragDistance:
	ret = -1;
	break;

    default:
	ret = QCommonStyle::pixelMetric(metric, widget);
	break;
    }

    return ret;
}

QSize LightStyleV3::sizeFromContents( ContentsType contents,
				    const QWidget *widget,
				    const QSize &contentsSize,
				    const QStyleOption &data ) const
{
    QSize ret;

    switch (contents) {
    case CT_ComboBox:
	{
	    int fw = pixelMetric( PM_DefaultFrameWidth, widget ) * 2;
	    int sb = pixelMetric( PM_ScrollBarExtent ); // width of the arrow
	    int w = contentsSize.width();
	    int h = contentsSize.height();

	    w += fw + sb + 1;
	    h += fw;

	    // try to keep a similar height to buttons
	    if ( h < 21 )
		h = 21;

	    ret = QSize( w, h );
	    break;
	}

    case CT_PushButton:
	{
	    const QPushButton *button = (const QPushButton *) widget;
	    ret = QCommonStyle::sizeFromContents( contents, widget, contentsSize, data );
	    int w = ret.width(), h = ret.height();
	    int dbi = pixelMetric( PM_ButtonDefaultIndicator, widget ) * 2;
	    int mw = 80 - dbi, mh = 25 - dbi;

	    // only expand the button if we are displaying text...
	    if ( ! button->pixmap() ) {
		// button minimum size
		if ( w < mw )
		    w = mw;
		if ( h < mh )
		    h = mh;
	    }

	    ret = QSize( w, h );
	    break;
	}

    case CT_PopupMenuItem:
	{
	    if (! widget || data.isDefault())
		break;

	    QMenuItem *mi = data.menuItem();
	    const QPopupMenu *popupmenu = (const QPopupMenu *) widget;
	    int maxpmw = data.maxIconWidth();
	    int w = contentsSize.width(), h = contentsSize.height();

	    if (mi->custom()) {
		w = mi->custom()->sizeHint().width();
		h = mi->custom()->sizeHint().height();
		if (! mi->custom()->fullSpan() && h < 22)
		    h = 22;
	    } else if(mi->widget()) {
	    } else if (mi->isSeparator()) {
		w = 10;
		h = 4;
	    } else {
		// check is at least 16x16
		if (h < 16)
		    h = 16;
		if (mi->pixmap())
		    h = QMAX(h, mi->pixmap()->height());
		else if (! mi->text().isNull())
		    h = QMAX(h, popupmenu->fontMetrics().height() + 2);
		if (mi->iconSet() != 0)
		    h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small,
						      QIconSet::Normal).height());
		h += 2;
	    }

	    // check | 4 pixels | item | 8 pixels | accel | 4 pixels | check

	    // check is at least 16x16
	    maxpmw = QMAX(maxpmw, 16);
	    w += (maxpmw * 2) + 8;

	    if (! mi->text().isNull() && mi->text().find('\t') >= 0)
		w += 8;

	    ret = QSize(w, h);
	    break;
	}

    default:
	ret = QCommonStyle::sizeFromContents(contents, widget, contentsSize, data);
	break;
    }

    return ret;
}

int LightStyleV3::styleHint( StyleHint stylehint,
			   const QWidget *widget,
			   const QStyleOption &option,
			   QStyleHintReturn* returnData ) const
{
    int ret;

    switch (stylehint) {
    case SH_EtchDisabledText:
    case SH_Slider_SnapToValue:
    case SH_PrintDialog_RightAlignButtons:
    case SH_FontDialog_SelectAssociatedText:
    case SH_PopupMenu_AllowActiveAndDisabled:
    case SH_MenuBar_AltKeyNavigation:
    case SH_MenuBar_MouseTracking:
    case SH_PopupMenu_MouseTracking:
    case SH_ComboBox_ListMouseTracking:
    case SH_ScrollBar_MiddleClickAbsolutePosition:
	ret = 1;
	break;

    case SH_MainWindow_SpaceBelowMenuBar:
	ret = 0;
	break;

    case SH_ScrollBar_BackgroundMode:
	ret = NoBackground;
	break;

    default:
	ret = QCommonStyle::styleHint(stylehint, widget, option, returnData);
	break;
    }

    return ret;
}

QPixmap LightStyleV3::stylePixmap( StylePixmap stylepixmap,
				   const QWidget *widget,
				   const QStyleOption &data ) const
{
    return basestyle->stylePixmap( stylepixmap, widget, data );
}
#include "lightstyle-v3.moc"
