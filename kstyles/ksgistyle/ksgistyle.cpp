//--------------------------------------------------------------------
//
//  

#ifndef INCLUDE_MENUITEM_DEF
#define INCLUDE_MENUITEM_DEF
#endif

#include "ksgistyle.h"
#include <qpainter.h>
#include <qwidget.h>
#include <kapp.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <qbitmap.h>
#include <qdrawutil.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qpopupmenu.h>
#include <qmenudata.h>

#include <limits.h>

#include <stdio.h>

#define SB_BORDER_WIDTH 0
#define SLIDER_MIN	22

//--------------------------------------------------------------------
//
//	static defines
//

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

static const unsigned char check_mask[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0xfe, 0x10, 0x3f, 
  0xb8, 0x07, 0xfc, 0x03, 0xfc, 0x01, 0xf0, 0x01, 0xe0, 0x00, 0x40, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static const unsigned char downarrow_bits[] = {
 0xff,0x01,0xff,0x01,0xfe,0x00,0xfe,0x00,0x7c,0x00,0x7c,0x00,0x38,0x00,0x38,
 0x00,0x10,0x00};

static const unsigned char leftarrow_bits[] = {
 0x80,0x01,0xe0,0x01,0xf8,0x01,0xfe,0x01,0xff,0x01,0xfe,0x01,0xf8,0x01,0xe0,
 0x01,0x80,0x01};

static const unsigned char rightarrow_bits[] = {
 0x03,0x00,0x0f,0x00,0x3f,0x00,0xff,0x00,0xff,0x01,0xff,0x00,0x3f,0x00,0x0f,
 0x00,0x03,0x00};

static const unsigned char uparrow_bits[] = {
  0x10, 0x00, 0x38, 0x00, 0x38, 0x00, 0x7c, 0x00, 0x7c, 0x00, 0xfe, 0x00,
  0xfe, 0x00, 0xff, 0x01, 0xff, 0x01};

//--------------------------------------------------------------------
//
//	KSgiStyle - Contstructor
//

      
KSgiStyle::KSgiStyle()
	:KStyle()
{
  //
  // set some default sizes
  //
  setScrollBarExtent(19, 19);
  setButtonDefaultIndicatorWidth (0);
 
	
}

//--------------------------------------------------------------------
//
//	KSgiStyle - Destructor
//

      

KSgiStyle::~KSgiStyle()
{
}

//--------------------------------------------------------------------
//
//	polish - set up widget
//

void
KSgiStyle::polish(QWidget *w)
{
  if (w->isTopLevel())
    return;
		
  if(w->inherits("QButton") || w->inherits("QComboBox")){
    w->installEventFilter(this);
    QPalette pal = w->palette();
    // we use this as a flag since it's otherwise unused.
    pal.setColor(QColorGroup::Shadow, Qt::white);
    w->setPalette(pal);
  } 

}

//--------------------------------------------------------------------
//
//	unpolish - un set up widget
//

      
void 
KSgiStyle::unPolish(QWidget *w)
{
  if (w->isTopLevel()) 
  	return;

  if(w->inherits("QButton") || w->inherits("QComboBox")){
    w->removeEventFilter(this);
    QPalette pal = w->palette();
    pal.setColor(QColorGroup::Shadow,
                     kapp->palette().active().color(QColorGroup::Shadow));
    w->setPalette(pal);
  } 
}

//--------------------------------------------------------------------
//
//	eventFilter - style defined event filter. Used here to do 
//		      highlight on mouse over events
//
     
bool
KSgiStyle::eventFilter(QObject* obj, QEvent* ev)
{
  if(obj->inherits("QButton") || obj->inherits("QComboBox")){
    if (ev->type() == QEvent::Enter) {
    	QWidget *btn = (QWidget *)obj;
        if (btn->isEnabled()){
     	  QPalette pal = btn->palette();
	  palStore = pal;
          pal.setColor(QColorGroup::Shadow,
                     pal.active().color(QColorGroup::Midlight));
          btn->setPalette(pal);

        }
    } else if (ev->type() == QEvent::Leave) {
        QWidget *btn = (QWidget *)obj;
        QPalette pal = btn->palette();
        pal.setColor(QColorGroup::Shadow, Qt::white);
        btn->setPalette(pal);
    }
  } 
	
  return(false);
}


//--------------------------------------------------------------------
//
//	drawButton - used by various UI components to draw button-like things
//

void 
KSgiStyle::drawButton(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, bool sunken, const QBrush* fill)
{
  QPen 	oldPen = p->pen();
  int	x2 = x+w-1;
  int 	y2 = y+h-1;
	
  //
  // draw outline
  //
  p->setPen (sunken ? g.dark() : Qt::black);
  p->drawLine(x, y2, x2, y2);
  p->drawLine(x2, y, x2, y2);
  p->setPen(sunken ?  Qt::black : g.dark());
  p->drawLine(x, y, x2, y);
  p->drawLine(x, y, x, y2);
	
  if (g.shadow() == Qt::white) {
	drawFullShadeButton(p, x+1, y+1, w-2, h-2, g, sunken, fill);
  } else {
	drawFullShadeButton(p, x+1, y+1, w-2, h-2, g, sunken,
			    &g.brush(QColorGroup::Midlight));
  }	
  p->setPen(oldPen);
}

//--------------------------------------------------------------------
//
//	drawFullShadeButton - draws a button with 2 levels of shading
//

void
KSgiStyle::drawFullShadeButton(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, bool sunken, const QBrush* fill)
{
  int x2 = x+w-1;
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

//--------------------------------------------------------------------
//
//	drawPartShadeButton - draws a button with 1 level of shading
//

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

//--------------------------------------------------------------------
//
//	drawBevelButton - draws a simple beveled button, used in scroll
//			lists
//

void 
KSgiStyle::drawBevelButton( QPainter *p, int x, int y, int w, int h,
				const QColorGroup &g, bool sunken, const QBrush* fill)
{
    drawFullShadeButton(p, x, y, w, h, g, sunken, fill);
}

//--------------------------------------------------------------------
//
//	drawPushButton - draws a standard push button. This is the
//			implementation that QPushbutton would call.
//

void 
KSgiStyle::drawPushButton(QPushButton *btn, QPainter *p)
{
    QRect r = btn->rect();
    bool sunken = btn->isOn() || btn->isDown();
    QColorGroup g = btn->colorGroup();
    
    drawButton(p, r.x(), r.y(), r.width(), r.height(), g, sunken);
}


//--------------------------------------------------------------------
//
//	buttonRect - return area in a button available for text
//

QRect 
KSgiStyle::buttonRect(int x, int y, int w, int h)
{
    return(QRect(x+2, y+2, w-4, h-4));
}

//--------------------------------------------------------------------
//
// indicatorSize - return fixed size of an indicator
//

QSize
KSgiStyle::indicatorSize() const
{
    return(QSize(16, 16));
}


//--------------------------------------------------------------------
//
// drawIndicator - draws a standard indicator, called directly by
//

void
KSgiStyle::drawIndicator(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, int state, bool down, 
                               bool enabled)
{

  QPen oldPen = p->pen();
  int x2 = x+w-1;
  int y2 = y+h-1;
  
	
  
   if (enabled) {
	
    //
    // draw outline
    //
    p->setPen (Qt::black);
    p->drawLine(x, y2, x2, y2);
    p->drawLine(x2, y, x2, y2);
    p->setPen(g.dark());
    p->drawLine(x, y, x2, y);
    p->drawLine(x, y, x, y2);
    
    //
    // shouldn't have to do this, I thought that was what mask was for.
    // Clear the bg....
    //
    p->fillRect(x+1, y+1, w-2, h-2, g.brush(QColorGroup::Background));
  
  
	
    if (g.shadow() == Qt::white) {
  	  drawPartShadeButton(p, x+1, y+1, w-2, h-2, g, down, &(g.brush(QColorGroup::Background)));
    } else {
	  drawPartShadeButton(p, x+1, y+1, w-2, h-2, g, down, &(g.brush(QColorGroup::Midlight)));
    }

  } else {
    //
    // draw outline
    //
    p->setPen(g.dark());
    p->drawRect(x, y, w, h);

    p->fillRect(x+1, y+1, w-2, h-2, g.mid());
    
  }
  
  p->setPen(oldPen);
    
  //
  // red check mark
  //
	
  if (state)
    drawCheckMark(p, x, y+2, 16, 16, g);
}

//--------------------------------------------------------------------
//
// drawIndicatorMask - draw indicator as mask 
//

void
KSgiStyle::drawIndicatorMask(QPainter *p, int x, int y, int w, int h, 
                              int )
{
  p->fillRect(x, y, w, h, Qt::color1);
}

//--------------------------------------------------------------------
//
// drawCheckMark - draw red check mark
//

void 
KSgiStyle::drawCheckMark( QPainter *p, int x, int y, int w, int h,
			const QColorGroup&, bool, bool)
{
  //
  // This should only happen the first time through to set the mask
  //
  if (checkPix.isNull()) {
    checkPix = QPixmap(check_xpm);
    checkMask = QBitmap(16, 16, check_mask, true);
    checkPix.setMask(checkMask);
  }

  p->drawPixmap (x+(w-16)/2, y+(h-16)/2, checkPix);
}

//--------------------------------------------------------------------
//
// exclusiveIndicatorSize - return fixed size of an exclusive indicator
//

QSize 
KSgiStyle::exclusiveIndicatorSize() const
{
  return(QSize(16,16));
}

//--------------------------------------------------------------------
//
// drawExclusiveIndicator - draw a standard exclusive indicator
//

void
KSgiStyle::drawExclusiveIndicator(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, bool on, bool, bool enabled)
{
static const unsigned char exclusive_outline_bits[] = {
  0x80, 0x01, 0x40, 0x02, 0x20, 0x04, 0x10, 0x08, 0x08, 0x10, 0x04, 0x20,
  0x02, 0x40, 0x01, 0x80, 0x01, 0x80, 0x06, 0x60, 0x0c, 0x30, 0x18, 0x18,
  0x30, 0x0c, 0x60, 0x06, 0xc0, 0x03, 0x80, 0x01, };
static const unsigned char exclusive_fill_bits[] = {
  0x00, 0x00, 0x80, 0x01, 0x40, 0x02, 0xa0, 0x05, 0xd0, 0x0b, 0xe8, 0x17,
  0xf4, 0x2f, 0xfa, 0x5f, 0xfe, 0x7f, 0xf8, 0x1f, 0xf0, 0x0f, 0xe0, 0x07,
  0xc0, 0x03, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, };
static const unsigned char exclusive_hilite_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x40, 0x02, 0x20, 0x04, 0x10, 0x08,
  0x08, 0x10, 0x04, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
static const unsigned char exclusive_arrow_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x07,
  0x00, 0x0f, 0x00, 0x0f, 0x00, 0x07, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
static const unsigned char exclusive_shadow_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x00, 0x04, 0x00, 0x02, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
static const unsigned char exclusive_inactfill_bits[] = {
  0x00, 0x00, 0x80, 0x01, 0xc0, 0x03, 0xe0, 0x07, 0xf0, 0x0f, 0xf8, 0x1f,
  0xfc, 0x3f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfc, 0x3f, 0xf8, 0x1f, 0xf0, 0x0f,
  0xe0, 0x07, 0xc0, 0x03, 0x80, 0x01, 0x00, 0x00, };

  if (outlineBitmap.isNull())
  {
     outlineBitmap = QBitmap(16, 16, exclusive_outline_bits, true);
     fillBitmap = QBitmap(16, 16, exclusive_fill_bits, true);
     inactfillBitmap = QBitmap(16, 16, exclusive_inactfill_bits, true);
     hiliteBitmap = QBitmap(16, 16, exclusive_hilite_bits, true);
     arrowBitmap = QBitmap(16, 16, exclusive_arrow_bits, true);
     shadowBitmap = QBitmap(16, 16, exclusive_shadow_bits, true);
     
     outlineBitmap.setMask(outlineBitmap);
     fillBitmap.setMask(fillBitmap);
     inactfillBitmap.setMask(inactfillBitmap);
     hiliteBitmap.setMask(hiliteBitmap);
     arrowBitmap.setMask(arrowBitmap);
     shadowBitmap.setMask(shadowBitmap);
  }
 
  QPen oldPen = p->pen();
	
  p->fillRect( x, y, w, h, g.brush(QColorGroup::Background));

  
  p->setPen(g.dark());
  p->drawPixmap(x, y, outlineBitmap);
  
  if (enabled) {
    p->setPen((g.shadow() == Qt::white) ? g.background() : g.midlight());
    p->drawPixmap(x, y, fillBitmap);
    p->setPen(g.light());
    p->drawPixmap(x, y, hiliteBitmap);
  } else {
    p->setPen(g.mid());
    p->drawPixmap(x, y, inactfillBitmap);
  }
  
  
  if (on) {
    p->setPen(Qt::blue);
    p->drawPixmap(x, y, arrowBitmap);
    p->setPen(Qt::black);
    p->drawPixmap(x, y, shadowBitmap);
  }
	
  p->setPen(oldPen);
}

//--------------------------------------------------------------------
//
// comboButtonRect - specify where the contents of a combo button
//		     button pulldown can live.
//

QRect 
KSgiStyle::comboButtonRect (int x, int y, int w, int h)
{
  return(QRect(x+3, y+3, w-24, h-6));

}		

//--------------------------------------------------------------------
//
// comboButtonFocusRect - specify where the text of a combo button
//

QRect 
KSgiStyle::comboButtonFocusRect (int x, int y, int w, int h)
{
  return(QRect(x+3, y+3, w-22, h-6));
}

//--------------------------------------------------------------------
//
// drawComboButton - draw the actual combo button
//

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
    int x2 = w-x-20-1;
    int y2 = h-y-2-1;
    p->setPen(g.dark());
    p->drawLine(x+2, y+2, x2, y+2);
    p->drawLine(x+2, y+3, x+2, y2);
    p->setPen(g.midlight());
    p->drawLine(x+3, y2, x2, y2);
    p->drawLine(x2, y+3, x2, y2);
  
    //
    // normal dark shadow looks too dark in conjunction with the
    // black outline here so lighten it up under the editable section
    //
    
    p->setPen(g.mid());
    p->drawLine(x+2, y2+1, x2-1, y2+1);
  }
 	
  p->setPen(oldPen);
}

