#include <qpainter.h>  
#include "ktoolbar.h"
#include <qtooltip.h> 
#include <qdrawutl.h>
#include <qpixmap.h>
#include <qpmcache.h>
#include <qpalette.h>
#include "ktopwidget.h"

#include "ktoolbar.moc"

#define ToolbarWidth 30

#define CONTEXT_LEFT 0
#define CONTEXT_RIGHT 1
#define CONTEXT_TOP 2
#define CONTEXT_BOTTOM 3

#define DECOR_WIDTH (16)

const int extraMotifWidth = 10;
const int extraMotifHeight = 10;
  if (fullWidth == true)
  //debug ("Ho, ho, hooo... Up-Date!!! (vertical)");
KToolbarItem::KToolbarItem(QPixmap& pixmap, int ID, 
			   QWidget *parent, char *name) :
  KButton(parent, name) {
    resize(24,24);
    id = ID;
    EnabledPixmap = pixmap;
    calcDisabled();
    setPixmap(EnabledPixmap);
    connect((QPushButton*)this, SIGNAL(clicked()), 
	    this, SLOT(ButtonClicked()));
    connect(this, SIGNAL(pressed()), this, SLOT(ButtonPressed()));
    connect(this, SIGNAL(released()), this, SLOT(ButtonReleased()));
};

void KToolbarItem::calcDisabled() {
  QBitmap bitmap(22,22,TRUE);
  QPainter p;
  p.begin(&bitmap);
  int off=0;
  p.setPen(color1);
  for (int y=0;y<22;y++) {
    off = y;
    for (int x=0;x<22;x++) {
      if (off)
	p.drawPoint(x,y);
      off = (off + 1) % 2;
    };
  };
  p.end();
  DisabledPixmap = EnabledPixmap;
  DisabledPixmap.setMask(bitmap);
};
   QApplication::sendEvent() which, as far as I understand, notifies
KToolbarItem::KToolbarItem( QWidget *parent, char *name) :
  KButton(parent,name) {
    resize(0,0);
    hide();
    id = -1;
};

void KToolbarItem::enable(bool enabled) {
  setPixmap((enabled ? EnabledPixmap : DisabledPixmap));
  setEnabled(enabled);
};

void KToolbarItem::ButtonClicked() {
  emit clicked(id);
};

void KToolbarItem::ButtonPressed() {
  emit pressed(id);
};

void KToolbarItem::ButtonReleased() {
  emit released(id);
};

KToolbar::KToolbar(QWidget *parent, char *name) 
  : QFrame(parent, name) 
{
  init();
};

void KToolbar::ContextCallback(int index) {
  switch (index) {
  case CONTEXT_LEFT:
    setPos(Left);
    break;
  case CONTEXT_RIGHT:
    setPos(Right);
    break;
  case CONTEXT_TOP:
    setPos(Top);
    break;
  case CONTEXT_BOTTOM:
    setPos(Bottom);
    break;
  };
};

void KToolbar::init() {
  context = new QPopupMenu(0, "context");
  context->insertItem( "Left", CONTEXT_LEFT);
  context->insertItem( "Top",  CONTEXT_TOP);
  context->insertItem( "Right", CONTEXT_RIGHT);
  context->insertItem( "Bottom", CONTEXT_BOTTOM);
  connect(context, SIGNAL(activated(int)), this, SLOT(ContextCallback(int)));
  
  position = Top;
  setFrameStyle( QFrame::Panel | QFrame::Raised);
  setLineWidth(1);
  resize(width(),ToolbarWidth);
  QFrame *decoration;
  decoration = new QFrame(  this );;
  decoration->setFrameStyle( QFrame::VLine | QFrame::Raised );
  decoration->setMidLineWidth(1);
  decoration->setGeometry( 4, 3, 4, height()-7 );

  decoration = new QFrame(  this );;
  decoration->setFrameStyle( QFrame::VLine | QFrame::Raised );
  decoration->setMidLineWidth(1);
  decoration->setGeometry( 8, 3, 4, height()-7 );
  
  QFrame::setFrameStyle( QFrame::Box | QFrame::Sunken);
};

void KToolbar::drawContents ( QPainter *) {
};

