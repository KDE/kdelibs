
#include "kaction.h"

#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kmenubar.h>
#include <qobjectlist.h>
#include <kapp.h>

#include <X11/Xlib.h>

static void get_fonts( QStringList &lst )
{
    int numFonts;
    Display *kde_display;
    char** fontNames;
    char** fontNames_copy;
    QString qfontname;

    kde_display = kapp->getDisplay();

    bool have_installed = kapp->kdeFonts( lst );

    if ( have_installed )
	return;

    fontNames = XListFonts( kde_display, "*", 32767, &numFonts );
    fontNames_copy = fontNames;

    for( int i = 0; i < numFonts; i++ ) {

	if ( **fontNames != '-' ) {
	    fontNames ++;
	    continue;
	};

	qfontname = "";
	qfontname = *fontNames;
	int dash = qfontname.find ( '-', 1, TRUE );

	if ( dash == -1 ) {
	    fontNames ++;
	    continue;
	}

	int dash_two = qfontname.find ( '-', dash + 1 , TRUE );

	if ( dash == -1 ) {
	    fontNames ++;
	    continue;
	}


	qfontname = qfontname.mid( dash +1, dash_two - dash -1 );

	if ( !qfontname.contains( "open look", TRUE ) ) {
	    if( qfontname != "nil" ){
		if( lst.find( qfontname ) == lst.end() )
		    lst.append( qfontname );
	    }
	}

	fontNames ++;

    }

    XFreeFontNames( fontNames_copy );
}

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
    locked = FALSE;
    checked = FALSE;
    locked2 = FALSE;
}

KToggleAction::KToggleAction( const QString& text, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QToggleAction( text, accel, receiver, slot, parent, name )
{
    locked = FALSE;
    checked = FALSE;
    locked2 = FALSE;
}

KToggleAction::KToggleAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* parent, const char* name )
    : QToggleAction( text, pix, accel, parent, name )
{
    locked = FALSE;
    checked = FALSE;
    locked2 = FALSE;
}

KToggleAction::KToggleAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QToggleAction( text, pix, accel, receiver, slot, parent, name )
{
    locked = FALSE;
    checked = FALSE;
    locked2 = FALSE;
}

KToggleAction::KToggleAction( QObject* parent, const char* name )
    : QToggleAction( parent, name )
{
    locked = FALSE;
    checked = FALSE;
    locked2 = FALSE;
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
	addContainer( bar, id_ );
	
	connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

	index =  containerCount() - 1;
  } else
      index = QToggleAction::plug( widget );

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

void KToggleAction::setChecked( bool c )
{
    checked = c;

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

    emit activated();
    emit toggled( isChecked() );
}

void KToggleAction::slotActivated()
{
    if ( locked )
	return;

    locked = TRUE;
    if ( parent() && !exclusiveGroup().isEmpty() )
    {
	const QObjectList *list = parent()->children();
	if ( list )
	{
	    QObjectListIt it( *list );
	    for( ; it.current(); ++it )
	    {
		if ( it.current()->inherits( "KToggleAction" ) && it.current() != this &&
		     ((KToggleAction*)it.current())->exclusiveGroup() == exclusiveGroup() ) {
 		    ((KToggleAction*)it.current())->blockSignals( TRUE );
		    ((KToggleAction*)it.current())->setChecked( FALSE );
 		    ((KToggleAction*)it.current())->blockSignals( FALSE );
		}
	    }
	}
    }
    setChecked( !isChecked() );
    locked = FALSE;
}

bool KToggleAction::isChecked() const
{
    return checked;
}

// ############ Reggie: uuuuuuugly hack!!! Will remove it soon.
void KToggleAction::uncheckGroup()
{
    if ( parent() && !exclusiveGroup().isEmpty() )
    {
	const QObjectList *list = parent()->children();
	if ( list )
	{
	    QObjectListIt it( *list );
	    for( ; it.current(); ++it )
	    {
		if ( it.current()->inherits( "KToggleAction" ) &&
		     ((KToggleAction*)it.current())->exclusiveGroup() == exclusiveGroup() ) {
 		    ((KToggleAction*)it.current())->blockSignals( TRUE );
		    ((KToggleAction*)it.current())->setChecked( FALSE );
 		    ((KToggleAction*)it.current())->blockSignals( FALSE );
		}
	    }
	}
    }
}

KSelectAction::KSelectAction( const QString& text, int accel = 0,
			      QObject* parent = 0, const char* name = 0 )
    : QSelectAction( text, accel, parent, name )
{
}

KSelectAction::KSelectAction( const QString& text, int accel,
			      QObject* receiver, const char* slot, QObject* parent,
			      const char* name = 0 )
    : QSelectAction( text, accel, parent, name )
{
}

