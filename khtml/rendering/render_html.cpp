#include "render_html.h"

#include <qpainter.h>

#include <kdebug.h>

using namespace khtml;

RenderHtml::RenderHtml()
    : RenderFlow()
{
}

RenderHtml::~RenderHtml()
{
}

void RenderHtml::print(QPainter *p, int _x, int _y, int _w, int _h, int _tx, int _ty)
{
    _tx += m_x;
    _ty += m_y;

    printObject(p, _x, _y, _w, _h, _tx, _ty);
}

void RenderHtml::printBoxDecorations(QPainter *p,int, int _y,
				       int, int _h, int _tx, int _ty)
{
    kdDebug( 6040 ) << renderName() << "::printDecorations()" << endl;

    QColor c = m_style->backgroundColor();

    int w = width();
    int h = height();

    int bx = _tx - marginLeft();
    int by = _ty - marginTop();
    int bw = w + marginLeft() + marginRight() + borderLeft() + borderRight();
    int bh = h + marginTop() + marginBottom() + borderTop() + borderBottom();

    if( bw < parent()->width() )
	bw = parent()->width();
    if( bh < parent()->height() )
	bh = parent()->height();
    
    if(c.isValid()) {
	p->fillRect(bx, by, bw, bh, c);
    }
    if(m_bgImage) {
	kdDebug( 6040 ) << "printing bgimage at " << bx << "/" << by << " " << bw << "/" << bh << endl;
	// ### might need to add some correct offsets
	// ### use paddingX/Y
	
	int sx = 0;
	int sy = 0;
	
	if( !m_style->backgroundAttachment() ) {
	    //kdDebug(0) << "fixed background" << endl;
	    QRect r = viewRect();
	    sx = bx - r.x();
	    sy = by - r.y();
	
	} 	

	switch(m_style->backgroundRepeat()) {
	case NO_REPEAT:
	    if(m_bgImage->pixmap().width() < bw)
		bw = m_bgImage->pixmap().width();
	case REPEAT_X:
	    if(m_bgImage->pixmap().height() < bh)
		bh = m_bgImage->pixmap().height();
	    break;
	case REPEAT_Y:
	    if(m_bgImage->pixmap().width() < bw)
		bw = m_bgImage->pixmap().width();
	    break;
	case REPEAT:
	    break;
	}
	kdDebug( 6040 ) << "printing bgimage at " << bx << "/" << by << " " << bw << "/" << bh << endl;
	p->drawTiledPixmap(bx, by, bw, bh,m_bgImage->pixmap(), sx, sy);
    }

    if(m_style->hasBorder())
    {
	if(m_style->borderTopStyle() != BNONE)
	{
	    c = m_style->borderTopColor();
	    if(!c.isValid()) c = m_style->color();
	    drawBorder(p, _tx, _ty, _tx + w, _ty, m_style->borderTopWidth(),
		       BSTop, c, m_style->borderTopStyle());
	}
	if(m_style->borderBottomStyle() != BNONE)
	{
	    c = m_style->borderBottomColor();
	    if(!c.isValid()) c = m_style->color();
	    drawBorder(p, _tx, _ty + h, _tx + w, _ty + h, m_style->borderBottomWidth(),
		       BSBottom, c, m_style->borderBottomStyle());
	}
	if(m_style->borderLeftStyle() != BNONE)
	{
	    c = m_style->borderLeftColor();
	    if(!c.isValid()) c = m_style->color();
	    drawBorder(p, _tx, _ty, _tx, _ty + h, m_style->borderLeftWidth(),
		       BSLeft, c, m_style->borderLeftStyle());
	}
	if(m_style->borderRightStyle() != BNONE)
	{
	    c = m_style->borderRightColor();
	    if(!c.isValid()) c = m_style->color();
	    drawBorder(p, _tx + w, _ty, _tx + w, _ty + h, m_style->borderRightWidth(),
		       BSRight, c, m_style->borderRightStyle());
	}
    }
}

void RenderHtml::repaint()
{
    int bx = - marginLeft();
    int by = - marginTop();
    int bw = m_width + marginLeft() + marginRight() + borderLeft() + borderRight();
    int bh = m_height + marginTop() + marginBottom() + borderTop() + borderBottom();
    repaintRectangle(bx, by, bw, bh);
}
