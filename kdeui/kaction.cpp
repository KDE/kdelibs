
#include "kaction.h"

#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kmenubar.h>
#include <qobjectlist.h>

static int get_toolbutton_id()
{
    static int toolbutton_no = -2;
    return toolbutton_no--;
}

KAction::KAction( const QString& text, int accel, QObject* parent, const char* name )
 : QAction( text, accel, parent, name )
{
}

KAction::KAction( const QString& text, int accel,
	            QObject* receiver, const char* slot, QObject* parent, const char* name )
 : QAction( text, accel, receiver, slot, parent, name )
{
}

KAction::KAction( const QString& text, const QIconSet& pix, int accel,
                    QObject* parent, const char* name )
 : QAction( text, pix, accel, parent, name )
{
}

KAction::KAction( const QString& text, const QIconSet& pix, int accel,
	            QObject* receiver, const char* slot, QObject* parent, const char* name )
 : QAction( text, pix, accel, receiver, slot, parent, name )
{
}

KAction::KAction( QObject* parent, const char* name )
 : QAction( parent, name )
{
}

int KAction::plug( QWidget *w )
{
  if ( w->inherits( "KToolBar" ) )
  {
    KToolBar *bar = (KToolBar *)w;

    int id_ = get_toolbutton_id();
    bar->insertButton( iconSet().pixmap(), id_, SIGNAL( clicked() ), this, SLOT( slotActivated() ),
		       isEnabled(), text() );

    KToolBarButton *but = bar->getButton( id_ );

    connect( but, SIGNAL( clicked( int ) ), this, SLOT( slotActivated() ) );

    addContainer( bar, id_ );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  return QAction::plug( w );
}


void KAction::unplug( QWidget *w )
{
  if ( w->inherits( "KToolBar" ) )
  {
    KToolBar *bar = (KToolBar *)w;

    int id = findContainer( bar );

    bar->removeItem( id );
  }
}

void KAction::setEnabled( bool b )
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

void KAction::setText( const QString& text )
{
  //TODO?

  QAction::setText( text );
}


void KAction::setIconSet( const QIconSet& iconSet )
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

KToggleAction::KToggleAction( const QString& text, int accel, QObject* parent, const char* name )
    : QToggleAction( text, accel, parent, name )
{
}

KToggleAction::KToggleAction( const QString& text, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QToggleAction( text, accel, receiver, slot, parent, name )
{
}

KToggleAction::KToggleAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* parent, const char* name )
    : QToggleAction( text, pix, accel, parent, name )
{
}

KToggleAction::KToggleAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QToggleAction( text, pix, accel, receiver, slot, parent, name )
{
}

KToggleAction::KToggleAction( QObject* parent, const char* name )
    : QToggleAction( parent, name )
{
}

int KToggleAction::plug( QWidget* widget )
{
    if ( !widget->inherits("QPopupMenu") && !widget->inherits("QActionWidget" ) &&
	 !widget->inherits("KToolBar") )
    {
	qDebug("Can not plug KToggleAction in %s", widget->className() );
	return -1;	
    }

    int index = -1;
    if ( widget->inherits( "KToolBar" ) ) {
	KToolBar *bar = (KToolBar *)widget;

	int id_ = get_toolbutton_id();
	bar->insertButton( iconSet().pixmap(), id_, SIGNAL( clicked() ), this, SLOT( slotActivated() ),
			   isEnabled(), text() );

	KToolBarButton *but = bar->getButton( id_ );

	connect( but, SIGNAL( clicked( int ) ), this, SLOT( slotActivated() ) );

	addContainer( bar, id_ );
	
	connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

	index =  containerCount() - 1;
  } else
      index = QAction::plug( widget );

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
	KToolBar *bar = (KToolBar*)container( index );
	KToolBarButton* b = bar->getButton( menuId( index ) );
	bar->setToggle( menuId( index ), TRUE );
	bar->setButton( menuId( index ), isChecked() );
    }

    return index;
}

void KToggleAction::setChecked( bool checked )
{
    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget *w = container( i );
	if ( w->inherits( "KToolBar" ) ) {
	    	QWidget* r = ( (KToolBar*)w )->getButton( menuId( i ) );
		if ( r->inherits( "KToolBarButton" ) )
		    ( (KToolBar*)w )->setButton( menuId( i ), checked );
	} else if ( w->inherits( "QPopupMenu" ) )
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
		if ( it.current()->inherits( "KToggleAction" ) &&
		     ((KToggleAction*)it.current())->exclusiveGroup() == exclusiveGroup() )
		    ((KToggleAction*)it.current())->setChecked( FALSE );
	    }
	}
    }

    QToggleAction::setChecked( checked );

    emit activated();
    emit toggled( isChecked() );
}





#include "kaction.moc"

