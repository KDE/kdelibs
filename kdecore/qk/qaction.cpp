#include "qaction.h"
#include "qmenubar.h"
#include "qtoolbutton.h"
#include "qtoolbar.h"
#include "qwhatsthis.h"
#include "qcombobox.h"
#include "qlayout.h"
#include "qpushbutton.h"
#include "qevent.h"
#include "qiconview.h"
#include "qpopupmenu.h"
#include "qvalidator.h"
#include "qtl.h"

// NOT REVISED

static const char*folder[]={
"16 16 9 1",
"g c #808080",
"b c #ffa858",
"e c #c0c0c0",
"# c #000000",
"c c #ffdca8",
". c None",
"a c #585858",
"f c #a0a0a4",
"d c #ffffff",
"..#a#...........",
".#abc##.........",
".#daabc#####....",
".#ddeaabcccb#...",
".#dedeeabccca...",
".#edeeeeaaaab#..",
".#deeeeeeefe#ba.",
".#eeeeeeefef#ba.",
".#eeeeeefeff#ba.",
".#eeeeefefff#ba.",
".##geefeffff#ba.",
"...##gefffff#ba.",
".....##fffff#ba.",
".......##fff#b##",
".........##f#b##",
"...........####."};

static const char*unknown[]={
"16 16 7 1",
"# c #000000",
"b c #ffffff",
"e c #000000",
"d c #404000",
"c c #c0c000",
"a c #ffffc0",
". c None",
"................",
".........#......",
"......#.#a##....",
".....#b#bbba##..",
"....#b#bbbabbb#.",
"...#b#bba##bb#..",
"..#b#abb#bb##...",
".#a#aab#bbbab##.",
"#a#aaa#bcbbbbbb#",
"#ccdc#bcbbcbbb#.",
".##c#bcbbcabb#..",
"...#acbacbbbe...",
"..#aaaacaba#....",
"...##aaaaa#.....",
".....##aa#......",
".......##......."};




/*************************************************************
 *
 * QActionSeparator
 *
 *************************************************************/

QActionSeparator::QActionSeparator( QObject* parent, const char* name )
    : QAction( parent, name )
{
}

QActionSeparator::~QActionSeparator()
{
}

