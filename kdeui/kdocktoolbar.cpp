/** KDockToolbar
  * 
  * Toolbars that can dock and be torn off
  *
  * Kalle Dalheimer <kalle@kde.org>
  *
  */

#include "kdocktoolbar.h"
#include "kdocktoolbar.moc"

#include <stdio.h>
#include <stdlib.h>

KDockToolBar::KDockToolBar( QWidget *parent=NULL,char *name=NULL ) :
  KToolBar( parent, name )
{
  _bMightDock = false;
}

KDockToolBar::~KDockToolBar()
{
}

void KDockToolBar::mousePressEvent( QMouseEvent* pEvent )
{
  if( pEvent->button() == LeftButton )
	{
	  _bMightDock = true;
	  if( barPos() != Floating )
		_aOldPos = pEvent->pos();
	}
  else
	KToolBar::mousePressEvent( pEvent ); // pass on
}

void KDockToolBar::mouseReleaseEvent( QMouseEvent* pEvent )
{
  if( pEvent->button() == LeftButton )
	{
	  if( _bMightDock )
		{
		  if( barPos() != Floating )
			{
			  /* We undock wenn we have left our old position completely. This
			   * will have to be revisited when we want to care for multiple
			   * toolbars 
			   */
			  if( _bMightDock && 
				  ( mapToGlobal( pEvent->pos() ).y() > 
					((QWidget*)parent())->mapToGlobal( QFrame::pos() ).y()
					+ height() ) )
				{
				  _oldPosition = barPos();
				  _oldX = x();
				  _oldY = y();
				  _pParent = (QWidget *)parent();
				  setBarPos( Floating );
				}
			}
		  else 
			{
			  /* already floating, move the toolbar and 
			   * possibly dock back again */
			  QPoint newAbs = _pParent->pos();
			  newAbs.setX( newAbs.x()+_oldX );
			  newAbs.setY( newAbs.y()+_oldY );
			  
			  QPoint myAbs = pos();

			  fprintf( stderr, "newAbs = (%d,%d), myAbs = (%d,%d)\n",
					   newAbs.x(), newAbs.y(), myAbs.x(), myAbs.y() );
			  if( abs( newAbs.x() - myAbs.x() ) < 30 && 
				  abs( newAbs.y() - myAbs.y() ) < 30 )
				setBarPos( _oldPosition );
			}
		  
		  _bMightDock = false;
		}
	}
  else
	KToolBar::mouseReleaseEvent( pEvent ); // pass on
}

void KDockToolBar::mouseMoveEvent( QMouseEvent* pEvent )
{
  if( pEvent->state() == LeftButton )
	{
	  // draw an outline here
	}
  else
	KToolBar::mouseMoveEvent( pEvent );
}

void KDockToolBar::mouseDoubleClickEvent( QMouseEvent* pEvent )
{
  if( pEvent->button() == LeftButton )
	{
	  if( barPos() == Floating )
		setBarPos( _oldPosition );
	  else
		{
		  _oldPosition = barPos();
		  _oldX = x();
		  _oldY = y();
		  _pParent = (QWidget *)parent();
		  setBarPos( Floating );
		}
	}
  else
	KToolBar::mouseDoubleClickEvent( pEvent );
}
