
#include "k2action.h"

#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kmenubar.h>
#include <qobjectlist.h>

K2Action::K2Action( const QString& text, int accel, QObject* parent, const char* name )
 : QAction( text, accel, parent, name )
{
}

K2Action::K2Action( const QString& text, int accel,
	            QObject* receiver, const char* slot, QObject* parent, const char* name )
 : QAction( text, accel, receiver, slot, parent, name )
{
}

K2Action::K2Action( const QString& text, const QIconSet& pix, int accel,
                    QObject* parent, const char* name )
 : QAction( text, pix, accel, parent, name )
{
}

K2Action::K2Action( const QString& text, const QIconSet& pix, int accel,
	            QObject* receiver, const char* slot, QObject* parent, const char* name )
 : QAction( text, pix, accel, receiver, slot, parent, name )
{
}

K2Action::K2Action( QObject* parent, const char* name )
 : QAction( parent, name )
{
}

int K2Action::plug( QWidget *w )
{
  if ( w->inherits( "KToolBar" ) )
  {
    KToolBar *bar = (KToolBar *)w;

    int id = bar->insertButton( iconSet().pixmap(), 0, SIGNAL( activated() ), this, SLOT( slotActivated() ),
                                isEnabled(), text() );

    KToolBarButton *but = bar->getButton( id );
    
    connect( but, SIGNAL( clicked( int ) ), this, SLOT( slotActivated() ) );

    addContainer( bar, id );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  return QAction::plug( w );
}


void K2Action::unplug( QWidget *w )
{
  if ( w->inherits( "KToolBar" ) )
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

    if ( w->inherits( "KToolBar" ) )
      ((KToolBar *)w)->setItemEnabled( menuId( i ), b );

  }

  QAction::setEnabled( b );
}

void K2Action::setText( const QString& text )
{
  //TODO?

  QAction::setText( text );
}


void K2Action::setIconSet( const QIconSet& iconSet )
{
  int len = containerCount();

  for ( int i = 0; i < len; i++ )
  {
    QWidget *w = container( i );

    if ( w->inherits( "KToolBar" ) )
      ((KToolBar *)w)->setButtonPixmap( menuId( i ), iconSet.pixmap() );

  }

  QAction::setIconSet( iconSet );
}

K2ToggleAction::K2ToggleAction( const QString& text, int accel, QObject* parent, const char* name )
    : QToggleAction( text, accel, parent, name )
{
}

K2ToggleAction::K2ToggleAction( const QString& text, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QToggleAction( text, accel, receiver, slot, parent, name )
{
}

K2ToggleAction::K2ToggleAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* parent, const char* name )
    : QToggleAction( text, pix, accel, parent, name )
{
}

K2ToggleAction::K2ToggleAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QToggleAction( text, pix, accel, receiver, slot, parent, name )
{
}

K2ToggleAction::K2ToggleAction( QObject* parent, const char* name )
    : QToggleAction( parent, name )
{
}

int K2ToggleAction::plug( QWidget* widget )
{
    if ( !widget->inherits("QPopupMenu") && !widget->inherits("QActionWidget" ) &&
	 !widget->inherits("KToolBar") )
    {
	qDebug("Can not plug K2ToggleAction in %s", widget->className() );
	return -1;	
    }

    int index = QToggleAction::plug( widget );
    if ( index == -1 )
	return index;

    if ( widget->inherits("QPopupMenu") )
    {
	int id = menuId( index );

	popupMenu( index )->setItemChecked( id, isChecked() );
    }
    else if ( widget->inherits("QActionWidget" ) )
    {
    }
    else if ( widget->inherits("KToolBar") )
    {
	KToolBarButton* b = (KToolBarButton*)representative( index );
	b->beToggle( TRUE );
	b->on( isChecked() );
    }

    return index;
}

void K2ToggleAction::setChecked( bool checked )
{
    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	QWidget* r = representative( i );
	if ( w->inherits( "KToolBar" ) && r->inherits( "KToolBarButton" ) )
	    ((KToolBarButton*)r)->on( checked );
	else if ( w->inherits( "QPopupMenu" ) )
	    ((QPopupMenu*)w)->setItemChecked( menuId( i ), checked );
	else if ( w->inherits( "KMenuBar" ) )
	    ((KMenuBar*)w)->setItemChecked( menuId( i ), checked );
	else if ( w->inherits( "KActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );	
    }

    // Uncheck all the other toggle actions in the same group
    if ( parent() && !exclusiveGroup().isEmpty() )
    {
	const QObjectList *list = parent()->children();
	if ( list )
	{
	    QObjectListIt it( *list );
	    for( ; it.current(); ++it )
	    {
		if ( it.current()->inherits( "K2ToggleAction" ) &&
		     ((K2ToggleAction*)it.current())->exclusiveGroup() == exclusiveGroup() )
		    ((K2ToggleAction*)it.current())->setChecked( FALSE );
	    }
	}
    }

    QToggleAction::setChecked( checked );
    
    emit activated();
    emit toggled( isChecked() );
}





#include "k2action.moc"

