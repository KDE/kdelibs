#include <kstatusbar.h>
#include <ktopwidget.h>
#include "kstatusbar.moc"
#include <qpainter.h>
// Revision 1.10  1998/04/21 20:37:48  radej
#define StatusBarWidth 25

KStatusBarItem::KStatusBarItem( char *text, int ID,
		QWidget *parent, char *name) : QLabel( parent ) 
KStatusBarLabel::KStatusBarLabel( const QString& text, int _id,
    id = ID;
    
    QFontMetrics fm = fontMetrics();
    w = fm.width( text )+8;
    resize( w, 22 );
  w = fm.width( text )+8;
    setText( text );
	setFrameStyle( QFrame::Panel | QFrame::Sunken );
	setAlignment( AlignLeft | AlignVCenter );
  if ( style() == MotifStyle )
      setFrameStyle( QFrame::Panel | QFrame::Sunken );
  setAlignment( AlignLeft | AlignVCenter );
int KStatusBarItem::ID()
{
	return id;
void KStatusBarLabel::mouseReleaseEvent (QMouseEvent *)
{
}
KStatusBar::KStatusBar( QWidget *parent, char *name )
		: QFrame( parent, name )

	init();
  : QFrame( parent, name )
{
  delete tmpTimer; // What do I have to notice!?
};

void KStatusBar::drawContents( QPainter * )
{
}
	setFrameStyle( QFrame::NoFrame );
	resize( width(), StatusBarWidth );
  borderwidth = b;
  resize( width(),height() + 2* borderwidth);
KStatusBar::~KStatusBar()
{
	for ( KStatusBarItem *b = labels.first(); b!=NULL; b=labels.next() ) 
    	delete b;
}

bool KStatusBar::enable( BarStatus stat )
{
	bool mystat = isVisible();
	if ( (stat == Toggle && mystat) || stat == Hide )
		hide();
  	else
    	show();
    return ( isVisible() == mystat );
}

int KStatusBar::insertItem( char *text, int id )
{
	KStatusBarItem *label = new KStatusBarItem( text, id, this );
    labels.append( label );	
	updateRects( TRUE );
	return labels.at();
}

void KStatusBar::resizeEvent( QResizeEvent * ) {
	updateRects( );
                            width()-2*borderwidth, fieldheight);
  else
void KStatusBar::updateRects( bool res )
{
	int offset=0;

  	for ( KStatusBarItem *b = labels.first(); b!=NULL; b=labels.next() ) {
  		b->move( offset, 3 );	
  		offset+=b->width()+2;
  	}
  	
	if ( !res ) {
		KStatusBarItem *l = labels.getLast();
		if( l != NULL ) {
			offset-=l->width()+2;
			l->resize( width()-offset, 22 );
		}
	}
      }
    }
void KStatusBar::changeItem( char *text, int id )
}
	for ( KStatusBarItem *b = labels.first(); b!=NULL; b=labels.next() ) 
      		if ( b->ID() == id )
				b->setText( text );
void KStatusBar::slotReleased(int _id)

//Eh!!!

