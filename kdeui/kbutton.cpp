#include "kbutton.h"

#include <qpainter.h>
#include <qdrawutl.h>

KButton::KButton( QWidget *_parent, char *name )
    : QButton( _parent , name)
{
    raised = 0;
    connect( this, SIGNAL( pressed() ), this, SLOT( slotPressed () ) );
    connect( this, SIGNAL( released() ), this, SLOT( slotReleased () ) );
}

<<<<<<< kbutton.cpp
void KButton::enterEvent( QEvent* )
=======
void KButton::enterEvent( QEvent * )
>>>>>>> 1.2
{
    if ( isEnabled() )
	raised = 1;
    repaint();
}

<<<<<<< kbutton.cpp
void KButton::leaveEvent( QEvent* )
=======
void KButton::leaveEvent( QEvent * )
>>>>>>> 1.2
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
<<<<<<< kbutton.cpp
    if ( raised == 1 )
	{
		if ( style() == WindowsStyle )
			qDrawWinButton( painter, 0, 0, width(), height(), 
							colorGroup(), FALSE );
		else
			qDrawShadePanel( painter, 0, 0, width(), height(), 
							 colorGroup(), FALSE, 2, 0L );
	}
=======
//    static QColorGroup g( black, white, white, darkGray, lightGray, black, black );

   if ( raised == 1 )
     {
        if ( style() == WindowsStyle )
             qDrawWinButton( painter, 0, 0, width(), height(), colorGroup(), FALSE );
        else
             qDrawShadePanel( painter, 0, 0, width(), height(), colorGroup(), FALSE, 2, 0L );
    }
>>>>>>> 1.2
    else if ( raised == -1 )
<<<<<<< kbutton.cpp
	{
		if ( style() == WindowsStyle )
			qDrawWinButton( painter, 0, 0, width(), height(), 
							colorGroup(), TRUE );
		else
    		qDrawShadePanel( painter, 0, 0, width(), height(), 
							 colorGroup(), TRUE, 2, 0L );
	}
=======
      {
       if ( style() == WindowsStyle )
          qDrawWinButton( painter, 0, 0, width(), height(), colorGroup(), TRUE );
        else
          qDrawShadePanel( painter, 0, 0, width(), height(), colorGroup(), TRUE, 2, 0L );
        }
 
>>>>>>> 1.2

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