KSelectAction::KSelectAction( const QString& text, const QIconSet& pix, int accel = 0,
			      QObject* parent = 0, const char* name = 0 )
    : QSelectAction( text, pix, accel, parent, name )
{
}

KSelectAction::KSelectAction( const QString& text, const QIconSet& pix, int accel,
			      QObject* receiver, const char* slot, QObject* parent,
			      const char* name = 0 )
    : QSelectAction( text, pix, accel, receiver, slot, parent, name )
{
}

KSelectAction::KSelectAction( QObject* parent = 0, const char* name = 0 )
    : QSelectAction( parent, name )
{
}

void KSelectAction::setCurrentItem( int id )
{
    QSelectAction::setCurrentItem( id );
	
    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	if ( w->inherits( "KToolBar" ) ) {
	    QWidget* r = ( (KToolBar*)w )->getWidget( menuId( i ) );
	    if ( r->inherits( "QComboBox" ) ) {
		QComboBox *b = (QComboBox*)r;
		b->setCurrentItem( currentItem() );
	    }
	} else if ( w->inherits( "QActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );	
    }
	
    emit QAction::activated();
    emit activated( currentItem() );
    emit activated( currentText() );
}

void KSelectAction::setItems( const QStringList& lst )
{
    QSelectAction::setItems( lst );

    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	if ( w->inherits( "KToolBar" ) ) {
	    QWidget* r = ( (KToolBar*)w )->getWidget( menuId( i ) );
	    if ( r->inherits( "QComboBox" ) ) {
		QComboBox *b = (QComboBox*)r;
		b->clear();
		QStringList::ConstIterator it = items().begin();
		for( ; it != items().end(); ++it )
		    b->insertItem( *it );
	    }
	}
	else if ( w->inherits( "QActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );	
    }
}

int KSelectAction::plug( QWidget *widget )
{
    if ( widget->inherits("KPopupMenu") )
    {
	// Create the PopupMenu and store it in m_menu
	(void)popupMenu();
	
	QPopupMenu* menu = (QPopupMenu*)widget;
	int id;
	if ( !pixmap().isNull() )
        {
	    id = menu->insertItem( pixmap(), popupMenu() );
	}
	else
        {
	    if ( hasIconSet() )
		id = menu->insertItem( iconSet(), text(), popupMenu() );
	    else
		id = menu->insertItem( text(), popupMenu() );
	}

	menu->setItemEnabled( id, isEnabled() );
	menu->setWhatsThis( id, whatsThis() );

	addContainer( menu, id );
	connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	return containerCount() - 1;
    }
    else if ( widget->inherits("QActionWidget" ) )
    {
	connect( widget, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );	
	addContainer( widget, (int)0 );
	
	return containerCount() - 1;
    }
    else if ( widget->inherits("KToolBar") )
    {
	KToolBar* bar = (KToolBar*)widget;
	int id_ = get_toolbutton_id();
	bar->insertCombo( items(), id_, isEditable(), SIGNAL( activated( int ) ),
			  this, SLOT( slotActivated( int ) ) );
	QComboBox *cb = bar->getCombo( id_ );
	if ( cb )
	    cb->setMinimumWidth( cb->sizeHint().width() );
	addContainer( bar, id_ );

	connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	return containerCount() - 1;
    }

    qDebug("Can not plug QAction in %s", widget->className() );
    return -1;
}

void KSelectAction::clear()
{
    if ( popupMenu() )
	popupMenu()->clear();

    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	if ( w->inherits( "KToolBar" ) ) {
	    QWidget* r = ( (KToolBar*)w )->getWidget( menuId( i ) );
	    if ( r->inherits( "QComboBox" ) ) {
		QComboBox *b = (QComboBox*)r;
		b->clear();
	    }
	}
	else if ( w->inherits( "QActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );	
    }
}

KFontAction::KFontAction( const QString& text, int accel, QObject* parent, const char* name )
    : KSelectAction( text, accel, parent, name )
{
    get_fonts( fonts );
    setItems( fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : KSelectAction( text, accel, receiver, slot, parent, name )
{
    get_fonts( fonts );
    setItems( fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* parent, const char* name )
    : KSelectAction( text, pix, accel, parent, name )
{
    get_fonts( fonts );
    setItems( fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : KSelectAction( text, pix, accel, receiver, slot, parent, name )
{
    get_fonts( fonts );
    setItems( fonts );
    setEditable( TRUE );
}

KFontAction::KFontAction( QObject* parent, const char* name )
    : KSelectAction( parent, name )
{
    get_fonts( fonts );
    setItems( fonts );
    setEditable( TRUE );
}

void KFontAction::setFont( const QString &family )
{
    int i = fonts.findIndex( family );
    if ( i != -1 )
	setCurrentItem( i );
}


#include "kaction.moc"

