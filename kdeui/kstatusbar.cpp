#include <kstatusbar.h>
#include <ktopwidget.h>
#include <kstatusbar.moc>
#include <qpainter.h>
// Revision 1.10  1998/04/21 20:37:48  radej
// Added insertWidget and some reorganisation - BINARY INCOMPATIBLE


/*************************************************************************/
// We want a statusbar-fields to be by this amount heigher than fm.height().

// This does NOT include the border width which we set separately for the statusbar.


KStatusBarItem::KStatusBarItem( char *text, int ID,
		QWidget *parent, char *) : QLabel( parent ) 
KStatusBarLabel::KStatusBarLabel( const QString& text, int _id,
    id = ID;
  QLabel( parent, name) 
    QFontMetrics fm = fontMetrics();
    w = fm.width( text )+8;
    h = fm.height() + FONT_Y_DELTA;
    resize( w, h );
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


void KStatusBar::init()
{
  borderwidth = DEFAULT_BORDERWIDTH;
  fieldheight = fontMetrics().height() + FONT_Y_DELTA;

  tmpTimer = new QTimer(this);
  connect (tmpTimer, SIGNAL(timeout()), this, SLOT(clear()));

}

void KStatusBar::setHeight(int h){

  fieldheight = h - 2* borderwidth;
  resize( width(),h);
  
}


void KStatusBar::setBorderWidth(int b){
  
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
  else if (tempWidget)
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
    updateRects( ); // False? You wouldn't sell that to toolbar... (sven)
}

void KStatusBar::setInsertOrder(InsertOrder order){

  insert_order = order;
void KStatusBar::updateRects( bool res )
{

  

void KStatusBar::updateRects( bool res ){
  
    for ( KStatusBarItem *b = labels.first(); b!=NULL; b=labels.next() ) {

    int offset= borderwidth;	
    for ( KStatusBarItem *b = items.first(); b; b=items.next() ) {


      b->setGeometry( offset, borderwidth, b->width(), fieldheight );	
      offset+=b->width() + borderwidth;
      KStatusBarItem *l = labels.getLast();
      if( l != NULL ) {
	offset-=l->width() + borderwidth;
	l->setGeometry( offset ,borderwidth, width() - offset - borderwidth, fieldheight );
      if( l ) {
		offset-=l->width() + borderwidth;
		l->setGeometry( offset ,borderwidth, width() - offset - borderwidth, fieldheight );
      }
    }
  }
    for ( KStatusBarItem *b = labels.first(); b!=NULL; b=labels.next() ) {
    int offset = width();

    for ( KStatusBarItem *b = items.first(); b; b=items.next() ) {
      offset -=b->width() + borderwidth;
      b->setGeometry(offset,borderwidth,b->width(),fieldheight );

      KStatusBarItem *l = labels.getLast();
      if( l != NULL ) {
	offset+=l->width() - borderwidth;
	l->setGeometry(borderwidth,borderwidth,offset,fieldheight);
      if( l != 0L ) {
		offset+=l->width() - borderwidth;
		l->setGeometry(borderwidth,borderwidth,offset,fieldheight);
      }
    }
void KStatusBar::changeItem( char *text, int id )
}
	for ( KStatusBarItem *b = labels.first(); b!=NULL; b=labels.next() ) 
      		if ( b->ID() == id )
				b->setText( text );
  for ( KStatusBarItem *b = items.first(); b; b=items.next() ) 
	if ( b->ID() == id )
	  ((KStatusBarLabel *)b->getItem())->setText( text );
}
  for ( KStatusBarItem *b = labels.first(); b!=NULL; b=labels.next() ) 
void KStatusBar::setAlignment( int id, int align)
	b->setAlignment( align | AlignVCenter );
  for ( KStatusBarItem *b = items.first(); b; b=items.next() ) 
void KStatusBar::slotReleased(int _id)

//Eh!!!

