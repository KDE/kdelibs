#include "ksgistyle.h"
#include <qpainter.h>
#include <qwidget.h>
#include <kapp.h>
#include <qbitmap.h>
#include <qdrawutil.h>
#include <qpushbutton.h>
#include <qscrollbar.h>

#include <limits.h>

#define SB_BORDER_WIDTH 1
#define SLIDER_MIN	16

static const char * check_xpm[] = {
"16 16 3 1",
"       c #FFFFFFFFFFFF",
".      c #FFFF00000000",
"X      c #000000000000",
"                ",
"                ",
"                ",
"           .....",
"         ....XXX",
"    .   ...XXX  ",
"   ... ...X     ",
"  .......X      ",
"  ......X       ",
"    ....X       ",
"     ..X        ",
"      .         ",
"                ",
"                ",
"                ",
"                "};

static unsigned char check_mask[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0xfe, 0x10, 0x3f, 
	0xb8, 0x07, 0xfc, 0x03, 0xfc, 0x01, 0xf0, 0x01, 0xe0, 0x00, 0x40, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static unsigned char downarrow_bits[] = {
 0xff,0x01,0xff,0x01,0xfe,0x00,0xfe,0x00,0x7c,0x00,0x7c,0x00,0x38,0x00,0x38,
 0x00,0x10,0x00};

static unsigned char leftarrow_bits[] = {
 0x80,0x01,0xe0,0x01,0xf8,0x01,0xfe,0x01,0xff,0x01,0xfe,0x01,0xf8,0x01,0xe0,
 0x01,0x80,0x01};

static unsigned char rightarrow_bits[] = {
 0x03,0x00,0x0f,0x00,0x3f,0x00,0xff,0x00,0xff,0x01,0xff,0x00,0x3f,0x00,0x0f,
 0x00,0x03,0x00};

static unsigned char uparrow_bits[] = {
  0x10, 0x00, 0x38, 0x00, 0x38, 0x00, 0x7c, 0x00, 0x7c, 0x00, 0xfe, 0x00,
  0xfe, 0x00, 0xff, 0x01, 0xff, 0x01};

      
KSgiStyle::KSgiStyle()
	:KStyle()
{
	highlightWidget = NULL;
	
	//
	// Setup up pixmaps and bitmaps
	//
	checkPix = new QPixmap(check_xpm);
	checkPix->setMask (QBitmap(16, 16, check_mask, true));
	
  setButtonDefaultIndicatorWidth(4);
	setScrollBarExtent(19, 19);
	
}

KSgiStyle::~KSgiStyle()
{
	delete checkPix;
}

void
KSgiStyle::polish(QWidget *w)
{
	if (w->isTopLevel())
  	return;
		
	if(w->inherits("QButton")){
 //    w->installEventFilter(this);
  //   QPalette pal = w->palette();
     //
		 // we use this as a flag since it's otherwise unused.
     //
		// pal.setColor(QColorGroup::Highlight, Qt::black);
    // w->setPalette(pal);

  } else if (w->inherits("QComboBox")) {
    w->installEventFilter(this);		
	}

}

void 
KSgiStyle::unPolish(QWidget *w)
{
	if (w->isTopLevel()) 
  	return;

  if(w->inherits("QButton")){
//    w->removeEventFilter(this);
//    QPalette pal = w->palette();
 //   pal.setColor(QColorGroup::Highlight,
   //                  kapp->palette().active().color(QColorGroup::Highlight));
   // w->setPalette(pal);

  } else if (w->inherits("QComboBox")) {
    w->removeEventFilter(this);		
	}
}

bool
KSgiStyle::eventFilter(QObject *obj, QEvent *ev)
{
	if(obj->inherits("QButton")){
		if (ev->type() == QEvent::Enter) {
//        QWidget *btn = (QWidget *)obj;
 //       if (btn->isEnabled()){
 //     	QPalette pal = btn->palette();
 //       pal.setColor(QColorGroup::Highlight,
 //                    pal.active().color(QColorGroup::Midlight));
 //       btn->setPalette(pal);
 //       btn->repaint(false);
//			}
		} else if (ev->type() == QEvent::Leave) {
  //      QWidget *btn = (QWidget *)obj;
    //    QPalette pal = btn->palette();
   //     pal.setColor(QColorGroup::Highlight, Qt::black);
   //     btn->setPalette(pal);
    //   	btn->repaint(false);
		}
	} else if (obj->inherits("QComboBox")) {
    if (ev->type() == QEvent::Enter) {
			QWidget* combo = (QWidget *) obj;
			highlightWidget = combo;
			combo->repaint(false);
		} else if (ev->type() == QEvent::Leave) {
			QWidget* combo = (QWidget *) obj;
			highlightWidget = NULL;
			combo->repaint(false);
		
		}
	}
	
	return(false);
}


void 
KSgiStyle::drawButton(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, bool sunken, const QBrush* fill)
{

//	printf ("dev type %d, painter %x, highlightWidget %x\n",
//	p->device()->devType(),p->device(),highlightWidget);
	
	QPen oldPen = p->pen();
	int	x2 = x+w-1;
	int y2 = y+h-1;
	
	
//	if ((QWidget *) (p->device()) == highlightWidget) {
//	 	 p->fillRect (x+3, y+3, w-6, h-6, g.midlight());
//	} else if (g.highlight() != Qt::black && highlightWidget == NULL) {
//		p->fillRect (x+3, y+3, w-6, h-6, g.highlight());
//	} else {
//		p->fillRect(x+3, y+3, w-6, h-6, g.brush(QColorGroup::Background));
//	}
	//
	// draw outline
	//
	p->setPen (sunken ? g.dark() : Qt::black);
	p->drawLine(x, y2, x2, y2);
	p->drawLine(x2, y, x2, y2);
	p->setPen(sunken ?  Qt::black : g.dark());
	p->drawLine(x, y, x2, y);
	p->drawLine(x, y, x, y2);
	
	drawFullShadeButton(p, x+1, y+1, w-2, h-2, g, sunken, fill);
	
	p->setPen(oldPen);
}

void
KSgiStyle::drawFullShadeButton(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, bool sunken, const QBrush* fill)
{
	int	x2 = x+w-1;
	int y2 = y+h-1;

	QPen oldPen = p->pen();
	//
	// draw first step
	//
	p->setPen(sunken ? g.light() : g.dark());
	p->drawLine(x, y2, x2, y2);
	p->drawLine(x2, y, x2, y2);
	p->setPen(sunken ? g.dark() : g.light());
	p->drawLine(x, y, x2, y);
	p->drawLine(x, y, x, y2);
	
	//
	// draw inner step
	//
	
	p->setPen(sunken ? g.midlight() : g.mid());
	p->drawLine(x+1, y2-1, x2-1, y2-1);
	p->drawLine(x2-1, y+1, x2-1, y2-1);
	p->setPen(sunken ? g.mid() : g.midlight());
	p->drawLine(x+1, y+1, x2-1, y+1);
	p->drawLine(x+1, y+1, x+1, y2-1);

	if ( fill ) {
	    QBrush oldBrush = p->brush();
	    p->setBrush( *fill );
	    p->setPen( NoPen );
	    p->drawRect( x+2, y+2, w-4, h-4 );
	    p->setBrush( oldBrush );
	}
	
	p->setPen(oldPen);
}

void
KSgiStyle::drawPartShadeButton(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, bool sunken, const QBrush* fill)
{
	int	x2 = x+w-1;
	int y2 = y+h-1;

	QPen oldPen = p->pen();

	//
	// draw first step
	//
	p->setPen(sunken ? g.light() : g.mid());
	p->drawLine(x, y2, x2, y2);
	p->drawLine(x2, y, x2, y2);
	p->setPen(sunken ? g.mid() : g.light());
	p->drawLine(x, y, x2, y);
	p->drawLine(x, y, x, y2);
	
	//
	// draw inner step
	//
	
	p->setPen(sunken ? g.midlight() : g.mid());
	p->drawLine(x+1, y2-1, x2-1, y2-1);
	p->drawLine(x2-1, y+1, x2-1, y2-1);
	p->setPen(sunken ? g.mid() : g.midlight());
	p->drawLine(x+1, y+1, x2-1, y+1);
	p->drawLine(x+1, y+1, x+1, y2-1);
	
	if ( fill ) {
	    QBrush oldBrush = p->brush();
	    p->setBrush( *fill );
	    p->setPen( NoPen );
	    p->drawRect( x+2, y+2, w-4, h-4 );
	    p->setBrush( oldBrush );
	}

	p->setPen(oldPen);
}

void 
KSgiStyle::drawBevelButton( QPainter *p, int x, int y, int w, int h,
				const QColorGroup &g, bool sunken, const QBrush* fill)
{
    drawFullShadeButton(p, x, y, w, h, g, sunken, fill);
}

void 
KSgiStyle::drawPushButton(QPushButton *btn, QPainter *p)
{
    QRect r = btn->rect();
    bool sunken = btn->isOn() || btn->isDown();
    QColorGroup g = btn->colorGroup();
    
    drawButton(p, r.x(), r.y(), r.width(), r.height(), g, sunken);
		
 /*  if(btn->isDefault()){
        QPen oldPen = p->pen();

				p->setPen(Qt::black);
				p->setPen(Qt::DashLine);
        p->drawLine(r.x()+1, r.y(), r.right()-1, r.y());
        p->drawLine(r.x()+1, r.bottom(), r.right()-1, r.bottom());
        p->drawLine(r.x(), r.y()+1, r.x(), r.bottom()-1);
        p->drawLine(r.right(), r.y()+1, r.right(), r.bottom()-1);

    		p->setPen (oldPen);
		}
*/
}


QRect 
KSgiStyle::buttonRect(int x, int y, int w, int h)
{
    return(QRect(x+4, y+4, w-8, h-8));
}
/*
void 
KSgiStyle::drawButtonMask(QPainter *p, int x, int y, int w, int h)
{
    p->fillRect(x, y, w, h, QBrush(color1, SolidPattern));
}
*/
QSize
KSgiStyle::indicatorSize() const
{
    return(QSize(16, 16));
}


void
KSgiStyle::drawIndicator(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, int state, bool down, bool)
{
//	static QPixmap arrowPix(arrow_xpm);
//	static QBitmap arrowMask(16, 16, arrow_mask, true);
//	arrowPix.setMask (arrowMask);
	
	p->fillRect( x, y, w, h, g.brush(QColorGroup::Background));

	if (w < h) {
		y = h-w;
		h = w;
	} else {
		x = w-h;
		w = h;
	}
	
	QPen oldPen = p->pen();
	int	x2 = x+w-1;
	int y2 = y+h-1;
	
	//
	// draw outline
	//
	p->setPen (Qt::black);
	p->drawLine(x, y2, x2, y2);
	p->drawLine(x2, y, x2, y2);
	p->setPen(g.dark());
	p->drawLine(x, y, x2, y);
	p->drawLine(x, y, x, y2);
	
	drawPartShadeButton(p, x+1, y+1, w-2, h-2, g, 0);
	
	p->setPen(oldPen);
	
	if (state)
		p->drawPixmap (0, 2, *checkPix);
	
	
}


QSize 
KSgiStyle::exclusiveIndicatorSize() const
{
	return(QSize(16,16));
}

void
KSgiStyle::drawExclusiveIndicator(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, bool on, bool, bool)
{

	int mx = w/2;
	int my = h/2;
	int	x2 = w-1;
	int y2=h-1;
	
	QPen oldPen = p->pen();
	
	p->fillRect( x, y, w, h, g.brush(QColorGroup::Background));

	p->setPen(g.dark());
	p->drawLine(x, my-1, mx-1, y);
	p->drawLine(mx, y, x2, my-1);
	p->drawLine(x2, my, mx, y2);
	p->drawLine(mx-1, y2, x, my);
	p->drawLine(x2-2, my+1, mx, y2-1);
	p->drawLine(mx-1, y2-1, x+2, my+1);
	
	p->setPen(g.light());
	p->drawLine(x+2, my-1, mx-1, y+2);
	p->drawLine(mx, y+2, x2-2, my-1);
	
	if (on) {
		p->setPen(Qt::blue);
		p->drawLine(mx, y+3, mx, y2-5);
		p->drawLine(mx+1, y+4, mx+1, y2-6);
		p->drawLine(mx+2, y+5, mx+2, y2-7);
		p->drawLine(mx+3, y+6, mx+3, y2-8);
		p->setPen(Qt::black);
		p->drawLine(mx+1, y2-5, mx+4, y2-8);
	}
	
/*
	p->setPen(g.mid());
	p->drawLine(x+1, my-1, mx-1, y-1);
	p->drawLine(mx, y-1, x2-1, my-1);
	
	p->drawLine(x2, my, mx, y2);
	p->drawLine(mx-1, y2, x, my);
	p->
	if (state) {
	}
	*/
	p->setPen(oldPen);
}

QRect 
KSgiStyle::comboButtonRect (int x, int y, int w, int h)
{
//	return(QRect(x+3, y+3, w - (h / 3) - 13, h-6));
	return(QRect(x+4, y+4, w - 23, h-8));

}		
QRect 
KSgiStyle::comboButtonFocusRect (int x, int y, int w, int h)
{
//	return(QRect(x+3, y+3, w - (h / 3) - 13, h-6));
	return(QRect(x+4, y+4, w - 23, h-8));
}

void 
KSgiStyle::drawComboButton(QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &g, bool,
                         bool editable, bool,
                         const QBrush*)
{
	QPen oldPen = p->pen();
	
	drawButton (p, x, y, w, h, g, 0);
	
	int midY = (h-3)/2;
	p->setPen(Qt::black);
	p->drawLine(w-15, midY+3, w-8, midY+3); 
	p->drawLine(w-8, midY+3, w-8, midY+1); 
	p->setPen(g.dark());
	p->drawRect(w-16, midY, 8, 3);

  if(editable){
  	int x2 = w-x-18-1;
    int y2 = h-y-4-1;
    p->setPen(g.dark());
    p->drawLine(x+3, y+3, x2, y+3);
    p->drawLine(x+3, y+4, x+3, y2);
 	}
 	
	p->setPen(oldPen);
}

int
KSgiStyle::sliderLength() const
{
    return(30);
}

void 
KSgiStyle::drawSliderGroove(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, QCOORD,
                                    Orientation)
{
	QPen oldPen = p->pen();
	int	x2 = x+w-1;
	int y2 = y+h-1;

	p->setPen(g.mid());
	p->drawLine(x, y, x2, y);
	p->drawLine(x, y, x, y2);
	p->setPen(g.light());
	p->drawLine(x2, y, x2, y2);
	p->drawLine(x, y2, x2, y2);

	p->setPen(g.dark());
	p->drawLine(x+1, y+1, x2-1, y+1);
	p->drawLine(x+1, y+1, x+1, y2-1);
	p->setPen(g.midlight());
	p->drawLine(x2-1, y+1, x2-1, y2-1);
	p->drawLine(x+1, y2-1, x2-1, y2-1);

	p->setPen(Qt::black);
	p->drawLine(x+2, y+2, x2-2, y+2);
	p->drawLine(x+2, y+2, x+2, y2-2);
	p->drawLine(x2-2, y+2, x2-2, y2-2);
	p->drawLine(x+2, y2-2, x2-2, y2-2);

	p->setPen(g.midlight());
	p->drawLine(x+3, y+3, x2-3, y+3);
	p->drawLine(x+3, y+3, x+3, y2-3);
	p->setPen(g.mid());
	p->drawLine(x2-3, y+3, x2-3, y2-3);
	p->drawLine(x+3, y2-3, x2-3, y2-3);

	p->setPen(oldPen);
}

void 
KSgiStyle::drawSlider(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, Orientation orient,
                            bool, bool)
{
	if (orient == Horizontal) {
		y += 2;
		h -= 4;
	} else {
		x += 2;
		w -= 4;
	}

	QPen oldPen = p->pen();
	int	x2 = x+w-1;
	int y2 = y+h-1;

	p->fillRect(x+3, y+3, w-6, h-6, g.brush(QColorGroup::Background));

	//
	// draw outline
	//
	p->setPen (Qt::black);
	p->drawRect(x, y, w, h);
	
	//
	// draw first step
	//
	p->setPen(g.dark());
	p->drawLine(x+1, y2-1, x2-1, y2-1);
	p->drawLine(x2-1, y+1, x2-1, y2-1);
	p->setPen(g.light());
	p->drawLine(x+1, y+1, x2-1, y+1);
	p->drawLine(x+1, y+1, x+1, y2-1);
	
	//
	// draw inner step
	//
	
	p->setPen(g.dark());
	p->drawLine(x+2, y2-2, x2-2, y2-2);
	p->drawLine(x2-2, y+2, x2-2, y2-2);
	p->setPen(g.midlight());
	p->drawLine(x+2, y+2, x2-2, y+2);
	p->drawLine(x+2, y+2, x+2, y2-2);


	if (orient == Horizontal) {
			int midX = w/2;
			
			p->setPen(g.light());
			p->drawLine(x+midX-1, y+2, x+midX-1, h-y+1);
			p->setPen(Qt::black);
			p->drawLine(x+midX, y+2, x+midX, h-y+1);
	} else {
			int midY = h/2;

			p->setPen(g.light());
			p->drawLine(x+2, y+midY-1, w-x+1, y+midY-1);
			p->setPen(Qt::black);
			p->drawLine(x+2, y+midY, w-x+1, y+midY);
	}
		
		p->setPen(oldPen);

}
void 
KSgiStyle::drawCheckMark( QPainter *p, int x, int y, int w, int h,
									const QColorGroup&, bool, bool)
{
	p->drawPixmap (x+(w-16)/2, y+(h-16)/2, *checkPix);
}

void 
KSgiStyle::drawScrollBarControls(QPainter *p, const QScrollBar *sb,
                                       int sliderStart, uint controls,
                                       uint)
{
	int sliderMin, sliderMax, sliderLength, buttonDim;
  QRect add, sub, addPage, subPage, slider;
  int 				addX, addY, subX, subY;
  bool 				horizontal = sb->orientation() == QScrollBar::Horizontal;
  int 				len = (horizontal) ? sb->width() : sb->height();
  int 				extent = (horizontal) ? sb->height() : sb->width();
  QColorGroup g = sb->colorGroup();

	int 	w = sb->width();
	int 	h = sb->height();
	int		b = SB_BORDER_WIDTH;
		
  scrollBarMetrics(sb, sliderMin, sliderMax, sliderLength, buttonDim);
  if(sliderStart > sliderMax)
        sliderStart = sliderMax;

  int sliderEnd = sliderStart + sliderLength;
	int	sliderWidth = extent - b*2;
	
  if(horizontal){
        subY = addY = (extent - buttonDim)/2;
        subX = b;
        addX = len - buttonDim - b;
				
        subPage.setRect(buttonDim+1, b, sliderStart-buttonDim-1, sliderWidth);
        addPage.setRect(sliderEnd, b, addX-sliderEnd, sliderWidth);
        slider.setRect(sliderStart, b, sliderLength, sliderWidth);
  } else { 
        subX = addX = (extent - buttonDim)/2;
        subY = b;
        addY = len-buttonDim-b;

 
        subPage.setRect(b, buttonDim+1, sliderWidth, sliderStart-buttonDim-1);
        addPage.setRect(b, sliderEnd, sliderWidth, addY-sliderEnd);
        slider.setRect(b, sliderStart, sliderWidth, sliderLength);
  }
  
	sub.setRect(subX, subY, buttonDim, buttonDim);
  add.setRect(addX, addY, buttonDim, buttonDim);

	if (controls) {
			p->setPen (g.mid());
			p->drawLine(0, 0, w-2, 0);
			p->drawLine(0, 0, 0, h-2);
			p->setPen(g.background());
			p->drawLine(w-1, 0, w-1, h-1);
			p->drawLine(0, h-1, w-1, h-1);
	}
		
  if(controls & AddLine){
  	if(add.isValid()){
			if (horizontal) {
				int x2 = add.x() + add.width()-1;
				int y2 = add.y() + add.height()-1;
				
				p->setPen(g.dark());
				p->drawLine(add.x(), add.y()+1, add.x(), y2-1);

				p->setPen(Qt::black);
				p->drawLine(add.x(), y2, x2, y2);
				p->drawLine(add.x(), add.y(), x2, add.y());
				p->drawLine(x2, add.y(), x2, y2);
				drawFullShadeButton(p, add.x()+1, add.y()+1, add.width()-2, add.height()-2, g);
			} else {
				int x2 = add.x() + add.width()-1;
				int y2 = add.y() + add.height()-1;
				
				p->setPen(g.dark());
				p->drawLine(add.x()+1, add.y(), x2-1, add.y());

				p->setPen(Qt::black);
				p->drawLine(add.x(), y2, x2, y2);
				p->drawLine(add.x(), add.y(), add.x(), y2);
				p->drawLine(x2, add.y(), x2, y2);
				drawFullShadeButton(p, add.x()+1, add.y()+1, add.width()-2, add.height()-2, g);
 			}
			
	     drawScrollBarArrow(p, (horizontal) ? RightArrow : DownArrow, add.x()+4,
                             add.y()+4, g);
   }
  }
    
	if(controls & SubLine){
  	if(sub.isValid()){
			int x2 = sub.x() + sub.width()-1;
			int y2 = sub.y() + sub.height()-1;
	 			
			if (horizontal) {	
				p->setPen(g.dark());
				p->drawLine(x2, sub.y()+1, x2, y2);

				p->setPen(Qt::black);
				p->drawLine(sub.x(), sub.y(), x2, sub.y());
				p->drawLine(sub.x(), sub.y(), sub.x(), y2);
				p->drawLine(sub.x(), y2, x2, y2);
				drawFullShadeButton(p, sub.x()+1, sub.y()+1, sub.width()-2, sub.height()-2, g);
			} else {
					p->setPen(g.dark());
				p->drawLine(sub.x()+1, y2, x2-1, y2);

				p->setPen(Qt::black);
				p->drawLine(sub.x(), sub.y(), x2, sub.y());
				p->drawLine(sub.x(), sub.y(), sub.x(), y2);
				p->drawLine(x2, sub.y(), x2, y2);
				drawFullShadeButton(p, sub.x()+1, sub.y()+1, sub.width()-2, sub.height()-2, g);
 			}
			
	     drawScrollBarArrow(p, (horizontal) ? LeftArrow : UpArrow, sub.x()+4,
                             sub.y()+4, g);
    }
	}
 
  if((controls & SubPage) && subPage.isValid()){
		int x2 = subPage.x()+subPage.width()-1;
		int y2 = subPage.y()+subPage.height()-1;
		
		if (horizontal) {
			p->fillRect(subPage.x()+1, subPage.y()+2, subPage.width()-1, subPage.height()-4,
								  g.brush(QColorGroup::Background));
			p->setPen(Qt::black);
			p->drawLine(subPage.x(), subPage.y(), x2, subPage.y());
			p->drawLine(subPage.x(), y2, x2, y2);

			p->setPen(g.midlight());
			p->drawLine(subPage.x(), subPage.y()+1, subPage.x(), y2-1);
			p->drawLine(subPage.x(), subPage.y()+1, x2, subPage.y()+1);
			p->setPen(g.mid());
			p->drawLine(subPage.x(), y2-1, x2, y2-1);
		} else {
			p->fillRect(subPage.x()+2, subPage.y()+1, subPage.width()-4, subPage.height()-1,
								  g.brush(QColorGroup::Background));
			p->setPen(Qt::black);
			p->drawLine(subPage.x(), subPage.y(), subPage.x(), y2);
			p->drawLine(x2, subPage.y(), x2, y2);

			p->setPen(g.midlight());
			p->drawLine(subPage.x()+1, subPage.y(), subPage.x()+1, y2);
			p->drawLine(subPage.x()+1, subPage.y(), x2-2, subPage.y());
			p->setPen(g.mid());
			p->drawLine(x2-1, subPage.y(), x2-1, y2);
		}
	}
	
	if((controls & AddPage) && addPage.isValid()){
		int x2 = addPage.x()+addPage.width()-1;
		int y2 = addPage.y()+addPage.height()-1;
		
		if (horizontal) {
			p->fillRect(addPage.x(), addPage.y()+2, addPage.width()-1, addPage.height()-4,
								  g.brush(QColorGroup::Background));
			p->setPen(Qt::black);
			p->drawLine(addPage.x(), addPage.y(), x2, addPage.y());
			p->drawLine(addPage.x(), y2, x2, y2);

			p->setPen(g.midlight());
			p->drawLine(addPage.x(), addPage.y()+1, x2, addPage.y()+1);
			p->setPen(g.mid());
			p->drawLine(x2, addPage.y(), x2, y2-1);
			p->drawLine(addPage.x(), y2-1, x2, y2-1);
		} else {
			p->fillRect(addPage.x()+2, addPage.y(), addPage.width()-4, addPage.height()-1,
								  g.brush(QColorGroup::Background));
			p->setPen(Qt::black);
			p->drawLine(addPage.x(), addPage.y(), addPage.x(), y2);
			p->drawLine(x2, addPage.y(), x2, y2);

			p->setPen(g.midlight());
			p->drawLine(addPage.x()+1, addPage.y(), addPage.x()+1, y2);
			p->setPen(g.mid());
			p->drawLine(x2-1, addPage.y(), x2-1, y2);
			p->drawLine(addPage.x()+1, y2, x2-2, y2);
		}
 	}

	if(controls & Slider){
  	if(slider.isValid() && slider.width() > 1 && slider.height() > 1){
	   	if(horizontal){
				int y2 = slider.y() + slider.height()-1;
				int midX = slider.width()/2 + slider.x()-1;
				
				p->setPen(Qt::black);
  			p->drawRect(slider.x(), slider.y(), slider.width(), slider.height()); 
       	drawPartShadeButton(p, slider.x()+1, slider.y()+1, slider.width()-2,
                           slider.height()-2, g, 0, &(g.brush(QColorGroup::Background)));

				//
				// draw hash marks on slider
				//
				
				p->setPen(g.light());
				p->drawLine(midX-4, slider.y()+2, midX-4, y2-2);
				p->drawLine(midX  , slider.y()+2, midX  , y2-2);
				p->drawLine(midX+4, slider.y()+2, midX+4, y2-2);
				p->setPen(Qt::black);
				p->drawLine(midX-3, slider.y()+2, midX-3, y2-2);
				p->drawLine(midX+1, slider.y()+2, midX+1, y2-2);
				p->drawLine(midX+5, slider.y()+2, midX+5, y2-2);
      } else {
				int x2 = slider.x() + slider.width()-1;
				int midY = slider.height()/2 + slider.y()-1;
				
				p->setPen(Qt::black);
  			p->drawRect(slider.x(), slider.y(), slider.width(), slider.height()); 
       	drawPartShadeButton(p, slider.x()+1, slider.y()+1, slider.width()-2,
                           slider.height()-2, g, 0, &(g.brush(QColorGroup::Background)));
				
				//
				// draw hash marks on slider
				//
				
				p->setPen(g.light());
				p->drawLine(slider.x()+2, midY-4, x2-2, midY-4);
				p->drawLine(slider.x()+2, midY,   x2-2, midY);
				p->drawLine(slider.x()+2, midY+4, x2-2, midY+4);
				p->setPen(Qt::black);
				p->drawLine(slider.x()+2, midY-3, x2-2, midY-3);
				p->drawLine(slider.x()+2, midY+1, x2-2, midY+1);
				p->drawLine(slider.x()+2, midY+5, x2-2, midY+5);
      }
 
    }
  }
}

void 
KSgiStyle::scrollBarMetrics(const QScrollBar *sb, int &sliderMin,
                                  int &sliderMax, int &sliderLength,
                                  int &buttonDim)
{
	int maxLength;
  bool horizontal = sb->orientation() == QScrollBar::Horizontal;
  int len = (horizontal) ? sb->width() : sb->height();
  int extent = (horizontal) ? sb->height() : sb->width();
	int	b = SB_BORDER_WIDTH;	// border around sbar
		
  if(len > (extent - 1)*2)
        buttonDim = extent - b*2;
  else
        buttonDim = (len - b*2)/2 - 1;

  sliderMin = buttonDim + b;
	maxLength = len - b*2 - buttonDim*2;

	if ( sb->maxValue() == sb->minValue() ) {
			sliderLength = maxLength;
  } else {
			sliderLength = (sb->pageStep()*maxLength)/
											(sb->maxValue()-sb->minValue()+sb->pageStep());
			uint range = sb->maxValue()-sb->minValue();
	if ( sliderLength < SLIDER_MIN || range > INT_MAX/2 )
	    sliderLength = SLIDER_MIN;
	if ( sliderLength > maxLength )
	    sliderLength = maxLength;
  }
  
	sliderMax = sliderMin + maxLength - sliderLength;
}


QStyle::ScrollControl 
KSgiStyle::scrollBarPointOver(const QScrollBar *sb, int sliderStart,
                                                     const QPoint &p)
{
	if(!sb->rect().contains(p))
  	return(NoScroll);
  
	int sliderMin, sliderMax, sliderLength, buttonDim;
  scrollBarMetrics(sb, sliderMin, sliderMax, sliderLength, buttonDim);

  if(sb->orientation() == QScrollBar::Horizontal){
  	int x = p.x();
    if(x < buttonDim)
    	return(SubLine);
		else if (x < sliderStart) 
			return(SubPage);
    else if(x < sliderStart+sliderLength)
      return(Slider);
    else if(x <sliderMax+sliderLength)
       return(AddPage);
  
	  return(AddLine);
  } else{
    int y = p.y();
    if(y < buttonDim)
    	return(SubLine);
		else if (y < sliderStart) 
			return(SubPage);
    else if(y < sliderStart+sliderLength)
      return(Slider);
    else if(y <sliderMax+sliderLength)
      return(AddPage);
    
		return(AddLine);
  }
}


void 
KSgiStyle::drawScrollBarArrow(QPainter *p, Qt::ArrowType type, int x,
                                  int y, const QColorGroup &g)
{
	static QBitmap upArrow(9, 9, uparrow_bits, true);
  static QBitmap downArrow(9, 9, downarrow_bits, true);
  static QBitmap leftArrow(9, 9, leftarrow_bits, true);
  static QBitmap rightArrow(9, 9, rightarrow_bits, true);

  if(!upArrow.mask()){
  	upArrow.setMask(upArrow);
    downArrow.setMask(downArrow);
    leftArrow.setMask(leftArrow);
    rightArrow.setMask(rightArrow);
  }
  
	p->setPen(g.dark());
  switch(type){
    case Qt::UpArrow:
        p->drawPixmap(x, y, upArrow);
        break;
    case Qt::DownArrow:
        p->drawPixmap(x, y, downArrow);
        break;
    case Qt::LeftArrow:
        p->drawPixmap(x, y, leftArrow);
        break;
    default:
        p->drawPixmap(x, y, rightArrow);
        break;
	}


}

int 
KSgiStyle::splitterWidth() const
{
 	return 6;
}

void 
KSgiStyle::drawSplitter( QPainter *p,  int x, int y, int w, int h,
				  const QColorGroup &g,  Orientation)
{
	drawFullShadeButton( p, x, y, w, h, g );
}
