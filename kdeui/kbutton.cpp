#include "kbutton.h"

#include <qpainter.h>
#include <qdrawutl.h>

KButton::KButton( QWidget *_parent, const char *name )
    : QButton( _parent , name)
{
  raised = FALSE;
  setFocusPolicy( NoFocus );
}

void KButton::enterEvent( QEvent* )
{
  raised = TRUE;
  repaint(FALSE);
}

void KButton::leaveEvent( QEvent * )
{
  raised = FALSE;
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
  if ( isDown() || isOn() )
    {
      if ( style() == WindowsStyle )
	qDrawWinButton( painter, 0, 0, width(), 
			height(), colorGroup(), TRUE );
      else
	qDrawShadePanel( painter, 0, 0, width(), 
			 height(), colorGroup(), TRUE, 2, 0L );
    }
  else if ( raised )
    {
      if ( style() == WindowsStyle )
	qDrawWinButton( painter, 0, 0, width(), height(), 
			colorGroup(), FALSE );
      else
	qDrawShadePanel( painter, 0, 0, width(), height(), 
			 colorGroup(), FALSE, 2, 0L );
    }
  
  if ( pixmap() )
    {
      int dx = ( width() - pixmap()->width() ) / 2;
      int dy = ( height() - pixmap()->height() ) / 2;
      if ( isDown() && style() == WindowsStyle ) {
	dx++;
	dy++;
      }
      painter->drawPixmap( dx, dy, *pixmap() );
    }
}


KButton::~KButton()
{
}

#include "kbutton.moc"