int QActionSeparator::plug( QWidget* widget )
{
    if ( widget->inherits("QPopupMenu") )
    {
	QPopupMenu* menu = (QPopupMenu*)widget;
	
	int id = menu->insertSeparator();
	
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
    else if ( widget->inherits("QToolBar") )
    {
	QToolBar* bar = (QToolBar*)widget;
	
	QFrame* frame = new QFrame( bar );
	frame->setFrameStyle( QFrame::VLine );
	frame->setFrameShadow( QFrame::Sunken );

	addContainer( bar, frame );
	connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	return containerCount() - 1;
    }

    qDebug("Can not plug QActionSeparator in %s", widget->className() );
    return -1;
}

void QActionSeparator::unplug( QWidget* widget )
{
    if ( widget->inherits("QPopupMenu") )
    {
	QPopupMenu* menu = (QPopupMenu*)widget;
	int i = findContainer( menu );
	if ( i != -1 )
        {
	    menu->removeItem( menuId( i ) );
	    removeContainer( i );
	}
    }
    else if ( widget->inherits("QToolBar" ) )
    {
	QToolBar* bar = (QToolBar*)widget;
	int i = findContainer( bar );
	if ( i != -1 )
        {
	    delete representative( i );
	    removeContainer( i );
	}
    }
    else if ( widget->inherits("QActionWidget" ) )
    {
	int i = findContainer( widget );
	if ( i != -1 )
	    removeContainer( i );
    }
}

/*************************************************************
 *
 * QAction
 *
 *************************************************************/

QAction::QAction( QObject* parent, const char* name )
    : QObject( parent, name )
{
    m_count = 1;
    m_bIconSet = FALSE;
    m_enabled = TRUE;
    m_accel = 0;
    m_component = 0;

    if ( parent && parent->inherits("QActionCollection") )
	((QActionCollection*)parent)->insert( this );
}

QAction::QAction( const QString& text, int accel, QObject* parent, const char* name )
    : QObject( parent, name )
{
    m_count = 1;
    m_bIconSet = FALSE;
    m_enabled = TRUE;
    m_component = 0;

    setAccel( accel );
    setText( text );

    if ( parent && parent->inherits("QActionCollection") )
	((QActionCollection*)parent)->insert( this );
}

QAction::QAction( const QString& text, const QIconSet& iconset, int accel, QObject* parent, const char* name )
    : QObject( parent, name )
{
    m_count = 1;
    m_bIconSet = FALSE;
    m_enabled = TRUE;
    m_component = 0;

    setAccel( accel );
    setText( text );
    setIconSet( iconset );

    if ( parent && parent->inherits("QActionCollection") )
	((QActionCollection*)parent)->insert( this );
}

QAction::QAction( const QString& text, const QIconSet& iconset, int accel,
		  QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QObject( parent, name )
{
    m_count = 1;
    m_bIconSet = FALSE;
    m_enabled = TRUE;
    m_component = 0;

    setAccel( accel );
    setText( text );
    setIconSet( iconset );

    if ( parent && parent->inherits("QActionCollection") )
	((QActionCollection*)parent)->insert( this );

    connect( this, SIGNAL( activated() ), receiver, slot );
}

QAction::QAction( const QString& text, int accel,
		  QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QObject( parent, name )
{
    m_count = 1;
    m_bIconSet = FALSE;
    m_enabled = TRUE;
    m_component = 0;

    setAccel( accel );
    setText( text );

    if ( parent && parent->inherits("QActionCollection") )
	((QActionCollection*)parent)->insert( this );

    connect( this, SIGNAL( activated() ), receiver, slot );
}

QAction::~QAction()
{
    if ( parent() && parent()->inherits("QActionCollection") )
	((QActionCollection*)parent())->take( this );

    //    QValueList<Container>::Iterator it = m_containers.begin();
    //    while( it != m_containers.end() )
    //   	it = m_containers.remove( it );
}

/*!
  Returns TRUE if the action is plugged into some
  menu, menubar, toolbar etc.
*/
bool QAction::isPlugged() const
{
    return ( containerCount() > 0 );
}

QObject* QAction::component()
{
    return m_component;
}

void QAction::setComponent( QObject* obj )
{
    m_component = obj;
}


void QAction::update()
{
    // By default do nothing interesting
}

void QAction::incref()
{
    ++m_count;
}

bool QAction::decref()
{
    return ( !--m_count );
}

int QAction::count()
{
    return m_count;
}

void QAction::setAccel( int a )
{
    m_accel = a;

    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	if ( w->inherits( "QPopupMenu" ) )
        {
	    qDebug("Setting ACCEL");
	    ((QPopupMenu*)w)->setAccel( m_accel, menuId( i ) );
	}
	else if ( w->inherits( "QMenuBar" ) )
	    ((QMenuBar*)w)->setAccel( m_accel, menuId( i ) );
    }
}

int QAction::accel() const
{
    return m_accel;
}

void QAction::setGroup( const QString& grp )
{
    m_group = grp;

    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	if ( w->inherits( "QActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );
    }
}

QString QAction::group() const
{
    return m_group;
}

bool QAction::isEnabled() const
{
    return m_enabled;
}

void QAction::setEnabled( bool e )
{
    if ( e == m_enabled )
	return;

    m_enabled = e;

    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* r = representative( i );
	QWidget* c = container( i );
	if ( r && c->inherits("QToolBar") )
	    r->setEnabled( m_enabled );
	else if ( c->inherits("QPopupMenu") )
	    ((QPopupMenu*)c)->setItemEnabled( menuId( i ), m_enabled );
	else if ( c->inherits("QMenuBar") )
	    ((QMenuBar*)c)->setItemEnabled( menuId( i ), m_enabled );
    }

    emit enabled( m_enabled );
}

int QAction::plug( QWidget* widget )
{
    if ( widget->inherits("QPopupMenu") )
    {
	QPopupMenu* menu = (QPopupMenu*)widget;
	int id;
	if ( !m_pixmap.isNull() )
        {
	    id = menu->insertItem( m_pixmap, this, SLOT( slotActivated() ) );	
	}
	else
        {
	    if ( m_bIconSet )
		id = menu->insertItem( m_iconSet, m_text, this, SLOT( slotActivated() ) );
	    else
		id = menu->insertItem( m_text, this, SLOT( slotActivated() ) );
	}

	menu->setItemEnabled( id, m_enabled );
	menu->setWhatsThis( id, m_whatsThis );

	addContainer( menu, id );
	connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	return m_containers.count() - 1;
    }
    else if ( widget->inherits("QActionWidget" ) )
    {
	connect( widget, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );	
	addContainer( widget, (int)0 );
	
	return m_containers.count() - 1;
    }
    else if ( widget->inherits("QToolBar") )
    {
	QToolBar* bar = (QToolBar*)widget;
	QToolButton* b;
	if ( !m_pixmap.isNull() )
	    b = new QToolButton( m_pixmap, m_text, m_groupText, this, SLOT( slotActivated() ), bar );
	else if ( hasIconSet() )
	    b = new QToolButton( m_iconSet, m_text, m_groupText, this, SLOT( slotActivated() ), bar );
	else
        {
	    b = new QToolButton( bar );
	    b->setTextLabel( m_text );
	    connect( b, SIGNAL( clicked() ), this, SLOT( slotActivated() ) );
	}

	b->setEnabled( m_enabled );

	addContainer( bar, b );
	connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	return m_containers.count() - 1;
    }

    qDebug("Can not plug QAction in %s", widget->className() );
    return -1;
}

void QAction::unplug( QWidget* widget )
{
    if ( widget->inherits("QPopupMenu") )
    {
	QPopupMenu* menu = (QPopupMenu*)widget;
	int i = findContainer( menu );
	if ( i != -1 )
        {
	    menu->removeItem( menuId( i ) );
	    removeContainer( i );
	}
    }
    else if ( widget->inherits("QToolBar" ) )
    {
	QToolBar* bar = (QToolBar*)widget;
	int i = findContainer( bar );
	if ( i != -1 )
        {
	    delete representative( i );
	    removeContainer( i );
	}
    }
    else if ( widget->inherits("QActionWidget" ) )
    {
	int i = findContainer( widget );
	if ( i != -1 )
	    removeContainer( i );
    }
}

void QAction::setText( const QString& text )
{
    m_text = text;

    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	QWidget* r = representative( i );
	if ( w->inherits( "QToolBar" ) && r->inherits( "QToolButton" ) )
	    ((QToolButton*)r)->setTextLabel( text );
	else if ( w->inherits( "QPopupMenu" ) )
	    ((QPopupMenu*)w)->changeItem( menuId( i ), text );
	else if ( w->inherits( "QMenuBar" ) )
	    ((QMenuBar*)w)->changeItem( menuId( i ), text );	
	else if ( w->inherits( "QActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );	
    }
}

QString QAction::text() const
{
    return m_text;
}

/*!
  Returns the text with all '&' characters removed which
  are used to create underlines.

  \sa text()
*/
QString QAction::plainText() const
{
    QString stripped( m_text );
    int pos;
    while( ( pos = stripped.find( '&' ) ) != -1 )
	stripped.replace( pos, 1, QString::null );

    return stripped;
}

void QAction::setIconSet( const QIconSet& iconset )
{
    m_iconSet = iconset;
    m_bIconSet = TRUE;

    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	QWidget* r = representative( i );
	if ( w->inherits( "QToolBar" ) && r->inherits( "QToolButton" ) )	
	    ((QToolButton*)r)->setIconSet( iconset );
	else if ( w->inherits( "QPopupMenu" ) )
	    ((QPopupMenu*)w)->changeItem( menuId( i ), iconset, m_text );
	else if ( w->inherits( "QMenuBar" ) )
	    ((QMenuBar*)w)->changeItem( menuId( i ), iconset, m_text );
	else if ( w->inherits( "QActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );
	
    }
}

QIconSet QAction::iconSet() const
{
    return m_iconSet;
}

bool QAction::hasIconSet() const
{
    return m_bIconSet;
}

void QAction::setWhatsThis( const QString& text )
{
    m_whatsThis = text;

    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QToolBar* tb = toolBar( i );
	QPopupMenu* pm = popupMenu( i );
	if ( tb )
        {
	    QWhatsThis::remove( representative( i ) );
	    QWhatsThis::add( representative( i ), text );	
	}
	else if ( pm )
	    pm->setWhatsThis( menuId( i ), text );
    }
}

QString QAction::whatsThis() const
{
    return m_whatsThis;
}

QPixmap QAction::pixmap() const
{
    return m_pixmap;
}

QWidget* QAction::container( int index )
{
    return m_containers[ index ].m_container;
}

QToolBar* QAction::toolBar( int index )
{
    QWidget* w = m_containers[ index ].m_container;
    if ( !w || !w->inherits( "QToolBar" ) )
	return 0;

    return (QToolBar*)w;
}

QPopupMenu* QAction::popupMenu( int index )
{
    QWidget* w = m_containers[ index ].m_container;
    if ( !w || !w->inherits( "QPopupMenu" ) )
	return 0;

    return (QPopupMenu*)w;
}

QWidget* QAction::representative( int index )
{
    return m_containers[ index ].m_representative;
}

int QAction::menuId( int index )
{
    return m_containers[ index ].m_id;
}

int QAction::containerCount() const
{
    return m_containers.count();
}

void QAction::addContainer( QWidget* c, int id )
{
    Container p;
    p.m_container = c;
    p.m_id = id;
    m_containers.append( p );
}

void QAction::addContainer( QWidget* c, QWidget* w )
{
    Container p;
    p.m_container = c;
    p.m_representative = w;
    m_containers.append( p );
}

void QAction::slotDestroyed()
{
    const QObject* o = sender();

    int i;
    do
    {
	i = findContainer( (QWidget*)o );
	if ( i != -1 )
	    removeContainer( i );
    } while ( i != -1 );
}

int QAction::findContainer( QWidget* widget )
{
    int pos = 0;
    QValueList<Container>::Iterator it = m_containers.begin();
    while( it != m_containers.end() )
    {
	if ( (*it).m_representative == widget || (*it).m_container == widget )
	    return pos;
	++it;
	++pos;
    }

    return -1;
}

void QAction::removeContainer( int index )
{
    int i = 0;
    QValueList<Container>::Iterator it = m_containers.begin();
    while( it != m_containers.end() )
    {
	if ( i == index )
        {
	    m_containers.remove( it );
	    return;
	}
	++it;
	++i;
    }
}

void QAction::slotActivated()
{
    emit activated();
}

QDomElement QAction::configuration( QDomDocument& doc, bool properties ) const
{
    QDomElement e = QObject::configuration( doc, properties );
    if ( m_component )
	e.setAttribute( "component", m_component->name() );

    return e;
}

/******************************************************
 *
 * QActionMenu
 ******************************************************/

QActionMenu::QActionMenu( QObject* parent, const char* name )
    : QAction( parent, name )
{
    // ##### use some parent here ...
    m_popup = new QPopupMenu( 0 );
}

QActionMenu::QActionMenu( const QString& text, QObject* parent, const char* name )
    : QAction( text, 0, parent, name )
{
    // ##### use some parent here ...
    m_popup = new QPopupMenu( 0 );
}

QActionMenu::QActionMenu( const QString& text, const QIconSet& icon, QObject* parent, const char* name )
    : QAction( text, icon, 0, parent, name )
{
    // ##### use some parent here ...
    m_popup = new QPopupMenu( 0 );
}

QActionMenu::~QActionMenu()
{
    delete m_popup;
}

void QActionMenu::popup( const QPoint& global )
{
    popupMenu()->popup( global );
}

QPopupMenu* QActionMenu::popupMenu()
{
    return m_popup;
}

void QActionMenu::insert( QAction* cmd )
{
    // m_children.append( cmd );
    if ( cmd )
	cmd->plug( m_popup );
}

void QActionMenu::remove( QAction* cmd )
{
    if ( cmd )
	cmd->unplug( m_popup );
}

int QActionMenu::plug( QWidget* widget )
{
    if ( widget->inherits("QMenuBar") )
    {
	QMenuBar* bar = (QMenuBar*)widget;
	int id;
	id = bar->insertItem( text(), m_popup );

	addContainer( bar, id );
	connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	return containerCount() - 1;	
    }
    else if ( widget->inherits("QPopupMenu") )
    {
	QPopupMenu* menu = (QPopupMenu*)widget;
	int id;
	if ( !pixmap().isNull() )
	    id = menu->insertItem( pixmap(), m_popup );	
	else
        {
	    if ( hasIconSet() )
		id = menu->insertItem( iconSet(), text(), m_popup );
	    else
		id = menu->insertItem( text(), m_popup );
	}

	menu->setItemEnabled( id, isEnabled() );
	
	addContainer( menu, id );
	connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	return containerCount() - 1;	
    }
    else if ( widget->inherits("QToolBar") )
    {
	QToolBar* bar = (QToolBar*)widget;
	QToolButton* b;
	if ( !pixmap().isNull() )
	    b = new QToolButton( pixmap(), text(), group(), this, SLOT( slotActivated() ), bar );
	else if ( hasIconSet() )
	    b = new QToolButton( iconSet(), text(), group(), this, SLOT( slotActivated() ), bar );
	else
        {
	    b = new QToolButton( bar );
	    b->setTextLabel( text() );
	    connect( b, SIGNAL( clicked() ), this, SLOT( slotActivated() ) );
	}

	b->setEnabled( isEnabled() );

	addContainer( bar, b );
	connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	b->setPopup( m_popup );
	b->setPopupDelay( 0 );
	
	return containerCount() - 1;
    }
    else if ( widget->inherits("QActionWidget" ) )
	return QAction::plug( widget );

    qDebug("Can not plug QActionMenu in %s", widget->className() );
    return FALSE;
}

void QActionMenu::unplug( QWidget* widget )
{
    if ( widget->inherits("QMenuBar") )
    {
	QMenuBar* bar = (QMenuBar*)widget;
	int i = findContainer( bar );
	if ( i != -1 )
        {
	    bar->removeItem( menuId( i ) );
	    removeContainer( i );
	}
    }
    else
	QAction::unplug( widget );
}

bool QActionMenu::setConfiguration( const QDomElement& element )
{
    QDomElement i = element.firstChild().toElement();
    for( ; !i.isNull(); i = i.nextSibling().toElement() )
    {
	if ( i.tagName() == "Action" )
        {
	    // Find the collection. Usually that is the parent
	    QActionCollection* col = 0;
	    if ( parent() && parent()->inherits("QActionCollection") )
		col = (QActionCollection*)parent();
	    if ( !col )
            {
		qDebug("QActionMenu: The toplevel widget does not have a QActionCollection." );
		return FALSE;
	    }
	
	    // Find the action
	    QString aname = i.attribute( "name" );
	    QAction* action = col->action( aname );
	    if ( action )
		insert( action );
	    else
		qDebug("QActionMenu: A QAction of name %s is not available.", aname.latin1() );
	}
	else
        {
	    QChar c = i.tagName()[0];
	    if ( c.upper() == c )
            {
		QObject* o = i.toObject( parent() );
		if ( !o || !o->inherits( "QAction" ) )
		    return FALSE;
	
		insert( (QAction*)o );
	    }
	}
    }

    return QAction::setConfiguration( element );
}

/******************************************************
 *
 * QActionCollection
 *
 ******************************************************/

QActionCollection::QActionCollection( QObject* parent, const char* name )
    : QObject( parent, name )
{
}

QActionCollection::~QActionCollection()
{
}


void QActionCollection::childEvent( QChildEvent* ev )
{
    if ( ev->inserted() && ev->child()->inherits( "QAction" ) )
	insert( (QAction*)ev->child() );
    else if ( ev->removed() )
	// We can not emit a removed signal here since the
	// actions destructor did already run :-(
	m_actions.remove( (QAction*)ev->child() );
}

void QActionCollection::insert( QAction* action )
{
    uint len = m_actions.count();
    for( uint i = 0; i < len; ++i )
    {
	QAction* a = m_actions.at( i );
	if ( action == a )
	    return;
    }

    m_actions.append( action );
    emit inserted( action );
}

void QActionCollection::remove( QAction* action )
{
    uint len = m_actions.count();
    for( uint i = 0; i < len; ++i )
    {
	QAction* a = m_actions.at( i );
	if ( action == a )
        {
	    m_actions.remove( i );
	    emit removed( action );
	    delete action;
	    return;
	}
    }
}

QAction* QActionCollection::take( QAction* action )
{
    uint len = m_actions.count();
    for( uint i = 0; i < len; ++i )
    {
	QAction* a = m_actions.at( i );
	if ( action == a )
        {
	    m_actions.remove( a );
	    emit removed( action );
	    return a;
	}
    }

    return 0;
}

/*!
  Searches the action that matches all three constraints. Each parameter may be 0,
  in this case they are not used during evaluation.
*/
QAction* QActionCollection::action( const char* name, const char* classname, QObject* component )
{
    QListIterator<QAction> it( m_actions );
    for( ; it.current(); ++it )
    {
	if ( ( !name || strcmp( it.current()->name(), name ) == 0 ) &&
	     ( !classname || strcmp( it.current()->className(), classname ) == 0 ) &&
	     ( !component || component == it.current()->component() ) )
	    return it.current();
    }
    return 0;
}

QAction* QActionCollection::action( int index )
{
    return m_actions.at( index );
}

uint QActionCollection::count() const
{
    return m_actions.count();
}

QStringList QActionCollection::groups() const
{
    QStringList lst;

    QListIterator<QAction> it( m_actions );
    for( ; it.current(); ++it )
	if ( !it.current()->group().isEmpty() && !lst.contains( it.current()->group() ) )
	    lst.append( it.current()->group() );

    return lst;
}

QValueList<QAction*> QActionCollection::actions( const QString& group )
{
    QValueList<QAction*> lst;

    QListIterator<QAction> it( m_actions );
    for( ; it.current(); ++it )
	if ( it.current()->group() == group )
	    lst.append( it.current() );
	else if ( it.current()->group().isEmpty() && group.isEmpty() )
	    lst.append( it.current() );

    return lst;
}

QValueList<QAction*> QActionCollection::actions()
{
    QValueList<QAction*> lst;

    QListIterator<QAction> it( m_actions );
    for( ; it.current(); ++it )
	lst.append( it.current() );

    return lst;
}

bool QActionCollection::setConfiguration( const QDomElement& element )
{
    QDomElement it = element.firstChild().toElement();
    for( ; !it.isNull(); it = it.nextSibling().toElement() )
    {
	QObject* o = it.toObject( this );
	if ( !o )
	    qDebug("QActionCollection: Could not create <%s>.", it.tagName().latin1() );
	else if ( o->inherits( "QAction" ) )
	    insert( (QAction*)o );
	else
	    qDebug("QActionCollection: <%s> is not a QAction.", it.tagName().latin1() );
    }

    return QObject::setConfiguration( element );
}

QDomElement QActionCollection::configuration( QDomDocument& doc, bool properties ) const
{
    QDomElement e = QObject::configuration( doc, properties );

    QListIterator<QAction> it( m_actions );
    for( ; it.current(); ++it )
	if ( it.current()->isPlugged() )
	    e.appendChild( it.current()->configuration( doc, properties ) );

    return e;
}

/******************************************************
 *
 * QActionWidget
 *
 ******************************************************/

QActionWidget::QActionWidget( QActionCollection* col, QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    init();

    setCollection( col );
}

QActionWidget::QActionWidget( QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    init();

    m_collection = 0;
}

void QActionWidget::init()
{
    m_icons = new QIconView( this );
    m_group = new QComboBox( this );

    m_icons->setResizeMode( QIconView::Adjust );

    QVBoxLayout* l = new QVBoxLayout( this, 0, 6 );
    l->addWidget( m_group );
    l->addWidget( m_icons );

    connect( m_group, SIGNAL( activated( const QString& ) ), this, SLOT( showGroup( const QString& ) ) );
    connect( m_icons, SIGNAL( dropped( QDropEvent* ) ), this, SLOT( slotDropped( QDropEvent* ) ) );
    connect( m_icons, SIGNAL( rightButtonPressed( QIconViewItem*, const QPoint& ) ),
	     this, SLOT( rightButtonPressed( QIconViewItem*, const QPoint& ) ) );
    connect( m_icons, SIGNAL( selectionChanged() ), this, SLOT( selectionChanged() ) );
}

QActionWidget::~QActionWidget()
{
}

void QActionWidget::showGroup( const QString& grp )
{
    setCurrentGroup( grp, TRUE );
}

QString QActionWidget::currentGroup() const
{
    return m_group->currentText();
}

void QActionWidget::setCurrentGroup( const QString& grp, bool update )
{
    if ( !m_collection )
	return;

    if ( !update && grp == m_group->currentText() )
	return;

    int i = 0;
    for( ; i < m_group->count(); ++i )
	if ( m_group->text( i ) == grp )
        {
	    m_group->setCurrentItem( i );
	    break;
	}

    if ( i == m_group->count() )
	return;

    m_icons->clear();

    QValueList<QAction*> cmds;
    if ( grp == tr("Misc") )
	cmds = m_collection->actions( "" );
    else
	cmds = m_collection->actions( grp );
    QValueList<QAction*>::Iterator it = cmds.begin();
    for( ; it != cmds.end(); ++it )
    {
	QIconViewItem* item = new QIconViewItem( m_icons );
	item->setText( (*it)->plainText() );
	item->setName( (*it)->name() );
	if ( (*it)->hasIconSet() )
	    item->setIcon( (*it)->iconSet() );
	else if ( (*it)->inherits("QActionMenu") )
	    item->setIcon( QIconSet( QPixmap( folder ) ) );
	else
	    item->setIcon( QIconSet( QPixmap( unknown ) ) );
    }
}

void QActionWidget::slotDropped( QDropEvent* mime )
{
    qDebug( "  Formats:\n" );
    int i = 0;
    const char *str = mime->format( i );
    qDebug( "    %s\n", str );
    while ( str ) {
	str = mime->format( ++i );
	qDebug( "    %s\n", str );
    }

}

void QActionWidget::selectionChanged()
{
    if ( !m_collection )
    {
	emit selectionChanged( 0 );
	return;
    }

    QIconViewItem* item = m_icons->selectedItem();
    if ( !item )
    {
	emit selectionChanged( 0 );
	return;
    }

    emit selectionChanged( m_collection->action( item->name() ) );
}

void QActionWidget::clearSelection()
{
    m_icons->selectAll( FALSE );
}

QAction* QActionWidget::currentAction()
{
    if ( !m_collection )
	return 0;

    QIconViewItem* item = m_icons->currentItem();
    if ( !item )
	return 0;

    return m_collection->action( item->name() );
}

QAction* QActionWidget::selectedAction()
{
    if ( !m_collection )
	return 0;

    QIconViewItem* item = m_icons->selectedItem();
    if ( !item )
	return 0;

    return m_collection->action( item->name() );
}

QActionCollection* QActionWidget::collection()
{
    return m_collection;
}

void QActionWidget::setCollection( QActionCollection* col )
{
    if ( col == m_collection )
	return;

    connect( col, SIGNAL( inserted( QAction* ) ), this, SLOT( insertAction( QAction* ) ) );
    connect( col, SIGNAL( removed( QAction* ) ), this, SLOT( removeAction( QAction* ) ) );

    m_icons->clear();

    m_collection = col;

    if ( !m_collection )
	return;

    QStringList groups = m_collection->groups();
    QStringList::ConstIterator it = groups.begin();
    for( ; it != groups.end(); ++it )
	m_group->insertItem( *it );
	
    QValueList<QAction*> cmds = m_collection->actions( "" );
    if ( !cmds.isEmpty() )
	m_group->insertItem( tr("Misc") );

    showGroup( m_group->currentText() );

    cmds = m_collection->actions();
    QValueList<QAction*>::ConstIterator it2 = cmds.begin();
    for( ; it2 != cmds.end(); ++it2 )
	(*it2)->plug( this );
}

void QActionWidget::insertAction( QAction* action )
{
    action->plug( this );

    // Insert a new icon if we show the correct group already
    if ( action->group() == m_group->currentText() ||
	 action->group().isEmpty() && m_group->currentText() == tr("Misc") )
    {
	QIconViewItem* item = new QIconViewItem( m_icons );
	item->setText( action->plainText() );
	item->setName( action->name() );
	if ( action->hasIconSet() )
	    item->setIcon( action->iconSet() );
	else if ( action->inherits("QActionMenu") )
	    item->setIcon( QIconSet( QPixmap( folder ) ) );
	else
	    item->setIcon( QIconSet( QPixmap( unknown ) ) );
    }
    else
    {
	// Check wether there is a new group. If so:
	// insert and show it.
	bool b = TRUE;
	int len = m_group->count();
	for( int i = 0; i < len; ++i )
	    if ( m_group->text(i) == action->group() )
		b = FALSE;
	    else if ( m_group->text(i) == tr("Misc") && action->group().isEmpty() )
		b = FALSE;
	
	if ( b )
        {
	    QString grp( action->group() );
	    if ( grp.isEmpty() )
		grp = tr("Misc");
	    m_group->insertItem( grp );
	    setCurrentGroup( grp );
	}
    }
}

void QActionWidget::updateAction( QAction* action )
{
    qDebug("====================== ACTION %s ==============", action->name() );

    QIconViewItem* item = m_icons->firstItem();
    for( ; item; item = item->nextItem() )
	if ( strcmp( item->name(), action->name() ) == 0 )
	    break;

    if ( item )
    {
	qDebug("==================");
	// The action is visible, so modify the QIconViewItem
	// QIconViewItem* item = (QIconViewItem*)o;
	item->setText( action->plainText() );
	item->setName( action->name() );
	if ( action->hasIconSet() )
	    item->setIcon( action->iconSet() );
	else if ( action->inherits("QActionMenu") )
	    item->setIcon( QIconSet( QPixmap( folder ) ) );
	else
	    item->setIcon( QIconSet( QPixmap( unknown ) ) );
    }
    else
    {
	// Check wether there is a new group. If so:
	// insert it.
	bool b = TRUE;
	int len = m_group->count();
	for( int i = 0; i < len; ++i )
	    if ( m_group->text(i) == action->group() )
		b = FALSE;
	    else if ( m_group->text(i) == tr("Misc") && action->group().isEmpty() )
		b = FALSE;
	
	if ( b )
        {
	    QString grp( action->group() );
	    if ( grp.isEmpty() )
		grp = tr("Misc");
	    m_group->insertItem( grp );
	}
    }
}

void QActionWidget::removeAction( QAction* )
{
    showGroup( m_group->currentText() );

    // ##### a bug in QIconView ?
    //       No repaint and segfaults
    /*
    if ( action->group() == m_group->currentText() ||
	 ( action->group().isEmpty() && m_group->currentText() == tr("Misc") ) )
    {
	QIconViewItem* item = m_icons->findItem( action->text() );
	if ( item )
	    delete item;
    }
    */
}

void QActionWidget::rightButtonPressed( QIconViewItem* item, const QPoint& pos )
{
    if ( !item )
    {
	emit rightButtonPressed( (QAction*)0, pos );
	return;
    }

    emit rightButtonPressed( m_collection->action( item->name() ), pos );
}

void QActionWidget::addGroup( const QString& group )
{
    m_group->insertItem( group );
}

/******************************************************
 *
 * QActionDialog
 *
 ******************************************************/

QActionDialog::QActionDialog( QActionCollection* col, QWidget* parent, const char* name, bool modal )
    : QDialog( parent, name, modal )
{
    m_widget = new QActionWidget( col, this );

    QPushButton* close = new QPushButton( tr("Close"), this );
    connect( close, SIGNAL( clicked() ), this, SLOT( accept() ) );

    QVBoxLayout* l = new QVBoxLayout( this, 6, 6 );
    l->addWidget( m_widget );

    QHBoxLayout* h = new QHBoxLayout( 6 );
    l->addLayout( h );
    h->addStretch( 1 );
    h->addWidget( close );
}

QActionDialog::~QActionDialog()
{
}

QActionWidget* QActionDialog::actionWidget()
{
    return m_widget;
}

/******************************************************
 *
 * QToggleAction
 *
 ******************************************************/

QToggleAction::QToggleAction( const QString& text, int accel, QObject* parent, const char* name )
    : QAction( text, accel, parent, name )
{
    m_checked = FALSE;
    m_lock = FALSE;
}

QToggleAction::QToggleAction( const QString& text, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QAction( text, accel, receiver, slot, parent, name )
{
    m_checked = FALSE;
    m_lock = FALSE;
}

QToggleAction::QToggleAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* parent, const char* name )
    : QAction( text, pix, accel, parent, name )
{
    m_checked = FALSE;
    m_lock = FALSE;
}

QToggleAction::QToggleAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QAction( text, pix, accel, receiver, slot, parent, name )
{
    m_checked = FALSE;
    m_lock = FALSE;
}

QToggleAction::QToggleAction( QObject* parent, const char* name )
    : QAction( parent, name )
{
    m_checked = FALSE;
    m_lock = FALSE;
}

int QToggleAction::plug( QWidget* widget )
{
    if ( !widget->inherits("QPopupMenu") && !widget->inherits("QActionWidget" ) &&
	 !widget->inherits("QToolBar") )
    {
	qDebug("Can not plug QToggleAction in %s", widget->className() );
	return -1;	
    }

    int index = QAction::plug( widget );
    if ( index == -1 )
	return index;

    if ( widget->inherits("QPopupMenu") )
    {
	int id = menuId( index );

	popupMenu( index )->setItemChecked( id, m_checked );
    }
    else if ( widget->inherits("QActionWidget" ) )
    {
    }
    else if ( widget->inherits("QToolBar") )
    {
	QToolButton* b = (QToolButton*)representative( index );
	b->setToggleButton( TRUE );
	b->setOn( m_checked );
    }

    return index;
}

void QToggleAction::setChecked( bool checked )
{
    if ( m_checked == checked )
	return;

    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	QWidget* r = representative( i );
	if ( w->inherits( "QToolBar" ) && r->inherits( "QToolButton" ) )
	    ((QToolButton*)r)->setOn( checked );
	else if ( w->inherits( "QPopupMenu" ) )
	    ((QPopupMenu*)w)->setItemChecked( menuId( i ), checked );
	else if ( w->inherits( "QMenuBar" ) )
	    ((QMenuBar*)w)->setItemChecked( menuId( i ), checked );
	else if ( w->inherits( "QActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );	
    }

    m_checked = checked;

    emit activated();
    emit toggled( m_checked );
}

bool QToggleAction::isChecked()
{
    return m_checked;
}

void QToggleAction::slotActivated()
{
    if ( m_lock )
	return;

    m_lock = TRUE;
    setChecked( !m_checked );
    m_lock = FALSE;
}

// ------------------------------------------------------------

QSelectAction::QSelectAction( const QString& text, int accel, QObject* parent, const char* name )
    : QAction( text, accel, parent, name )
{
    m_lock = FALSE;
    m_menu = 0;
    m_edit = FALSE;
}

QSelectAction::QSelectAction( const QString& text, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QAction( text, accel, receiver, slot, parent, name )
{
    m_lock = FALSE;
    m_menu = 0;
    m_current = -1;
    m_edit = FALSE;
}

QSelectAction::QSelectAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* parent, const char* name )
    : QAction( text, pix, accel, parent, name )
{
    m_lock = FALSE;
    m_menu = 0;
    m_current = -1;
    m_edit = FALSE;
}

QSelectAction::QSelectAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QAction( text, pix, accel, receiver, slot, parent, name )
{
    m_lock = FALSE;
    m_menu = 0;
    m_current = -1;
    m_edit = FALSE;
}

QSelectAction::QSelectAction( QObject* parent, const char* name )
    : QAction( parent, name )
{
    m_lock = FALSE;
    m_menu = 0;
    m_current = -1;
    m_edit = FALSE;
}

void QSelectAction::setCurrentItem( int id )
{
    if ( id >= (int)m_list.count() )
	return;

    if ( m_menu )
    {
	if ( m_current >= 0 )
	    m_menu->setItemChecked( m_current, FALSE );
	if ( id >= 0 )
	    m_menu->setItemChecked( id, TRUE );
    }

    m_current = id;
	
    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	QWidget* r = representative( i );
	if ( w->inherits( "QToolBar" ) && r->inherits( "QComboBox" ) )
        {
	    QComboBox* b = (QComboBox*)r;
	    b->setCurrentItem( m_current );
	}
	else if ( w->inherits( "QActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );	
    }
	
    emit QAction::activated();
    emit activated( currentItem() );
    emit activated( currentText() );
}

QPopupMenu* QSelectAction::popupMenu()
{
    if ( !m_menu )
    {
	m_menu = new QPopupMenu;
	QStringList::ConstIterator it = m_list.begin();
	int id = 0;
	for( ; it != m_list.end(); ++it )
	    m_menu->insertItem( *it, this, SLOT( slotActivated( int ) ), 0, id++ );
    }

    return m_menu;
}

void QSelectAction::setItems( const QStringList& lst )
{
    m_list = lst;
    m_current = -1;

    if ( m_menu )
    {
	m_menu->clear();
	QStringList::ConstIterator it = m_list.begin();
	int id = 0;
	for( ; it != m_list.end(); ++it )
	    m_menu->insertItem( *it, this, SLOT( slotActivated( int ) ), 0, id++ );
    }

    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	QWidget* r = representative( i );
	if ( w->inherits( "QToolBar" ) && r->inherits( "QComboBox" ) )
        {
	    QComboBox* b = (QComboBox*)r;
	    b->clear();
	    QStringList::ConstIterator it = m_list.begin();
	    for( ; it != m_list.end(); ++it )
		b->insertItem( *it );
	}
	else if ( w->inherits( "QActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );	
    }
}

QStringList QSelectAction::items()
{
    return m_list;
}

QString QSelectAction::currentText()
{
    if ( currentItem() < 0 )
	return QString::null;

    return m_list[ currentItem() ];
}

int QSelectAction::currentItem()
{
    return m_current;
}

int QSelectAction::plug( QWidget* widget )
{
    if ( widget->inherits("QPopupMenu") )
    {
	// Create the PopupMenu and store it in m_menu
	(void)popupMenu();
	
	QPopupMenu* menu = (QPopupMenu*)widget;
	int id;
	if ( !pixmap().isNull() )
        {
	    id = menu->insertItem( pixmap(), m_menu );
	}
	else
        {
	    if ( hasIconSet() )
		id = menu->insertItem( iconSet(), text(), m_menu );
	    else
		id = menu->insertItem( text(), m_menu );
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
    else if ( widget->inherits("QToolBar") )
    {
	QToolBar* bar = (QToolBar*)widget;
	QComboBox* b = new QComboBox( isEditable(), bar );

	QStringList::ConstIterator it = m_list.begin();
	for( ; it != m_list.end(); ++it )
	    b->insertItem( *it );
	
	b->setEnabled( isEnabled() );
	
	connect( b, SIGNAL( activated( int ) ), this, SLOT( slotActivated( int ) ) );
	
	addContainer( bar, b );
	connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	return containerCount() - 1;
    }

    qDebug("Can not plug QAction in %s", widget->className() );
    return -1;
}

void QSelectAction::slotActivated( int id )
{
    if ( m_current == id )
	return;

    if ( m_lock )
	return;

    m_lock = TRUE;
    setCurrentItem( id );
    m_lock = FALSE;
}

void QSelectAction::clear()
{
    if ( m_menu )
	m_menu->clear();

    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	QWidget* r = representative( i );
	if ( w->inherits( "QToolBar" ) && r->inherits( "QComboBox" ) )
        {
	    QComboBox* b = (QComboBox*)r;
	    b->clear();
	}
	else if ( w->inherits( "QActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );	
    }
}

void QSelectAction::setEditable( bool edit )
{
    m_edit = edit;
}

bool QSelectAction::isEditable() const
{
    return m_edit;
}

// ------------------------------------------------------------

QFontAction::QFontAction( const QString& text, int accel, QObject* parent, const char* name )
    : QSelectAction( text, accel, parent, name )
{
    setItems( m_fdb.families() );
}

QFontAction::QFontAction( const QString& text, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QSelectAction( text, accel, receiver, slot, parent, name )
{
    setItems( m_fdb.families() );
}

QFontAction::QFontAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* parent, const char* name )
    : QSelectAction( text, pix, accel, parent, name )
{
    setItems( m_fdb.families() );
}

QFontAction::QFontAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QSelectAction( text, pix, accel, receiver, slot, parent, name )
{
    setItems( m_fdb.families() );
}

QFontAction::QFontAction( QObject* parent, const char* name )
    : QSelectAction( parent, name )
{
    setItems( m_fdb.families() );
}

// ------------------------------------------------------------

QFontSizeAction::QFontSizeAction( const QString& text, int accel, QObject* parent, const char* name )
    : QSelectAction( text, accel, parent, name )
{
    init();
}

QFontSizeAction::QFontSizeAction( const QString& text, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QSelectAction( text, accel, receiver, slot, parent, name )
{
    init();
}

QFontSizeAction::QFontSizeAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* parent, const char* name )
    : QSelectAction( text, pix, accel, parent, name )
{
    init();
}

QFontSizeAction::QFontSizeAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QSelectAction( text, pix, accel, receiver, slot, parent, name )
{
    init();
}

QFontSizeAction::QFontSizeAction( QObject* parent, const char* name )
    : QSelectAction( parent, name )
{
    init();
}

void QFontSizeAction::init()
{
    m_lock = FALSE;

    QStringList lst;
    lst.append( "8" );
    lst.append( "9" );
    lst.append( "10" );
    lst.append( "11" );
    lst.append( "12" );
    lst.append( "14" );
    lst.append( "16" );
    lst.append( "18" );
    lst.append( "20" );
    lst.append( "24" );
    lst.append( "32" );
    lst.append( "48" );
    lst.append( "64" );

    setItems( lst );
}

void QFontSizeAction::setFontSize( int size )
{
    if ( size == fontSize() )
	return;

    if ( size < 1 || size > 128 )
    {
	qDebug( "QFontSizeAction: Size %i is out of range", size );
	return;
    }

    int index = items().findIndex( QString::number( size ) );
    if ( index == -1 )
    {
	QStringList lst = items();
	lst.append( QString::number( size ) );
	qHeapSort( lst );
	setItems( lst );
	index = lst.findIndex( QString::number( size ) );
	setCurrentItem( index );
    }
    else
    {
	// Avoid dupes in combo boxes ...
	setItems( items() );
	setCurrentItem( index );
    }

    emit QAction::activated();
    emit activated( index );
    emit activated( QString::number( size ) );
    emit fontSizeChanged( size );
}

int QFontSizeAction::fontSize()
{
    return currentText().toInt();
}

void QFontSizeAction::slotActivated( int index )
{
    QSelectAction::slotActivated( index );

    emit fontSizeChanged( items()[ index ].toInt() );
}

void QFontSizeAction::slotActivated( const QString& size )
{
    if ( m_lock )
	return;

    if ( size.toInt() < 1 || size.toInt() > 128 )
    {
	qDebug( "QFontSizeAction: Size %s is out of range", size.latin1() );
	return;
    }

    m_lock = TRUE;
    setFontSize( size.toInt() );
    m_lock = FALSE;
}

int QFontSizeAction::plug( QWidget* widget )
{
    if ( widget->inherits("QToolBar") )
    {
	QToolBar* bar = (QToolBar*)widget;
	QComboBox* b = new QComboBox( TRUE, bar );
	b->setValidator( new QIntValidator( b ) );
	
	QStringList lst = items();
	QStringList::ConstIterator it = lst.begin();
	for( ; it != lst.end(); ++it )
	    b->insertItem( *it );
	
	b->setEnabled( isEnabled() );
	
	connect( b, SIGNAL( activated( const QString& ) ), this, SLOT( slotActivated( const QString& ) ) );
	
	addContainer( bar, b );
	connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	return containerCount() - 1;
    }

    return QSelectAction::plug( widget );
}
