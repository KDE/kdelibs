#include <qpainter.h>  
#include <ktoolbar.h>
#include <qtooltip.h> 
#include <qdrawutl.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qpalette.h>
#include "kbutton.h"

#include "ktoolbar.moc"

#define ToolbarWidth 30

#define CONTEXT_LEFT 0
#define CONTEXT_RIGHT 1
#define CONTEXT_TOP 2
#define CONTEXT_BOTTOM 3

KToolBarItem::KToolBarItem( QPixmap& pixmap, int ID,
		QWidget *parent, char *name) : KButton( parent, name ) 
{
    resize(24,24);
    id = ID;
    if ( ! pixmap.isNull() ) 
    	enabledPixmap = pixmap;
    else {
	warning("KToolBarItem: pixmap is empty, perhaps some missing file");
 	enabledPixmap.resize( 22, 22);
    };
    makeDisabledPixmap();
    KButton::setPixmap( enabledPixmap );
    connect( this, SIGNAL( clicked() ), 
	    this, SLOT( ButtonClicked() ) );
    connect(this, SIGNAL( pressed() ), this, SLOT( ButtonPressed() ) );
    connect(this, SIGNAL( released() ), this, SLOT( ButtonReleased() ) );
}

void KToolBarItem::setPixmap( QPixmap &pixmap )
{
  if ( ! pixmap.isNull() )
    enabledPixmap = pixmap;
  else
    {
      warning("KToolBarItem: pixmap is empty, perhaps some missing file");
      enabledPixmap.resize( 22, 22);
    }
  // makeDisabledPixmap();
     KButton::setPixmap( enabledPixmap );
}
  icontext=config->readNumEntry("IconText", 0);
void KToolBarItem::makeDisabledPixmap()
{
	QPalette pal = palette();
	QColorGroup g = pal.disabled();
	
	// Find the outline of the colored portion of the normal pixmap
	
	QBitmap *pmm = (QBitmap*) enabledPixmap.mask();
        QPixmap pm;
        if (pmm != NULL) {
	   pmm->setMask( *pmm );
	   pm = *pmm;
        } else {
           pm.resize(22 , 22);
	   enabledPixmap.fill(this, 0, 0);
           warning("KToolBarItem::makeDisabledPixmap: mask is null.");
        };
    else
	// Prepare the disabledPixmap for drawing
	
	disabledPixmap.resize(22,22);
	disabledPixmap.fill( g.background() );
  if (fullWidth == true)
	// Draw the outline found above in white and then overlay a grey version
	// for a cheesy 3D effect ! BTW. this is the way that Qt 1.2+ does it for
	// Windows style
         }
	QPainter p;
	p.begin( &disabledPixmap );
	p.setPen( white );
	p.drawPixmap(1, 1, pm);
	p.setPen( g.text() );
	p.drawPixmap(0, 0, pm);
	p.end();
}
  //debug ("Ho, ho, hooo... Up-Date!!! (vertical)");
void KToolBarItem::paletteChange(const QPalette &)
{
	if( ID() != -1 ) {
		makeDisabledPixmap();
		if ( !isEnabled() )
			KButton::setPixmap( disabledPixmap );
		repaint( TRUE );
	}
}
   QApplication::sendEvent() which, as far as I understand, notifies