//--------------------------------------------------------------------
//
// slider Length - minimum slider length
//

int
KSgiStyle::sliderLength() const
{
  return(30);
}

//--------------------------------------------------------------------
//
// drawSliderGroove - draw the trench for the slider (not scrollbar)
//

void 
KSgiStyle::drawSliderGroove(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, QCOORD,
                                    Orientation)
{
  QPen oldPen = p->pen();
  int x2 = x+w-1;
  int y2 = y+h-1;

  //
  // frame in
  //
  
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

  //
  // inner black outline
  //
  
  p->setPen(Qt::black);
  p->drawLine(x+2, y+2, x2-2, y+2);
  p->drawLine(x+2, y+2, x+2, y2-2);
  p->drawLine(x2-2, y+2, x2-2, y2-2);
  p->drawLine(x+2, y2-2, x2-2, y2-2);

  //
  // inner raised groove
  //
  
  p->setPen(g.midlight());
  p->drawLine(x+3, y+3, x2-3, y+3);
  p->drawLine(x+3, y+3, x+3, y2-3);
  p->setPen(g.mid());
  p->drawLine(x2-3, y+3, x2-3, y2-3);
  p->drawLine(x+3, y2-3, x2-3, y2-3);

  //
  // fill trough
  //
  
  p->fillRect(x+4, y+4, w-8, h-8, g.brush(QColorGroup::Background));
  p->setPen(oldPen);
}