KToolbar::~KToolbar() {
  delete context;
  for (KToolbarItem *b = buttons.first(); b!=NULL; b=buttons.next()) 
    delete b;
};

void KToolbar::setPos(Position pos) {
  if (position != pos) {
    position = pos;
    updateRects(TRUE);
    ((KTopLevelWidget*)parent())->updateRects();
  };
};

KToolbar::Position KToolbar::Pos() {
  return position;
};

bool KToolbar::enable(BarStatus stat) {
  bool mystat = isVisible();
  if ((stat == Toggle && mystat) || stat == Hide)
    hide();
  else
    show();
  return (isVisible()==mystat);
};

void KToolbar::updateRects(bool res=FALSE) {
  int offset=3+DECOR_WIDTH;
  for (KToolbarItem *b = buttons.first(); b!=NULL; b=buttons.next()) {
    switch (position) {
    case Top:
    case Bottom:
      if (b->ID()!=-1) {
	b->move(offset,3);
	offset+=25;
      } else
	offset+=4;
      break;
    case Left:
    case Right:
      if (b->ID()!=-1) {
	b->move(3,offset);
	offset+=25;
      } else
	offset+=4;
      break;
    };
  };
  if (res) 
    switch (position) {
    case Top:
    case Bottom:
      resize(offset+3, ToolbarWidth);
      break;
    case Left:
    case Right:
      resize(ToolbarWidth,offset+3);
      break;
    };
};

int KToolbar::insertItem(KPixmap& pixmap, int id, bool enabled, 
			 char *tooltiptext, int index) {
  KToolbarItem *button = new KToolbarItem(pixmap, id, this);
  if (index==-1)
    buttons.append(button);
  else
    buttons.insert(index,button);
  connect(button, SIGNAL(clicked(int)), this, SLOT(ItemClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ItemReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ItemPressed(int)));
  button->enable(enabled);
  if (tooltiptext)
    QToolTip::add(button, tooltiptext);
  updateRects(TRUE);
  return buttons.at();
};

int KToolbar::insertItem(KPixmap& pixmap, const char *signal, 
			 const QObject *receiver, const char *slot,
			 bool enabled, 
			 char *tooltiptext, int index) {
  KToolbarItem *button = new KToolbarItem(pixmap, 0, this);
  if (index==-1) 
    buttons.append(button);
  else
    buttons.insert(index,button);
  if (tooltiptext)
    QToolTip::add(button, tooltiptext);
  connect(button, signal, receiver, slot);
  button->enable(enabled);
  updateRects(TRUE);
  return buttons.at();
};

int KToolbarItem::ID() {
  return id;
};

void KToolbar::mousePressEvent ( QMouseEvent *m) {
  if (m->button()==RightButton) 
    context->popup(mapToGlobal(m->pos()), 0);
};

int KToolbar::insertSeparator(int index) {
  KToolbarItem *separ = new KToolbarItem(this);
  if (index==-1)
    buttons.append(separ);
  else
    buttons.insert(index, separ);
  updateRects(TRUE);
  return buttons.at();
};

void KToolbar::resizeEvent(QResizeEvent*) {
  if (position == Floating)
    updateRects();
};


void KToolbar::ItemClicked(int id) {
  emit clicked(id);
};

void KToolbar::ItemPressed(int id) {
  emit pressed(id);
};

void KToolbar::ItemReleased(int id) {
  emit released(id);
};

// The following is from kpushbt.cpp in kghostview 0.3.1



