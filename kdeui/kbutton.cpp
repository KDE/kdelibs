#include "kbutton.h"

#include <qpainter.h>
#include <qdrawutl.h>

KButton::KButton( QWidget *_parent, char *_name )
    : QButton( _parent, _name )
{
    raised = 0;
    connect( this, SIGNAL( pressed() ), this, SLOT( slotPressed () ) );
    connect( this, SIGNAL( released() ), this, SLOT( slotReleased () ) );
}

void KButton::enterEvent( QEvent *_ev )
{
    if ( isEnabled() )
	raised = 1;
    repaint();
}

void KButton::leaveEvent( QEvent *_ev )
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

void KButton::paint( QPainter *painter )
{
    static QColorGroup g( black, white, white, darkGray, lightGray, black, black );
    if ( raised == 1 )
	{
		if ( style() == WindowsStyle )
			qDrawWinButton( painter, 0, 0, width(), height(), g, FALSE );
		else
			qDrawShadePanel( painter, 0, 0, width(), height(), g, FALSE, 2, 0L );
	}
    else if ( raised == -1 )
	{
		if ( style() == WindowsStyle )
			qDrawWinButton( painter, 0, 0, width(), height(), g, TRUE );
		else
    		qDrawShadePanel( painter, 0, 0, width(), height(), g, TRUE, 2, 0L );
	}

    if ( pixmap() )
    {
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
    raised = 0;
    repaint();
}

KButton::~KButton()
{
}

#include "kbutton.moc"