//--------------------------------------------------------------------
//
// drawSlider - draw the actual slider button
//

void 
KSgiStyle::drawSlider(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, Orientation orient,
                            bool, bool)
{
  x += 2;
  y += 2;
  w -= 4;
  h -= 4;

  int x2 = x+w-1;
  int y2 = y+h-1;

  QPen oldPen = p->pen();
	
  //
  // draw outline
  //
  p->setPen (Qt::black);
  p->drawLine(x, y, x, y2);
  p->drawLine(x2, y, x2, y2);
	
  //
  // slider button
  //
  
  drawPartShadeButton(p, x+1, y+1, w-2, h-2, g);
  p->fillRect(x+3, y+3, w-6, h-6, g.brush(QColorGroup::Background));

 	
	
  //
  // draw notch in middle
  //		
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

//--------------------------------------------------------------------
//
// drawScrollBarControls - draw the scroll bar controls (up and down
//			   buttons, trench and slider)
//			FIXME want to find a way to hilight on mouse over
//

void 
KSgiStyle::drawScrollBarControls(QPainter *p, const QScrollBar *sb,
                                  int sliderStart, uint controls, uint)
{
  int	      sliderMin, sliderMax, sliderLength, buttonDim;
  QRect       add, sub, addPage, subPage, slider; // rects for buttons & trough
  int 	      addX, addY, subX, subY;		// dimensions for buttons
  bool 	      horizontal = sb->orientation() == QScrollBar::Horizontal;
  int 	      len = (horizontal) ? sb->width() : sb->height();
  int 	      extent = (horizontal) ? sb->height() : sb->width();
  int		  b = SB_BORDER_WIDTH;		// Optional border around scrollbar
  QColorGroup g = sb->colorGroup();
		
	//
	// find dimensions for scroll bar objects
	//
	
  scrollBarMetrics(sb, sliderMin, sliderMax, sliderLength, buttonDim);
  if(sliderStart > sliderMax)
        sliderStart = sliderMax;

  int sliderEnd = sliderStart + sliderLength;
  int	sliderWidth = extent - b*2;
	
	//
	// compute bounding areas of individual controls
	//
	
  if(horizontal){
        subY = addY = (extent - buttonDim)/2;
        subX = b;
        addX = len - buttonDim - b;
				
        subPage.setRect(buttonDim+b, b, sliderStart-buttonDim-b, sliderWidth);
        addPage.setRect(sliderEnd, b, addX-sliderEnd, sliderWidth);
        slider.setRect(sliderStart, b, sliderLength, sliderWidth);
  } else { 
        subX = addX = (extent - buttonDim)/2;
        subY = b;
        addY = len-buttonDim-b;

 
        subPage.setRect(b, buttonDim+b, sliderWidth, sliderStart-buttonDim-b);
        addPage.setRect(b, sliderEnd, sliderWidth, addY-sliderEnd);
        slider.setRect(b, sliderStart, sliderWidth, sliderLength);
  }
  
  sub.setRect(subX, subY, buttonDim, buttonDim);
  add.setRect(addX, addY, buttonDim, buttonDim);
	
  //
  // Draw add line button
  //
		
  if(controls & AddLine){
  	if(add.isValid()){
			int x2 = add.x() + add.width()-1;
			int y2 = add.y() + add.height()-1;
				
			p->setPen(Qt::black);
			p->drawLine(add.x(), y2, x2, y2);
			p->drawLine(x2, add.y(), x2, y2);
			
			if (horizontal) {
				p->drawLine(add.x(), add.y(), x2, add.y());
				p->setPen(g.dark());
				p->drawLine(add.x(), add.y()+1, add.x(), y2-1);
			} else {				
				p->drawLine(add.x(), add.y(), add.x(), y2);
				p->setPen(g.dark());
				p->drawLine(add.x()+1, add.y(), x2-1, add.y());
 			}
			
			drawFullShadeButton(p, add.x()+1, add.y()+1, add.width()-2, add.height()-2, g);
	        drawScrollBarArrow(p, (horizontal) ? RightArrow : DownArrow, add.x()+4,
                             add.y()+4, g);
   	}
  }
  
  //
  // Draw subtract line button
  //
	  
  if(controls & SubLine){
  	if(sub.isValid()){
		int x2 = sub.x() + sub.width()-1;
		int y2 = sub.y() + sub.height()-1;
	 		
		//
		// common outline between horizontal and vertical button
		//
				
		p->setPen(Qt::black);
		p->drawLine(sub.x(), sub.y(), x2, sub.y());
		p->drawLine(sub.x(), sub.y(), sub.x(), y2);
			
		if (horizontal) {	
			p->drawLine(sub.x(), y2, x2, y2);
			p->setPen(g.dark());
			p->drawLine(x2, sub.y()+1, x2, y2-1);
		} else {
			p->drawLine(x2, sub.y(), x2, y2);
			p->setPen(g.dark());
			p->drawLine(sub.x()+1, y2, x2-1, y2);
 		}
			
		drawFullShadeButton(p, sub.x()+1, sub.y()+1, sub.width()-2, sub.height()-2, g);
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
			p->drawLine(x2, addPage.y()+2, x2, y2-1);
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

	//
	// draw slider component
	//
	
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


//--------------------------------------------------------------------
//
// scrollBarMetrics - compute where controls are drawn
//

void 
KSgiStyle::scrollBarMetrics(const QScrollBar *sb, int &sliderMin,
                                  int &sliderMax, int &sliderLength,
                                  int &buttonDim)
{
  int 	maxLength;
  bool 	horizontal 	= sb->orientation() == QScrollBar::Horizontal;
  int 	len 		= (horizontal) ? sb->width() : sb->height();
  int 	extent 		= (horizontal) ? sb->height() : sb->width();
  int	b 		    = SB_BORDER_WIDTH;	// border around sbar
		
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


//--------------------------------------------------------------------
//
// scrollBarPointOver - determine where the pointer is before drawing
//

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


//--------------------------------------------------------------------
//
// drawScrollBarArrow - draw the arrows for scroll bar
//	

void 
KSgiStyle::drawScrollBarArrow(QPainter *p, Qt::ArrowType type, int x,
                                  int y, const QColorGroup &g)
{
  if (upArrow.isNull())
  {
    upArrow = QBitmap(9, 9, uparrow_bits, true);
    downArrow = QBitmap(9, 9, downarrow_bits, true);
    leftArrow = QBitmap(9, 9, leftarrow_bits, true);
    rightArrow = QBitmap(9, 9, rightarrow_bits, true);
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

//--------------------------------------------------------------------
//
// splitterWidth - return width a splitter should be drawn
//	

int 
KSgiStyle::splitterWidth() const
{
  return 6;
}

//--------------------------------------------------------------------
//
// drawSplitter - draw the splitter
//	

void 
KSgiStyle::drawSplitter( QPainter *p,  int x, int y, int w, int h,
				  const QColorGroup &g,  Orientation)
{
  drawFullShadeButton( p, x, y, w, h, g );
}

//--------------------------------------------------------------------
//
//	drawPopupPanel - draws a popup panel, used for menus
//

void
KSgiStyle::drawPopupPanel(QPainter *p, int x, int y, int w, int h,
			            const QColorGroup &g,  int lineWidth, const QBrush *fill )
{
  QPen oldPen = p->pen();
  p->setPen(Qt::black);
  p->drawRect(x, y, w, h);
  if (lineWidth <= 2) {
    drawPartShadeButton(p, x+1, y+1, w-2, h-2, g, 0, fill);
  } else {
    drawFullShadeButton(p, x+1, y+1, w-2, h-2, g, 0, fill);
  }		
  
  p->setPen(oldPen);
}

//--------------------------------------------------------------------
//
//	drawPanel - draws a panel
//

void
KSgiStyle::drawPanel(QPainter *p, int x, int y, int w, int h,
		const QColorGroup &g, bool sunken, 
		int /*lineWidth*/, const QBrush* fill)
{
//	if (lineWidth <= 2) {
//		drawPartShadeButton(p, x, y, w, h, g, sunken, fill);
//	} else {
		drawFullShadeButton(p, x, y, w, h, g, sunken, fill);
//	}
}

//--------------------------------------------------------------------
//
//	drawKToolBar
//

void KSgiStyle::drawKToolBar(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, KToolBarPos,
                          QBrush*)
{
   qDrawShadePanel(p, x, y, w, h, g, false, 1,
                    &g.brush(QColorGroup::Background));
}

void KSgiStyle::drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, bool, QBrush*)
{
//  qDrawShadePanel(p, x, y, w, h, g, false, 1,
//                        &g.brush(QColorGroup::Background));
  drawFullShadeButton (p, x, y, w, h, g, false,
                   &g.brush(QColorGroup::Background));
}


void 
KSgiStyle::drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, bool sunken,
                                    bool raised, bool enabled, bool popup,
                                    KToolButtonType icontext,
                                    const QString& btext, const QPixmap *pixmap,
                                    QFont *font, QWidget *)
{
  int dx, dy;

  QFontMetrics fm(*font);

  if(raised || sunken){
    p->setPen(Qt::black);
    p->drawRect(x, y, w, h);
    drawFullShadeButton(p, x+1, y+1, w-2, h-2, g, sunken);
        
  } else {
    p->fillRect(x, y, w, h, g.brush(QColorGroup::Background));
  }
  
  p->setPen(g.buttonText());

  if (icontext == Icon){ // icon only
      if (pixmap){
          dx = ( w - pixmap->width() ) / 2;
          dy = ( h - pixmap->height() ) / 2;
          if ( sunken ) {
                ++dx;
                ++dy;
          }
          p->drawPixmap( x+dx, y+dy, *pixmap );
      }
  } else if (icontext == IconTextRight){ // icon and text (if any)
      if (pixmap){
          dx = 4;
          dy = ( h - pixmap->height() ) / 2;
          if ( sunken ){
              ++dx;
              ++dy;
          }
          p->drawPixmap( x+dx, y+dy, *pixmap );
      }
      
      if (!btext.isNull()){
            int tf = AlignVCenter|AlignLeft;
            if (pixmap)
                dx= 4 + pixmap->width() + 2;
            else
                dx= 4;
            dy = 0;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(KGlobalSettings::toolBarHighlightColor());
            p->drawText(x+dx, y+dy, w-dx, h, tf, btext);
        }
  } else if (icontext == Text){ // only text, even if there is a icon
        if (!btext.isNull()){
          int tf = AlignVCenter|AlignLeft;
          if (!enabled)
              p->setPen(g.dark());
          dx= (w - fm.width(btext)) / 2;
          dy= (h - fm.lineSpacing()) / 2;
          if ( sunken ){
              ++dx;
              ++dy;
          }
          if (font)
              p->setFont(*font);
          if(raised)
              p->setPen(KGlobalSettings::toolBarHighlightColor());
          p->drawText(x+dx, y+dy, fm.width(btext), fm.lineSpacing(), tf, btext);
      }
  } else if (icontext == IconTextBottom){
      if (pixmap){
          dx = (w - pixmap->width()) / 2;
          dy = (h - fm.lineSpacing() - pixmap->height()) / 2;
          if ( sunken ){
              ++dx;
              ++dy;
          }
          p->drawPixmap( x+dx, y+dy, *pixmap );
      }
      if (!btext.isNull()){
          int tf = AlignBottom|AlignHCenter;
          dy= pixmap->height();
          dx = 2;
          
          if ( sunken ){
              ++dx;
              ++dy;
          }
          if (font)
              p->setFont(*font);
          if(raised)
              p->setPen(KGlobalSettings::toolBarHighlightColor());
          p->drawText(x, y, w, h-3, tf, btext);
      }
  }
  if (popup){
    if (enabled)
        qDrawArrow (p, Qt::DownArrow, Qt::WindowsStyle, false, w-5, h-5, 0, 0,
                      g, true);
    else
        qDrawArrow (p, Qt::DownArrow, Qt::WindowsStyle, false, w-5, h-5,
                      0, 0, g, false);
  }
}


void 
KSgiStyle::drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool active, QMenuItem *mi,
                            QBrush *)
{
  if ( p->font() == KGlobalSettings::generalFont() )
    p->setFont( KGlobalSettings::menuFont() );

  if(active){
    QApplication::style().drawItem(p, x, y, w, h,
                                   AlignCenter|ShowPrefix|DontClip|SingleLine,
                                   g, mi->isEnabled(), mi->pixmap(), mi->text(),
                                  -1, &g.highlight());
  } else
    QApplication::style().drawItem(p, x, y, w, h,
                                       AlignCenter|ShowPrefix|DontClip|SingleLine,
                                       g, mi->isEnabled(), mi->pixmap(), mi->text(),
                                       -1, &g.buttonText() );
}

void KSgiStyle::drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw,
                                     int tab, QMenuItem* mi,
                                     const QPalette& pal, bool act,
                                     bool enabled, int x, int y, int w, int h)
{
static const int motifItemFrame         = 2;
static const int motifItemHMargin       = 3;
static const int motifItemVMargin       = 2;
static const int motifArrowHMargin      = 6;
static const int windowsRightBorder     = 12;

  bool        dis = !enabled;
  QColorGroup itemg = dis ? pal.disabled() : pal.active();

  maxpmw = QMAX( maxpmw, 20 );
  int checkcol = maxpmw;


  if ( p->font() == KGlobalSettings::generalFont() )
      p->setFont( KGlobalSettings::menuFont() );

  if (!mi) {
    return;
  }
  
  if (mi->isSeparator()) {
    p->setPen (itemg.dark());
    p->drawLine (x, y, x+w, y);
    p->setPen (itemg.light() );
    p->drawLine (x, y+1, x+w, y+1);
    return;
  }

  if (act && enabled) {
 
    int x2 = x+w-1;
    int y2 = y+h-1;
        
    //
    // draw highlight around current menu item
    //
    
    QPen oldPen = p->pen();
        
    p->setPen(itemg.dark());
    p->drawLine(x2, y, x2, y2-1);
    p->drawLine(x, y2, x2, y2);
    p->fillRect(x, y, w-1, h-1, itemg.light());
        
    p->setPen(oldPen);
  } else {
    p->fillRect (x, y, w, h, itemg.background());
  }
  
  //
  // draw check mark
  //
  
  if (checkable && !mi->iconSet() && mi->isChecked()) {
    int w2 = checkcol-4;
    int h2 = h-2;

    //
    // make sure the check is square
    //

    if (w2 > h2) w2 = h2;

    drawIndicator (p, x+2, y+1, w2, h2, itemg, TRUE, FALSE, enabled);
  } 
  
  //
  //
  // draw associated icon
  //
        
  else if (mi->iconSet()) {
    QIconSet::Mode mode = dis? QIconSet::Disabled : QIconSet::Normal;
    if (!dis)
      mode = QIconSet::Active;
    QPixmap pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode);
    int pixw = pixmap.width();
    int pixh = pixmap.height();
 
    QRect cr(x, y, checkcol, h);
    QRect pmr(0, 0, pixw, pixh);
    pmr.moveCenter( cr.center() );
    p->setPen(itemg.highlightedText());
    p->drawPixmap(pmr.topLeft(), pixmap );
  }
      
  p->setPen(itemg.buttonText());
  QColor discol;
  if (dis) {
    discol = itemg.text();
    p->setPen(discol);
  }
      
  int xm = motifItemFrame + checkcol + motifItemHMargin;
  
  QString s = mi->text();
  if (!s.isNull()) {
    int t = s.find( '\t' );
    int m = motifItemVMargin;
    const int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
    if (t >= 0) {
      if (dis) {
        p->setPen (itemg.light() );
	p->drawText (x+w-tab-windowsRightBorder-motifItemHMargin-motifItemFrame+1,
		     y+m+1, tab, h-2*m, text_flags, s.mid( t+1 ));
	p->setPen( discol );
      }
      p->drawText (x+w-tab-windowsRightBorder-motifItemHMargin-motifItemFrame,
                   y+m, tab, h-2*m, text_flags, s.mid( t+1 ));
    }
    
    if (dis) {
      p->setPen (itemg.light());
      p->drawText( x+xm+1, y+m+1, w-xm+1, h-2*m, text_flags, s, t );
      p->setPen( discol );
    }
    p->drawText(x+xm, y+m, w-xm-tab+1, h-2*m, text_flags, s, t);
  } else if (mi->pixmap()) {
    QPixmap *pixmap = mi->pixmap();
    if (pixmap->depth() == 1)
      p->setBackgroundMode(OpaqueMode);
    p->drawPixmap( x+xm, y+motifItemFrame, *pixmap);
    if (pixmap->depth() == 1)
      p->setBackgroundMode(TransparentMode);
  }
  
  if (mi->popup()) {
    int dim = (h-2*motifItemFrame) / 2;
    if (act) {
      if (!dis)
        discol = itemg.buttonText();
    
      QColorGroup itemg2 (discol, itemg.highlight(),
                           white, white,
                           dis ? discol : white,
                           discol, white);
      drawArrow (p, RightArrow, FALSE,
              x+w - motifArrowHMargin - motifItemFrame - dim,  y+h/2-dim/2,
              dim, dim, itemg2, TRUE);
    } else {
      drawArrow (p, RightArrow, FALSE,
              x+w - motifArrowHMargin - motifItemFrame - dim,  y+h/2-dim/2,
	      dim, dim, itemg, mi->isEnabled() );
      
    }
  }
}

int 
KSgiStyle::popupMenuItemHeight(bool /*checkable*/, QMenuItem *mi,
                                 const QFontMetrics &fm)
{
  if (mi->isSeparator())
    return 2;

  int h = 0;
  if (mi->pixmap())
     h = mi->pixmap()->height();

  if (mi->iconSet())
     h = QMAX(mi->iconSet()->
                 pixmap(QIconSet::Small, QIconSet::Normal).height(), h);

  h = QMAX(fm.height() + 4, h);

  // we want a minimum size of 18
  h = QMAX(h, 18);

  return h;
}
