
#include "k2action.h"

#include <ktoolbar.h>

K2Action::K2Action( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 )
 : QAction( text, accel, parent, name )
{
}

K2Action::K2Action( const QString& text, int accel,
	            QObject* receiver, const char* slot, QObject* parent, const char* name = 0 )
 : QAction( text, accel, receiver, slot, parent, name )
{
}

K2Action::K2Action( const QString& text, const QIconSet& pix, int accel = 0,
                    QObject* parent = 0, const char* name = 0 )
 : QAction( text, pix, accel, parent, name )
{
}

K2Action::K2Action( const QString& text, const QIconSet& pix, int accel,
	            QObject* receiver, const char* slot, QObject* parent, const char* name = 0 )
 : QAction( text, pix, accel, receiver, slot, parent, name )
{
}

K2Action::K2Action( QObject* parent = 0, const char* name = 0 )
 : QAction( parent, name )
{
}

int K2Action::plug( QWidget *w )
{
  if ( w->inherits( "KToolBar" ) )
  {
    KToolBar *bar = (KToolBar *)w;

    int id = bar->insertButton( pixmap(), 0, SIGNAL( activated() ), this, SLOT( slotActivated() ),
                                isEnabled(), text() );

    addContainer( bar, id );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  QAction::plug( w );
}


void K2Action::unplug( QWidget *w )
{
  if ( w->inherits( "KToolBar " ) )
  {
    KToolBar *bar = (KToolBar *)w;

    int id = findContainer( bar );

    bar->removeItem( id );
  }
}

void K2Action::setEnabled( bool b )
{
  int len = containerCount();

  for ( int i = 0; i < len; i++ )
  {
    QWidget *w = container( i );

    if ( w->inherits( "KToolBar " ) )
      ((KToolBar *)w)->setItemEnabled( menuId( i ), b );

  }

  QAction::setEnabled( b );
}

#include "k2action.moc"

