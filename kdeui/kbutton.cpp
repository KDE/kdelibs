#include "kbutton.h"

#include <qpainter.h>
#include <qdrawutl.h>

KButton::KButton( QWidget *_parent, const char *name )
    : QButton( _parent , name)
{
    raised = 0;
    connect( this, SIGNAL( pressed() ), this, SLOT( slotPressed () ) );
    connect( this, SIGNAL( released() ), this, SLOT( slotReleased () ) );
}

void KButton::enterEvent( QEvent* )
{
    if ( isEnabled() )
	raised = 1;
    repaint();
}

void KButton::leaveEvent( QEvent * )
{
    raised = 0;
    repaint();
}
    
void KButton::drawButton( QPainter *_painter )
{
    paint( _painter );
}

void KButton::drawButtonLabel( QPainter *_painter )
{
    paint( _painter );
}

void KButton::setOn( bool enable )
{
	QButton::setOn( enable );
	
	if (enable)
		raised = -1;
	else
		raised = 0;
	
	repaint();
}

void KButton::paint( QPainter *painter )
{
	if ( raised == 1 ) {
		if ( style() == WindowsStyle )
			qDrawWinButton( painter, 0, 0, width(), height(),
								colorGroup(), FALSE );
		else {
			qDrawShadePanel( painter, 0, 0, width(), height(), 
								colorGroup(), FALSE, 2, 0L );
			painter->setPen(black);
			painter->drawRect(0,0,width(),height()); 
		}
	} else if ( raised == -1 ) {
		if ( style() == WindowsStyle )
			qDrawWinButton( painter, 0, 0, width(), 
						height(), colorGroup(), TRUE );
		else
			qDrawShadePanel( painter, 0, 0, width(), 
						height(), colorGroup(), TRUE, 2, 0L );
    }
  
	if ( pixmap() ) {
		int dx = ( width() - pixmap()->width() ) / 2;
		int dy = ( height() - pixmap()->height() ) / 2;
		painter->drawPixmap( dx, dy, *pixmap() );
	}
}

void KButton::slotPressed()
{
    raised = -1;
    repaint();
}

void KButton::slotReleased()
{
	if ( isToggleButton() )
		raised =0;
	else
		raised = 1;

	repaint();
}

KButton::~KButton()
{
}

#include "kbutton.moc"