void KToolbarItem::drawButton( QPainter *paint )
{
    register QPainter *p = paint;
    GUIStyle	gs = style();
    QColorGroup g  = colorGroup();
    bool	updated = isDown();
    QColor	fillcol = g.background();
    int		x1, y1, x2, y2;

    rect().coords( &x1, &y1, &x2, &y2 );	// get coordinates

#define SAVE_PUSHBUTTON_PIXMAPS
#if defined(SAVE_PUSHBUTTON_PIXMAPS)
    QString pmkey;				// pixmap key
    int w, h;
    w = x2 + 1;
    h = y2 + 1;
    pmkey.sprintf( "$qt_push_%d_%d_%d_%d_%d_%d_%d", gs,
		   palette().serialNumber(), isDown(), 0, w, h,
		   isToggleButton() && isOn() );
    QPixmap *pm = QPixmapCache::find( pmkey );
    QPainter pmpaint;
    if ( pm ) {					// pixmap exists
	QPixmap pm_direct = *pm;
	pmpaint.begin( &pm_direct );
	pmpaint.drawPixmap( 0, 0, *pm );
	if ( text() )
	    pmpaint.setFont( font() );
	drawButtonLabel( &pmpaint );
	pmpaint.end();
	p->drawPixmap( 0, 0, pm_direct );
	
	if ( hasFocus() ) {
	    if ( style() == WindowsStyle ) {
		p->drawWinFocusRect( x1+3, y1+3, x2-x1-5, y2-y1-5 );
	    } else {
		p->setPen( black );
		p->drawRect( x1+3, y1+3, x2-x1-5, y2-y1-5 );
	    }
	}
	return;
    }
    bool use_pm = TRUE;
    if ( use_pm ) {
	pm = new QPixmap( w, h );		// create new pixmap
	CHECK_PTR( pm );
	pmpaint.begin( pm );
	p = &pmpaint;				// draw in pixmap
	p->setBackgroundColor( fillcol );
	p->eraseRect( 0, 0, w, h );
    }
#endif

    p->setPen( g.foreground() );
    p->setBrush( QBrush(fillcol,NoBrush) );

    if ( gs == WindowsStyle ) {		// Windows push button
	if ( isDown() ) {
	    if ( 0 ) {
		p->setPen( black );
		p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
		p->setPen( g.dark() );
		p->drawRect( x1+1, y1+1, x2-x1-1, y2-y1-1 );
	    }
	    else
		qDrawWinButton( p, x1, y1, w, h, g, TRUE );
	} else {
	    if ( 0 ) {
		p->setPen( black );
		p->drawRect( x1, y1, w, h );
		x1++; y1++;
		x2--; y2--;
	    }
	    if ( isToggleButton() && isOn() ) {
		qDrawWinButton( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE );
		if ( updated ) {
		    p->setPen( NoPen );
		    p->setBrush( g.mid() );
		    p->drawRect( x1+1, y1+1, x2-x1-2, y2-y1-2 );
		    updated = FALSE;
		}
	    } else {
		//qDrawWinButton( p, x1, y1, x2-x1+1, y2-y1+1, g, FALSE );
	    }
	}
	if ( updated )
	    p->fillRect( x1+1, y1+1, x2-x1-1, y2-y1-1, g.background() );
    }
    else if ( gs == MotifStyle ) {		// Motif push button
	if ( 0 ) {			// default Motif button
	    qDrawShadePanel( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE );
	    x1 += extraMotifWidth/2;
	    y1 += extraMotifHeight/2;
	    x2 -= extraMotifWidth/2;
	    y2 -= extraMotifHeight/2;
	}
	QBrush fill( fillcol );
	if ( isDown() ) {
	    qDrawShadePanel( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE, 2,
			     updated ? &fill : 0 );
	} else if ( isToggleButton() && isOn() ) {
	    qDrawShadePanel( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE, 2, 0 );
	    if ( updated ) {
		p->setPen( NoPen );
		p->setBrush( g.mid() );
		p->drawRect( x1+2, y1+2, x2-x1-3, y2-y1-3 );
	    }
	} else {
	   // qDrawShadePanel( p, x1, y1, x2-x1+1, y2-y1+1, g, FALSE, 2,
		//	     updated ? &fill : 0 );
	}
    }
    if ( p->brush().style() != NoBrush )
	p->setBrush( NoBrush );

#if defined(SAVE_PUSHBUTTON_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	p = paint;				// draw in default device
	p->drawPixmap( 0, 0, *pm );
	QPixmapCache::insert( pmkey, pm );	// save for later use
    }
#endif
    drawButtonLabel( p );
    if ( gs == MotifStyle && hasFocus() ) {
	p->setPen( black );
	p->drawRect( x1+3, y1+3, x2-x1-5, y2-y1-5 );
    }
    
}









      ++it;
    }
  }
}

#include "ktoolbar.moc"