KToolBarItem::KToolBarItem( QWidget *parent, char *name )
	: KButton( parent , name)
{
    resize(0,0);
    hide();
    id = -1;
}
{
void KToolBarItem::enable( bool enabled )
{
	KButton::setPixmap( (enabled ? enabledPixmap : disabledPixmap) );
	setEnabled( enabled );
}
{
void KToolBarItem::ButtonClicked()
{
	emit clicked( id );
}
    emit (moved(position));
void KToolBarItem::ButtonPressed()
{
	emit pressed( id );
}

void KToolBarItem::ButtonReleased()
{
	emit released( id );
}
  if (position == Floating)
KToolBar::KToolBar(QWidget *parent, char *name) 
	: QFrame( parent, name ) 
{
	init();
}
    emit (moved(position));
void KToolBar::enableMoving(bool flag) {
  moving = flag;
};
    updateRects( true );
void KToolBar::ContextCallback( int index )
{
	switch ( index ) {
		case CONTEXT_LEFT:
    		setPos( Left );
    		break;
	  	case CONTEXT_RIGHT:
    		setPos( Right );
    		break;
	  	case CONTEXT_TOP:
    		setPos( Top );
    		break;
	  	case CONTEXT_BOTTOM:
    		setPos( Bottom );
    		break;
	}
}

void KToolBar::init()
{
	context = new QPopupMenu( 0, "context" );
	context->insertItem( "Left", CONTEXT_LEFT );
	context->insertItem( "Top",  CONTEXT_TOP );
	context->insertItem( "Right", CONTEXT_RIGHT );
	context->insertItem( "Bottom", CONTEXT_BOTTOM );

	connect( context, SIGNAL( activated( int ) ), this,
				SLOT( ContextCallback( int ) ) );

	position = Top;
	moving = TRUE;
	setFrameStyle( QFrame::Panel | QFrame::Raised );
	setLineWidth( 2 );
	resize( width(), ToolbarWidth );
}

void KToolBar::drawContents ( QPainter *)
{
}

void KToolBar::setItemPixmap( int id, QPixmap& _pixmap )
{
  for ( KToolBarItem *b = buttons.first(); b!=NULL; b=buttons.next() )
    if ( b->ID() == id )
      b->setPixmap( _pixmap );
}    
      
KToolBar::~KToolBar()
{
	// Get a seg. fault in this destructor with the following line included.
	// Don't know why.
	
	// delete context;

	for ( KToolBarItem *b = buttons.first(); b!=NULL; b=buttons.next() )
    	delete b;
}

void KToolBar::setPos( Position pos )
{
	if (position != pos) {
		
		position = pos;
		updateRects( TRUE );
	 	emit moved ( pos );	
	}
}

KToolBar::Position KToolBar::Pos()
{
	return position;
}

bool KToolBar::enable( BarStatus stat )
{
	bool mystat = isVisible();
	if ( (stat == Toggle && mystat) || stat == Hide )
		hide();
  	else
    	show();
    return ( isVisible() == mystat );
}

void KToolBar::updateRects( bool res )
{
	int offset=3+9+4;

  	for ( KToolBarItem *b = buttons.first(); b!=NULL; b=buttons.next() ) {
    	switch ( position ) {
    	case Top:
    	case Bottom:
      		if ( b->ID() != -1 ) {
				b->move( offset, 3 );
				offset+=25;
      		} else {
				offset+=6;
   			}
      		break;
    	case Left:
    	case Right:
      		if ( b->ID() != -1 ) {
				b->move( 3, offset );
				offset+=25;
      		} else {
				offset+=6;
   			}
      		break;
      	case Floating:
      		break;
    	}
	}
	
	if ( res ) 
    	switch ( position ) {
    		case Top:
    		case Bottom:
      			resize( offset+3, ToolbarWidth );
      			break;
    		case Left:
    		case Right:
      			resize( ToolbarWidth, offset+3 );
      			break;
      		case Floating:
      			break;
    	}
}

int KToolBar::insertItem( QPixmap& pixmap, int id, bool enabled, 
		char *tooltiptext, int index )
{
	KToolBarItem *button = new KToolBarItem( pixmap, id, this );
	if ( index == -1 )
	  buttons.append( button );
  	else
	  buttons.insert( index, button );
  
	connect(button, SIGNAL(clicked(int)), this, SLOT(ItemClicked(int)));
	connect(button, SIGNAL(released(int)), this, SLOT(ItemReleased(int)));
	connect(button, SIGNAL(pressed(int)), this, SLOT(ItemPressed(int)));
	button->enable( enabled );

	if ( tooltiptext )
		QToolTip::add( button, tooltiptext );
		
	updateRects( TRUE );
	return buttons.at();
}

int KToolBar::insertItem( QPixmap& pixmap, int id, const char *signal,
		const QObject *receiver, const char *slot, bool enabled,
		char *tooltiptext, int index )
{
	KToolBarItem *button = new KToolBarItem( pixmap, id, this );
	if ( index == -1 ) 
		buttons.append( button );
	else
		buttons.insert( index, button );
		
	if ( tooltiptext )
		QToolTip::add( button, tooltiptext );
		
	connect( button, signal, receiver, slot );
	button->enable( enabled );
	updateRects( TRUE );
	/*	printf("insertItem %d\n",buttons.at()); */
	return buttons.at();
}

int KToolBarItem::ID()
{
	return id;
}

void KToolBar::mousePressEvent ( QMouseEvent *m )
{
  if ( m->button() == RightButton && moving )
    context->popup( mapToGlobal( m->pos() ), 0 );
}

int KToolBar::insertSeparator( int index )
{
	KToolBarItem *separ = new KToolBarItem( this );
	if ( index == -1 )
		buttons.append( separ );
	else
		buttons.insert( index, separ );
	
	updateRects( TRUE );
	return buttons.at();
}

void KToolBar::resizeEvent( QResizeEvent * )
{
	if ( position == Floating )
		updateRects();
}

void KToolBar::paintEvent( QPaintEvent * )
{
	int offset=3;
	QColorGroup g = QWidget::colorGroup();

	QPainter *paint = new QPainter();
	paint->begin( this );

	switch ( position ) {
    	case Top:
    	case Bottom:
			qDrawShadePanel( paint, offset, 3, 4, height()-8,
		     	 g , FALSE, 2);
		    offset+=4;
			qDrawShadePanel( paint, offset, 3, 4, height()-8,
		     	 g , FALSE, 2);
		    offset+=9;
		 	break;
		 	
		case Left:
    	case Right:
    		qDrawShadePanel( paint, 3, offset, width()-8, 4,
		     	 g , FALSE, 2); 
		    offset+=4;
			qDrawShadePanel( paint, 3, offset,  width()-8, 4,
		     	 g , FALSE, 2);
		    offset+=9;
		    break;
		case Floating:
      		break;
	}
	
	for ( KToolBarItem *b = buttons.first(); b!=NULL; b=buttons.next() ) {
    	switch ( position ) {
    	case Top:
    	case Bottom:
      		if ( b->ID() != -1 ) {
				offset+=25;
      		} else {
				offset+=6;
				qDrawShadePanel( paint, offset-4, 2, 2, height()-4,
		      	g , TRUE, 1); 
   			}
      		break;
    	case Left:
    	case Right:
      		if ( b->ID() != -1 ) {
				offset+=25;
      		} else {
				offset+=6;
				qDrawShadePanel( paint, 2, offset-4, width()-4, 2,
		      	g , TRUE, 1); 
   			}
      		break;
      	case Floating:
      		break;
    	}
	}
	
	qDrawShadePanel( paint, 0, 0, width(), height(),
											g , FALSE, 2);
	paint->end();
}


void KToolBar::setItemEnabled( int id, bool enabled )
{
  for ( KToolBarItem *b = buttons.first(); b!=NULL; b=buttons.next() ) 
    if ( b->ID() == id )
      b->enable( enabled );
}
				
	

void KToolBar::ItemClicked( int id )
{
	emit clicked( id );
}

void KToolBar::ItemPressed( int id )
{
	emit pressed( id );
}

void KToolBar::ItemReleased( int id )
{
	emit released( id );
}
      ++it;
    }
  }
}

#include "ktoolbar.moc"

