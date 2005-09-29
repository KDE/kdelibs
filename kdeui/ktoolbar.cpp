/* This file is part of the KDE libraries
    Copyright
    (C) 2000 Reginald Stadlbauer (reggie@kde.org)
    (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
    (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
    (C) 1997, 1998 Sven Radej (radej@kde.org)
    (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
    (C) 1999 Chris Schlaeger (cs@kde.org)
    (C) 1999 Kurt Granroth (granroth@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <config.h>

#ifdef KDE_USE_FINAL
#undef Always
#include <qdockwindow.h>
#endif

#include <string.h>

#include <qpainter.h>
#include <qtooltip.h>
#include <qdrawutil.h>
#include <qstring.h>
#include <qrect.h>
#include <qobjectlist.h>
#include <qtimer.h>
#include <qstyle.h>
#include <qlayout.h>

#include <ktoolbar.h>
#include <kmainwindow.h>
#include <klineedit.h>
#include <kseparator.h>
#include <klocale.h>
#include <kapplication.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <kpopupmenu.h>
#include <kanimwidget.h>
#include <kedittoolbar.h>
#include <kipc.h>
#include <kwin.h>
#include <kdebug.h>
#include <ktoolbarbutton.h>

enum {
    CONTEXT_TOP = 0,
    CONTEXT_LEFT = 1,
    CONTEXT_RIGHT = 2,
    CONTEXT_BOTTOM = 3,
    CONTEXT_FLOAT = 4,
    CONTEXT_FLAT = 5,
    CONTEXT_ICONS = 6,
    CONTEXT_TEXT = 7,
    CONTEXT_TEXTRIGHT = 8,
    CONTEXT_TEXTUNDER = 9,
    CONTEXT_ICONSIZES = 50 // starting point for the icon size list, put everything else before
};

class KToolBarPrivate
{
public:
    KToolBarPrivate() {
        m_iconSize     = 0;
        m_iconText     = KToolBar::IconOnly;
        m_highlight    = true;
        m_transparent  = true;
        m_honorStyle   = false;

        m_enableContext  = true;

        m_xmlguiClient   = 0;

        oldPos = Qt::DockUnmanaged;

        modified = m_isHorizontal = positioned = false;

        IconSizeDefault = 0;
        IconTextDefault = "IconOnly";

        NewLineDefault = false;
        OffsetDefault = 0;
        PositionDefault = "Top";
	HiddenDefault = false;
        idleButtons.setAutoDelete(true);
    }

    int m_iconSize;
    KToolBar::IconText m_iconText;
    bool m_highlight : 1;
    bool m_transparent : 1;
    bool m_honorStyle : 1;
    bool m_isHorizontal : 1;
    bool m_enableContext : 1;
    bool modified : 1;
    bool positioned : 1;

    QWidget *m_parent;

    QMainWindow::ToolBarDock oldPos;

    KXMLGUIClient *m_xmlguiClient;

    struct ToolBarInfo
    {
        ToolBarInfo() : index( -1 ), offset( -1 ), newline( false ), dock( Qt::DockTop ) {}
        ToolBarInfo( Qt::Dock d, int i, bool n, int o ) : index( i ), offset( o ), newline( n ), dock( d ) {}
        int index, offset;
        bool newline;
        Qt::Dock dock;
    };

    ToolBarInfo toolBarInfo;
    QValueList<int> iconSizes;
    QTimer repaintTimer;

  // Default Values.
  bool HiddenDefault;
  int IconSizeDefault;
  QString IconTextDefault;
  bool NewLineDefault;
  int OffsetDefault;
  QString PositionDefault;

   QPtrList<QWidget> idleButtons;
};

KToolBarSeparator::KToolBarSeparator(Orientation o , bool l, QToolBar *parent,
                                     const char* name )
    :QFrame( parent, name ), line( l )
{
    connect( parent, SIGNAL(orientationChanged(Orientation)),
             this, SLOT(setOrientation(Orientation)) );
    setOrientation( o );
    setBackgroundMode( parent->backgroundMode() );
    setBackgroundOrigin( ParentOrigin );
}

void KToolBarSeparator::setOrientation( Orientation o )
{
    orient = o;
    setFrameStyle( NoFrame );
}

void KToolBarSeparator::drawContents( QPainter* p )
{
    if ( line ) {
        QStyle::SFlags flags = QStyle::Style_Default;

        if ( orientation() == Horizontal )
            flags = flags | QStyle::Style_Horizontal;

        style().drawPrimitive(QStyle::PE_DockWindowSeparator, p,
                              contentsRect(), colorGroup(), flags);
    } else {
        QFrame::drawContents(p);
    }
}

void KToolBarSeparator::styleChange( QStyle& )
{
    setOrientation( orient );
}

QSize KToolBarSeparator::sizeHint() const
{
    int dim = style().pixelMetric( QStyle::PM_DockWindowSeparatorExtent, this );
    return orientation() == Vertical ? QSize( 0, dim ) : QSize( dim, 0 );
}

QSizePolicy KToolBarSeparator::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
}

KToolBar::KToolBar( QWidget *parent, const char *name, bool honorStyle, bool readConfig )
    : QToolBar( QString::fromLatin1( name ),
      dynamic_cast<QMainWindow*>(parent),
      parent, false,
      name ? name : "mainToolBar")
{
    init( readConfig, honorStyle );
}

KToolBar::KToolBar( QMainWindow *parentWindow, QMainWindow::ToolBarDock dock, bool newLine, const char *name, bool honorStyle, bool readConfig )
    : QToolBar( QString::fromLatin1( name ),
      parentWindow, dock, newLine,
      name ? name : "mainToolBar")
{
    init( readConfig, honorStyle );
}

KToolBar::KToolBar( QMainWindow *parentWindow, QWidget *dock, bool newLine, const char *name, bool honorStyle, bool readConfig )
    : QToolBar( QString::fromLatin1( name ),
      parentWindow, dock, newLine,
      name ? name : "mainToolBar")
{
    init( readConfig, honorStyle );
}

KToolBar::~KToolBar()
{
    emit toolbarDestroyed();
    delete d;
}

void KToolBar::init( bool readConfig, bool honorStyle )
{
    d = new KToolBarPrivate;
    setFullSize( true );
    d->m_honorStyle = honorStyle;
    context = 0;
    layoutTimer = new QTimer( this );
    connect( layoutTimer, SIGNAL( timeout() ),
             this, SLOT( rebuildLayout() ) );
    connect( &(d->repaintTimer), SIGNAL( timeout() ),
             this, SLOT( slotRepaint() ) );

    if ( kapp ) { // may be null when started inside designer
        connect(kapp, SIGNAL(toolbarAppearanceChanged(int)), this, SLOT(slotAppearanceChanged()));
        // request notification of changes in icon style
        kapp->addKipcEventMask(KIPC::IconChanged);
        connect(kapp, SIGNAL(iconChanged(int)), this, SLOT(slotIconChanged(int)));
    }

    // finally, read in our configurable settings
    if ( readConfig )
        slotReadConfig();

    if ( mainWindow() )
        connect( mainWindow(), SIGNAL( toolBarPositionChanged( QToolBar * ) ),
                 this, SLOT( toolBarPosChanged( QToolBar * ) ) );

    // Hack to make sure we recalculate our size when we dock.
    connect( this, SIGNAL(placeChanged(QDockWindow::Place)), SLOT(rebuildLayout()) );
}

int KToolBar::insertButton(const QString& icon, int id, bool enabled,
                            const QString& text, int index, KInstance *_instance )
{
    KToolBarButton *button = new KToolBarButton( icon, id, this, 0, text, _instance );

    insertWidgetInternal( button, index, id );
    button->setEnabled( enabled );
    doConnections( button );
    return index;
}


int KToolBar::insertButton(const QString& icon, int id, const char *signal,
                            const QObject *receiver, const char *slot,
                            bool enabled, const QString& text, int index, KInstance *_instance )
{
    KToolBarButton *button = new KToolBarButton( icon, id, this, 0, text, _instance);
    insertWidgetInternal( button, index, id );
    button->setEnabled( enabled );
    connect( button, signal, receiver, slot );
    doConnections( button );
    return index;
}


int KToolBar::insertButton(const QPixmap& pixmap, int id, bool enabled,
                            const QString& text, int index )
{
    KToolBarButton *button = new KToolBarButton( pixmap, id, this, 0, text);
    insertWidgetInternal( button, index, id );
    button->setEnabled( enabled );
    doConnections( button );
    return index;
}


int KToolBar::insertButton(const QPixmap& pixmap, int id, const char *signal,
                            const QObject *receiver, const char *slot,
                            bool enabled, const QString& text,
                            int index )
{
    KToolBarButton *button = new KToolBarButton( pixmap, id, this, 0, text);
    insertWidgetInternal( button, index, id );
    button->setEnabled( enabled );
    connect( button, signal, receiver, slot );
    doConnections( button );
    return index;
}


int KToolBar::insertButton(const QString& icon, int id, QPopupMenu *popup,
                            bool enabled, const QString &text, int index )
{
    KToolBarButton *button = new KToolBarButton( icon, id, this, 0, text );
    insertWidgetInternal( button, index, id );
    button->setEnabled( enabled );
    button->setPopup( popup );
    doConnections( button );
    return index;
}


int KToolBar::insertButton(const QPixmap& pixmap, int id, QPopupMenu *popup,
                            bool enabled, const QString &text, int index )
{
    KToolBarButton *button = new KToolBarButton( pixmap, id, this, 0, text );
    insertWidgetInternal( button, index, id );
    button->setEnabled( enabled );
    button->setPopup( popup );
    doConnections( button );
    return index;
}


int KToolBar::insertLined (const QString& text, int id,
                            const char *signal,
                            const QObject *receiver, const char *slot,
                            bool enabled ,
                            const QString& toolTipText,
                            int size, int index )
{
    KLineEdit *lined = new KLineEdit ( this, 0 );
    if ( !toolTipText.isEmpty() )
        QToolTip::add( lined, toolTipText );
    if ( size > 0 )
        lined->setMinimumWidth( size );
    insertWidgetInternal( lined, index, id );
    connect( lined, signal, receiver, slot );
    lined->setText(text);
    lined->setEnabled( enabled );
    return index;
}

int KToolBar::insertCombo (const QStringList &list, int id, bool writable,
                            const char *signal, const QObject *receiver,
                            const char *slot, bool enabled,
                            const QString& tooltiptext,
                            int size, int index,
                            QComboBox::Policy policy )
{
    KComboBox *combo = new KComboBox ( writable, this );

    insertWidgetInternal( combo, index, id );
    combo->insertStringList (list);
    combo->setInsertionPolicy(policy);
    combo->setEnabled( enabled );
    if ( size > 0 )
        combo->setMinimumWidth( size );
    if (!tooltiptext.isNull())
        QToolTip::add( combo, tooltiptext );

    if ( signal && receiver && slot )
        connect ( combo, signal, receiver, slot );
    return index;
}


int KToolBar::insertCombo (const QString& text, int id, bool writable,
                            const char *signal, QObject *receiver,
                            const char *slot, bool enabled,
                            const QString& tooltiptext,
                            int size, int index,
                            QComboBox::Policy policy )
{
    KComboBox *combo = new KComboBox ( writable, this );
    insertWidgetInternal( combo, index, id );
    combo->insertItem (text);
    combo->setInsertionPolicy(policy);
    combo->setEnabled( enabled );
    if ( size > 0 )
        combo->setMinimumWidth( size );
    if (!tooltiptext.isNull())
        QToolTip::add( combo, tooltiptext );
    connect (combo, signal, receiver, slot);
    return index;
}

int KToolBar::insertSeparator(int index, int id)
{
    QWidget *w = new KToolBarSeparator( orientation(), false, this, "tool bar separator" );
    insertWidgetInternal( w, index, id );
    return index;
}

int KToolBar::insertLineSeparator(int index, int id)
{
    QWidget *w = new KToolBarSeparator( orientation(), true, this, "tool bar separator" );
    insertWidgetInternal( w, index, id );
    return index;
}


int KToolBar::insertWidget(int id, int /*width*/, QWidget *widget, int index)
{
    removeWidgetInternal( widget ); // in case we already have it ?
    insertWidgetInternal( widget, index, id );
    return index;
}

int KToolBar::insertAnimatedWidget(int id, QObject *receiver, const char *slot,
                                    const QString& icons, int index )
{
    KAnimWidget *anim = new KAnimWidget( icons, d->m_iconSize, this );
    insertWidgetInternal( anim, index, id );

    if ( receiver )
        connect( anim, SIGNAL(clicked()), receiver, slot);

    return index;
}

KAnimWidget *KToolBar::animatedWidget( int id )
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return 0;
    KAnimWidget *aw = dynamic_cast<KAnimWidget *>(*it);
    if ( aw )
        return aw;
    QObjectList *l = queryList( "KAnimWidget" );
    if ( !l || !l->first() ) {
        delete l;
        return 0;
    }

    for ( QObject *o = l->first(); o; o = l->next() ) {
        KAnimWidget *aw = dynamic_cast<KAnimWidget *>(o);
        if ( aw )
        {
            delete l;
            return aw;
        }
    }

    delete l;
    return 0;
}


void KToolBar::addConnection (int id, const char *signal,
                               const QObject *receiver, const char *slot)
{
    QWidget* w = getWidget( id );
    if ( w )
        connect( w, signal, receiver, slot );
}

void KToolBar::setItemEnabled( int id, bool enabled )
{
    QWidget* w = getWidget( id );
    if ( w )
        w->setEnabled( enabled );
}


void KToolBar::setButtonPixmap( int id, const QPixmap& _pixmap )
{
    KToolBarButton * button = getButton( id );
    if ( button )
        button->setPixmap( _pixmap );
}


void KToolBar::setButtonIcon( int id, const QString& _icon )
{
    KToolBarButton * button = getButton( id );
    if ( button )
        button->setIcon( _icon );
}

void KToolBar::setButtonIconSet( int id, const QIconSet& iconset )
{
    KToolBarButton * button = getButton( id );
    if ( button )
        button->setIconSet( iconset );
}


void KToolBar::setDelayedPopup (int id , QPopupMenu *_popup, bool toggle )
{
    KToolBarButton * button = getButton( id );
    if ( button )
        button->setDelayedPopup( _popup, toggle );
}


void KToolBar::setAutoRepeat (int id, bool flag)
{
    KToolBarButton * button = getButton( id );
    if ( button )
        button->setAutoRepeat( flag );
}


void KToolBar::setToggle (int id, bool flag )
{
    KToolBarButton * button = getButton( id );
    if ( button )
        button->setToggle( flag );
}


void KToolBar::toggleButton (int id)
{
    KToolBarButton * button = getButton( id );
    if ( button )
        button->toggle();
}


void KToolBar::setButton (int id, bool flag)
{
    KToolBarButton * button = getButton( id );
    if ( button )
        button->on( flag );
}


bool KToolBar::isButtonOn (int id) const
{
    KToolBarButton * button = const_cast<KToolBar*>( this )->getButton( id );
    return button ? button->isOn() : false;
}


void KToolBar::setLinedText (int id, const QString& text)
{
    KLineEdit * lineEdit = getLined( id );
    if ( lineEdit )
        lineEdit->setText( text );
}


QString KToolBar::getLinedText (int id) const
{
    KLineEdit * lineEdit = const_cast<KToolBar*>( this )->getLined( id );
    return lineEdit ? lineEdit->text() : QString::null;
}


void KToolBar::insertComboItem (int id, const QString& text, int index)
{
    KComboBox * comboBox = getCombo( id );
    if (comboBox)
        comboBox->insertItem( text, index );
}

void KToolBar::insertComboList (int id, const QStringList &list, int index)
{
    KComboBox * comboBox = getCombo( id );
    if (comboBox)
        comboBox->insertStringList( list, index );
}


void KToolBar::removeComboItem (int id, int index)
{
    KComboBox * comboBox = getCombo( id );
    if (comboBox)
        comboBox->removeItem( index );
}


void KToolBar::setCurrentComboItem (int id, int index)
{
    KComboBox * comboBox = getCombo( id );
    if (comboBox)
        comboBox->setCurrentItem( index );
}


void KToolBar::changeComboItem  (int id, const QString& text, int index)
{
    KComboBox * comboBox = getCombo( id );
    if (comboBox)
        comboBox->changeItem( text, index );
}


void KToolBar::clearCombo (int id)
{
    KComboBox * comboBox = getCombo( id );
    if (comboBox)
        comboBox->clear();
}


QString KToolBar::getComboItem (int id, int index) const
{
    KComboBox * comboBox = const_cast<KToolBar*>( this )->getCombo( id );
    return comboBox ? comboBox->text( index ) : QString::null;
}


KComboBox * KToolBar::getCombo(int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return 0;
    return dynamic_cast<KComboBox *>( *it );
}


KLineEdit * KToolBar::getLined (int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return 0;
    return dynamic_cast<KLineEdit *>( *it );
}


KToolBarButton * KToolBar::getButton (int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return 0;
    return dynamic_cast<KToolBarButton *>( *it );
}


void KToolBar::alignItemRight (int id, bool right )
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    if ( rightAligned && !right && (*it) == rightAligned )
        rightAligned = 0;
    if ( (*it) && right )
        rightAligned = (*it);
}


QWidget *KToolBar::getWidget (int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    return ( it == id2widget.end() ) ? 0 : (*it);
}


void KToolBar::setItemAutoSized (int id, bool yes )
{
    QWidget *w = getWidget(id);
    if ( w && yes )
        setStretchableWidget( w );
}


void KToolBar::clear ()
{
    /* Delete any idle buttons, so QToolBar doesn't delete them itself, making a mess */
    for(QWidget *w=d->idleButtons.first(); w; w=d->idleButtons.next())
       w->blockSignals(false);    
    d->idleButtons.clear();
     
    QToolBar::clear();
    widget2id.clear();
    id2widget.clear();
}


void KToolBar::removeItem(int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
    {
        kdDebug(220) << name() << " KToolBar::removeItem item " << id << " not found" << endl;
        return;
    }
    QWidget * w = (*it);
    id2widget.remove( id );
    widget2id.remove( w );
    widgets.removeRef( w );
    delete w;
}


void KToolBar::removeItemDelayed(int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
    {
        kdDebug(220) << name() << " KToolBar::removeItem item " << id << " not found" << endl;
        return;
    }
    QWidget * w = (*it);
    id2widget.remove( id );
    widget2id.remove( w );
    widgets.removeRef( w );

    w->blockSignals(true);
    d->idleButtons.append(w);
    layoutTimer->start( 50, true );
}


void KToolBar::hideItem (int id)
{
    QWidget *w = getWidget(id);
    if ( w )
        w->hide();
}


void KToolBar::showItem (int id)
{
    QWidget *w = getWidget(id);
    if ( w )
        w->show();
}


int KToolBar::itemIndex (int id)
{
    QWidget *w = getWidget(id);
    return w ? widgets.findRef(w) : -1;
}

int KToolBar::idAt (int index)
{
    QWidget *w = widgets.at(index);
    return widget2id[w];
}

void KToolBar::setFullSize(bool flag )
{
    setHorizontalStretchable( flag );
    setVerticalStretchable( flag );
}


bool KToolBar::fullSize() const
{
    return isHorizontalStretchable() || isVerticalStretchable();
}


void KToolBar::enableMoving(bool flag )
{
    setMovingEnabled(flag);
}


void KToolBar::setBarPos (BarPosition bpos)
{
    if ( !mainWindow() )
        return;
    mainWindow()->moveDockWindow( this, (Dock)bpos );
    //kdDebug(220) << name() << " setBarPos dockWindowIndex=" << dockWindowIndex() << endl;
}


KToolBar::BarPosition KToolBar::barPos() const
{
    if ( !this->mainWindow() )
        return KToolBar::Top;
    Dock dock;
    int dm1, dm2;
    bool dm3;
    this->mainWindow()->getLocation( (QToolBar*)this, dock, dm1, dm3, dm2 );
    if ( dock == DockUnmanaged ) {
        return (KToolBar::BarPosition)DockTop;
    }
    return (BarPosition)dock;
}


bool KToolBar::enable(BarStatus stat)
{
    bool mystat = isVisible();

    if ( (stat == Toggle && mystat) || stat == Hide )
        hide();
    else
        show();

    return isVisible() == mystat;
}


void KToolBar::setMaxHeight ( int h )
{
    setMaximumHeight( h );
}

int KToolBar::maxHeight()
{
    return maximumHeight();
}


void KToolBar::setMaxWidth (int dw)
{
    setMaximumWidth( dw );
}


int KToolBar::maxWidth()
{
    return maximumWidth();
}


void KToolBar::setTitle (const QString& _title)
{
    setLabel( _title );
}


void KToolBar::enableFloating (bool )
{
}


void KToolBar::setIconText(IconText it)
{
    setIconText( it, true );
}


void KToolBar::setIconText(IconText icontext, bool update)
{
    bool doUpdate=false;

    if (icontext != d->m_iconText) {
        d->m_iconText = icontext;
        doUpdate=true;
        //kdDebug(220) << name() << "  icontext has changed, doUpdate=true" << endl;
    }
    else {
        //kdDebug(220) << name() << "  icontext hasn't changed, doUpdate=false" << endl;
    }

    if (!update)
        return;

    if (doUpdate)
        doModeChange(); // tell buttons what happened

    // ugly hack to force a QMainWindow::triggerLayout( true )
    QMainWindow *mw = mainWindow();
    if ( mw ) {
        mw->setUpdatesEnabled( false );
        mw->setToolBarsMovable( !mw->toolBarsMovable() );
        mw->setToolBarsMovable( !mw->toolBarsMovable() );
        mw->setUpdatesEnabled( true );
    }
}


KToolBar::IconText KToolBar::iconText() const
{
    return d->m_iconText;
}


void KToolBar::setIconSize(int size)
{
    setIconSize( size, true );
}

void KToolBar::setIconSize(int size, bool update)
{
    bool doUpdate=false;

    if ( size != d->m_iconSize ) {
            d->m_iconSize = size;
            doUpdate=true;
    }

    if (!update)
        return;

    if (doUpdate)
        doModeChange(); // tell buttons what happened

    // ugly hack to force a QMainWindow::triggerLayout( true )
    if ( mainWindow() ) {
        QMainWindow *mw = mainWindow();
        mw->setUpdatesEnabled( false );
        mw->setToolBarsMovable( !mw->toolBarsMovable() );
        mw->setToolBarsMovable( !mw->toolBarsMovable() );
        mw->setUpdatesEnabled( true );
    }
}


int KToolBar::iconSize() const
{
    if ( !d->m_iconSize ) // default value?
		return iconSizeDefault();

	return d->m_iconSize;
}

int KToolBar::iconSizeDefault() const
{
	if (!::qstrcmp(QObject::name(), "mainToolBar"))
		return KGlobal::iconLoader()->currentSize(KIcon::MainToolbar);

	return KGlobal::iconLoader()->currentSize(KIcon::Toolbar);
}

void KToolBar::setEnableContextMenu(bool enable )
{
    d->m_enableContext = enable;
}


bool KToolBar::contextMenuEnabled() const
{
    return d->m_enableContext;
}


void KToolBar::setItemNoStyle(int id, bool no_style )
{
    KToolBarButton * button = getButton( id );
    if (button)
        button->setNoStyle( no_style );
}


void KToolBar::setFlat (bool flag)
{
    if ( !mainWindow() )
        return;
    if ( flag )
        mainWindow()->moveDockWindow( this, DockMinimized );
    else
        mainWindow()->moveDockWindow( this, DockTop );
    // And remember to save the new look later
    KMainWindow *kmw = dynamic_cast<KMainWindow *>(mainWindow());
    if ( kmw )
        kmw->setSettingsDirty();
}


int KToolBar::count() const
{
    return id2widget.count();
}


void KToolBar::saveState()
{
    // first, try to save to the xml file
    if ( d->m_xmlguiClient && !d->m_xmlguiClient->xmlFile().isEmpty() ) {
        //kdDebug(220) << name() << " saveState: saving to " << d->m_xmlguiClient->xmlFile() << endl;
        QString barname(!::qstrcmp(name(), "unnamed") ? "mainToolBar" : name());
        // try to find our toolbar
        d->modified = false;
        // go down one level to get to the right tags
        QDomElement current;
        for( QDomNode n = d->m_xmlguiClient->domDocument().documentElement().firstChild();
             !n.isNull(); n = n.nextSibling()) {
            current = n.toElement();

            if ( current.tagName().lower() != "toolbar" )
                continue;

            QString curname(current.attribute( "name" ));

            if ( curname == barname ) {
                saveState( current );
                break;
            }
        }
        // if we didn't make changes, then just return
        if ( !d->modified )
            return;

        // now we load in the (non-merged) local file
        QString local_xml(KXMLGUIFactory::readConfigFile(d->m_xmlguiClient->xmlFile(), true, d->m_xmlguiClient->instance()));
        QDomDocument local;
        local.setContent(local_xml);

        // make sure we don't append if this toolbar already exists locally
        bool just_append = true;

        for( QDomNode n = local.documentElement().firstChild();
             !n.isNull(); n = n.nextSibling()) {
            QDomElement elem = n.toElement();

            if ( elem.tagName().lower() != "toolbar" )
                continue;

            QString curname(elem.attribute( "name" ));

            if ( curname == barname ) {
                just_append = false;
                local.documentElement().replaceChild( current, elem );
                break;
            }
        }

        if (just_append)
            local.documentElement().appendChild( current );

        KXMLGUIFactory::saveConfigFile(local, d->m_xmlguiClient->localXMLFile(), d->m_xmlguiClient->instance() );

        return;
    }

    // if that didn't work, we save to the config file
    KConfig *config = KGlobal::config();
    saveSettings(config, QString::null);
    config->sync();
}

QString KToolBar::settingsGroup() const
{
    QString configGroup;
    if (!::qstrcmp(name(), "unnamed") || !::qstrcmp(name(), "mainToolBar"))
        configGroup = "Toolbar style";
    else
        configGroup = QString(name()) + " Toolbar style";
    if ( this->mainWindow() )
    {
        configGroup.prepend(" ");
        configGroup.prepend( this->mainWindow()->name() );
    }
    return configGroup;
}

void KToolBar::saveSettings(KConfig *config, const QString &_configGroup)
{
    QString configGroup = _configGroup;
    if (configGroup.isEmpty())
        configGroup = settingsGroup();
    //kdDebug(220) << name() << " saveSettings() group=" << _configGroup << " -> " << configGroup << endl;

    QString position, icontext;
    int index;
    getAttributes( position, icontext, index );

    //kdDebug(220) << name() << "                position=" << position << " index=" << index << " offset=" << offset() << " newLine=" << newLine() << endl;

    KConfigGroupSaver saver(config, configGroup);

    if(!config->hasDefault("Position") && position == d->PositionDefault )
      config->revertToDefault("Position");
    else
      config->writeEntry("Position", position);

    //kdDebug(220) << name() << "                icontext=" << icontext << " hasDefault:" << config->hasDefault( "IconText" ) << " d->IconTextDefault=" << d->IconTextDefault << endl;

    if(d->m_honorStyle && icontext == d->IconTextDefault && !config->hasDefault("IconText") )
    {
      //kdDebug(220) << name() << "                reverting icontext to default" << endl;
      config->revertToDefault("IconText");
    }
    else
    {
      //kdDebug(220) << name() << "                writing icontext " << icontext << endl;
      config->writeEntry("IconText", icontext);
    }

    if(!config->hasDefault("IconSize") && iconSize() == iconSizeDefault() )
      config->revertToDefault("IconSize");
    else
      config->writeEntry("IconSize", iconSize());

    if(!config->hasDefault("Hidden") && isHidden() == d->HiddenDefault )
      config->revertToDefault("Hidden");
    else
      config->writeEntry("Hidden", isHidden());

    // Note that index, unlike the other settings, depends on the other toolbars
    // So on the first run with a clean local config file, even the usual
    // hasDefault/==IndexDefault test would save the toolbar indexes
    // (IndexDefault was 0, whereas index is the real index in the GUI)
    //
    // Saving the whole set of indexes is necessary though. When moving only
    // one toolbar, if we only saved the changed indexes, the toolbars wouldn't
    // reappear at the same position the next time.
    // The whole set of indexes has to be saved.
    //kdDebug(220) << name() << "                writing index " << index << endl;
    KMainWindow *kmw = dynamic_cast<KMainWindow *>(mainWindow());
    // don't save if there's only one toolbar

    // Don't use kmw->toolBarIterator() because you might
    // mess up someone else's iterator.  Make the list on your own
    QPtrList<KToolBar> toolbarList;
    QPtrList<QToolBar> lst;
    for ( int i = (int)QMainWindow::DockUnmanaged; i <= (int)DockMinimized; ++i ) {
        lst = kmw->toolBars( (ToolBarDock)i );
        for ( QToolBar *tb = lst.first(); tb; tb = lst.next() ) {
            if ( !tb->inherits( "KToolBar" ) )
                continue;
            toolbarList.append( (KToolBar*)tb );
        }
    }
    QPtrListIterator<KToolBar> toolbarIterator( toolbarList );
    if ( !kmw || toolbarIterator.count() > 1 )
        config->writeEntry("Index", index);
    else
        config->revertToDefault("Index");

    if(!config->hasDefault("Offset") && offset() == d->OffsetDefault )
      config->revertToDefault("Offset");
    else
      config->writeEntry("Offset", offset());

    if(!config->hasDefault("NewLine") && newLine() == d->NewLineDefault )
      config->revertToDefault("NewLine");
    else
      config->writeEntry("NewLine", newLine());
}


void KToolBar::setXMLGUIClient( KXMLGUIClient *client )
{
    d->m_xmlguiClient = client;
}

void KToolBar::setText( const QString & txt )
{
    setLabel( txt + " (" + kapp->caption() + ") " );
}


QString KToolBar::text() const
{
    return label();
}


void KToolBar::doConnections( KToolBarButton *button )
{
    connect(button, SIGNAL(clicked(int)), this, SIGNAL( clicked( int ) ) );
    connect(button, SIGNAL(doubleClicked(int)), this, SIGNAL( doubleClicked( int ) ) );
    connect(button, SIGNAL(released(int)), this, SIGNAL( released( int ) ) );
    connect(button, SIGNAL(pressed(int)), this, SIGNAL( pressed( int ) ) );
    connect(button, SIGNAL(toggled(int)), this, SIGNAL( toggled( int ) ) );
    connect(button, SIGNAL(highlighted(int, bool)), this, SIGNAL( highlighted( int, bool ) ) );
}

void KToolBar::mousePressEvent ( QMouseEvent *m )
{
    if ( !mainWindow() )
        return;
    QMainWindow *mw = mainWindow();
    if ( mw->toolBarsMovable() && d->m_enableContext ) {
        if ( m->button() == RightButton ) {
	    QGuardedPtr<KToolBar> guard( this );
            int i = contextMenu()->exec( m->globalPos(), 0 );
	    // "Configure Toolbars" recreates toolbars, so we might not exist anymore.
	    if ( guard )
                slotContextAboutToHide();
            switch ( i ) {
            case -1:
                return; // popup canceled
            case CONTEXT_LEFT:
                mw->moveDockWindow( this, DockLeft );
                break;
            case CONTEXT_RIGHT:
                mw->moveDockWindow( this, DockRight );
                break;
            case CONTEXT_TOP:
                mw->moveDockWindow( this, DockTop );
                break;
            case CONTEXT_BOTTOM:
                mw->moveDockWindow( this, DockBottom );
                break;
            case CONTEXT_FLOAT:
                mw->moveDockWindow( this, DockTornOff );
                break;
            case CONTEXT_FLAT:
                mw->moveDockWindow( this, DockMinimized );
                break;
            case CONTEXT_ICONS:
                setIconText( IconOnly );
                break;
            case CONTEXT_TEXTRIGHT:
                setIconText( IconTextRight );
                break;
            case CONTEXT_TEXT:
                setIconText( TextOnly );
                break;
            case CONTEXT_TEXTUNDER:
                setIconText( IconTextBottom );
                break;
            default:
                if ( i >= CONTEXT_ICONSIZES )
					setIconSize( i - CONTEXT_ICONSIZES );
                else
                    return; // assume this was an action handled elsewhere, no need for setSettingsDirty()
            }
            KMainWindow *kmw = dynamic_cast<KMainWindow *>(mw);
            if ( kmw )
                kmw->setSettingsDirty();
        }
    }
}

void KToolBar::doModeChange()
{
    for(QWidget *w=d->idleButtons.first(); w; w=d->idleButtons.next())
       w->blockSignals(false);
    d->idleButtons.clear();

    emit modechange();
}

void KToolBar::rebuildLayout()
{
    for(QWidget *w=d->idleButtons.first(); w; w=d->idleButtons.next())
       w->blockSignals(false);
    d->idleButtons.clear();

    layoutTimer->stop();
    QApplication::sendPostedEvents( this, QEvent::ChildInserted );
    QBoxLayout *l = boxLayout();

    // clear the old layout
    QLayoutIterator it = l->iterator();
    while ( it.current() )
        it.deleteCurrent();

    for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
        if ( w == rightAligned )
            continue;
        KToolBarSeparator *ktbs = dynamic_cast<KToolBarSeparator *>(w);
        if ( ktbs && !ktbs->showLine() ) {
            l->addSpacing( orientation() == Vertical ? w->sizeHint().height() : w->sizeHint().width() );
            w->hide();
            continue;
        }
        if ( dynamic_cast<QPopupMenu *>(w) ) // w is a QPopupMenu?
            continue;
        l->addWidget( w );
        w->show();
        if ((orientation() == Horizontal) && dynamic_cast<QLineEdit *>(w)) // w is QLineEdit ?
            l->addSpacing(2); // A little bit extra spacing behind it.
    }
    if ( rightAligned ) {
        l->addStretch();
        l->addWidget( rightAligned );
        rightAligned->show();
    }

    if ( fullSize() ) {
        if ( !rightAligned )
            l->addStretch();
        if ( stretchableWidget )
            l->setStretchFactor( stretchableWidget, 10 );
    }
    l->invalidate();
    QApplication::postEvent( this, new QEvent( QEvent::LayoutHint ) );
}

void KToolBar::childEvent( QChildEvent *e )
{
    if ( e->child()->isWidgetType() ) {
        QWidget * w = dynamic_cast<QWidget *>(e->child());
        if (!w || !(::qstrcmp( "qt_dockwidget_internal", w->name())))
        {
           QToolBar::childEvent( e );
           return;
        }
        if ( e->type() == QEvent::ChildInserted ) {
            if ( !dynamic_cast<QPopupMenu *>(w)) { // e->child() is not a QPopupMenu
                // prevent items that have been explicitly inserted by insert*() from
                // being inserted again
                if ( !widget2id.contains( w ) )
                {
                    int dummy = -1;
                    insertWidgetInternal( w, dummy, -1 );
                }
            }
        } else {
            removeWidgetInternal( w );
        }
        if ( isVisibleTo( 0 ) )
        {
            layoutTimer->start( 50, true );
            QBoxLayout *l = boxLayout();

            // clear the old layout so that we don't get unnecassery layout
            // changes till we have rebuild the thing
            QLayoutIterator it = l->iterator();
            while ( it.current() )
               it.deleteCurrent();
        }
    }
    QToolBar::childEvent( e );
}

void KToolBar::insertWidgetInternal( QWidget *w, int &index, int id )
{
    // we can't have it in widgets, or something is really wrong
    //widgets.removeRef( w );

    connect( w, SIGNAL( destroyed() ),
             this, SLOT( widgetDestroyed() ) );
    if ( index == -1 || index > (int)widgets.count() ) {
        index = (int)widgets.count();
        widgets.append( w );
    }
    else
        widgets.insert( index, w );
    if ( id == -1 )
        id = id2widget.count();
    id2widget.insert( id, w );
    widget2id.insert( w, id );
}

void KToolBar::showEvent( QShowEvent *e )
{
    QToolBar::showEvent( e );
    rebuildLayout();
}

void KToolBar::setStretchableWidget( QWidget *w )
{
    QToolBar::setStretchableWidget( w );
    stretchableWidget = w;
}

QSizePolicy KToolBar::sizePolicy() const
{
    if ( orientation() == Horizontal )
        return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    else
        return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
}

QSize KToolBar::sizeHint() const
{
    QSize minSize(0,0);
    KToolBar *ncThis = const_cast<KToolBar *>(this);

    ncThis->polish();

    int margin = static_cast<QWidget*>(ncThis)->layout()->margin() + frameWidth();
    switch( barPos() )
    {
     case KToolBar::Top:
     case KToolBar::Bottom:
       for ( QWidget *w = ncThis->widgets.first(); w; w = ncThis->widgets.next() )
       {
          QSize sh = w->sizeHint();
          if ( w->sizePolicy().horData() == QSizePolicy::Ignored )
             sh.setWidth( 1 );
          if ( w->sizePolicy().verData() == QSizePolicy::Ignored )
             sh.setHeight( 1 );
          sh = sh.boundedTo( w->maximumSize() )
                 .expandedTo( w->minimumSize() ).expandedTo( QSize(1, 1) );

          minSize = minSize.expandedTo(QSize(0, sh.height()));
          minSize += QSize(sh.width()+1, 0);
          if (dynamic_cast<QLineEdit *>(w)) // w is a QLineEdit ?
             minSize += QSize(2, 0); // A little bit extra spacing behind it.
       }

       minSize += QSize(QApplication::style().pixelMetric( QStyle::PM_DockWindowHandleExtent ), 0);
       minSize += QSize(margin*2, margin*2);
       break;

     case KToolBar::Left:
     case KToolBar::Right:
       for ( QWidget *w = ncThis->widgets.first(); w; w = ncThis->widgets.next() )
       {
          QSize sh = w->sizeHint();
          if ( w->sizePolicy().horData() == QSizePolicy::Ignored )
             sh.setWidth( 1 );
          if ( w->sizePolicy().verData() == QSizePolicy::Ignored )
             sh.setHeight( 1 );
          sh = sh.boundedTo( w->maximumSize() )
                 .expandedTo( w->minimumSize() ).expandedTo( QSize(1, 1) );

          minSize = minSize.expandedTo(QSize(sh.width(), 0));
          minSize += QSize(0, sh.height()+1);
       }
       minSize += QSize(0, QApplication::style().pixelMetric( QStyle::PM_DockWindowHandleExtent ));
       minSize += QSize(margin*2, margin*2);
       break;

     default:
       minSize = QToolBar::sizeHint();
       break;
    }
    return minSize;
}

QSize KToolBar::minimumSize() const
{
    return minimumSizeHint();
}

QSize KToolBar::minimumSizeHint() const
{
    return sizeHint();
}

bool KToolBar::highlight() const
{
    return d->m_highlight;
}

void KToolBar::hide()
{
    QToolBar::hide();
}

void KToolBar::show()
{
    QToolBar::show();
}

void KToolBar::resizeEvent( QResizeEvent *e )
{
    bool b = isUpdatesEnabled();
    setUpdatesEnabled( false );
    QToolBar::resizeEvent( e );
    if (b)
    {
      if (layoutTimer->isActive())
      {
         // Wait with repainting till layout is complete.
         d->repaintTimer.start( 100, true );
      }
      else
      {
         // Repaint now
         slotRepaint();
      }
    }
}

void KToolBar::slotIconChanged(int group)
{
    if ((group != KIcon::Toolbar) && (group != KIcon::MainToolbar))
        return;
    if ((group == KIcon::MainToolbar) != !::qstrcmp(name(), "mainToolBar"))
        return;

    doModeChange();

    if (isVisible())
        updateGeometry();
}

void KToolBar::slotReadConfig()
{
    //kdDebug(220) << name() << " slotReadConfig" << endl;
    // Read appearance settings (hmm, we used to do both here,
    // but a well behaved application will call applyMainWindowSettings
    // anyway, right ?)
    applyAppearanceSettings(KGlobal::config(), QString::null );
}

void KToolBar::slotAppearanceChanged()
{
    // Read appearance settings from global file.
    applyAppearanceSettings(KGlobal::config(), QString::null, true /* lose local settings */ );

    // And remember to save the new look later
    KMainWindow *kmw = dynamic_cast<KMainWindow *>(mainWindow());
    if ( kmw )
        kmw->setSettingsDirty();
}

//static
bool KToolBar::highlightSetting()
{
    QString grpToolbar(QString::fromLatin1("Toolbar style"));
    KConfigGroupSaver saver(KGlobal::config(), grpToolbar);
    return KGlobal::config()->readBoolEntry(QString::fromLatin1("Highlighting"),true);
}

//static
bool KToolBar::transparentSetting()
{
    QString grpToolbar(QString::fromLatin1("Toolbar style"));
    KConfigGroupSaver saver(KGlobal::config(), grpToolbar);
    return KGlobal::config()->readBoolEntry(QString::fromLatin1("TransparentMoving"),true);
}

//static
KToolBar::IconText KToolBar::iconTextSetting()
{
    QString grpToolbar(QString::fromLatin1("Toolbar style"));
    KConfigGroupSaver saver(KGlobal::config(), grpToolbar);
    QString icontext = KGlobal::config()->readEntry(QString::fromLatin1("IconText"),QString::fromLatin1("IconOnly"));
    if ( icontext == "IconTextRight" )
        return IconTextRight;
    else if ( icontext == "IconTextBottom" )
        return IconTextBottom;
    else if ( icontext == "TextOnly" )
        return TextOnly;
    else
        return IconOnly;
}

void KToolBar::applyAppearanceSettings(KConfig *config, const QString &_configGroup, bool forceGlobal)
{
    QString configGroup = _configGroup.isEmpty() ? settingsGroup() : _configGroup;
    //kdDebug(220) << name() << " applyAppearanceSettings: configGroup=" << configGroup << " forceGlobal=" << forceGlobal << endl;

    // If we have application-specific settings in the XML file,
    // and nothing in the application's config file, then
    // we don't apply the global defaults, the XML ones are preferred
    // (see applySettings for a full explanation)
    // This is the reason for the xmlgui tests below.
    bool xmlgui = d->m_xmlguiClient && !d->m_xmlguiClient->xmlFile().isEmpty();

    KConfig *gconfig = KGlobal::config();

    static const QString &attrIconText  = KGlobal::staticQString("IconText");
    static const QString &attrHighlight = KGlobal::staticQString("Highlighting");
    static const QString &attrTrans     = KGlobal::staticQString("TransparentMoving");
    static const QString &attrIconSize  = KGlobal::staticQString("IconSize");

    // we actually do this in two steps.
    // First, we read in the global styles [Toolbar style] (from the KControl module).
    // Then, if the toolbar is NOT 'mainToolBar', we will also try to read in [barname Toolbar style]
    bool highlight;
    int transparent;
    bool applyIconText = !xmlgui; // if xmlgui is used, global defaults won't apply
    bool applyIconSize = !xmlgui;

    int iconSize = d->IconSizeDefault;
    QString iconText = d->IconTextDefault;

    // this is the first iteration
    QString grpToolbar(QString::fromLatin1("Toolbar style"));
    { // start block for KConfigGroupSaver
        KConfigGroupSaver saver(gconfig, grpToolbar);

        // first, get the generic settings
        highlight   = gconfig->readBoolEntry(attrHighlight, true);
        transparent = gconfig->readBoolEntry(attrTrans, true);

        // we read in the IconText property *only* if we intend on actually
        // honoring it
        if (d->m_honorStyle)
            d->IconTextDefault = gconfig->readEntry(attrIconText, d->IconTextDefault);
        else
            d->IconTextDefault = "IconOnly";

        // Use the default icon size for toolbar icons.
        d->IconSizeDefault = gconfig->readNumEntry(attrIconSize, d->IconSizeDefault);

        iconSize = d->IconSizeDefault;
        iconText = d->IconTextDefault;

        if ( !forceGlobal && config->hasGroup(configGroup) )
        {
            config->setGroup(configGroup);

            // first, get the generic settings
            highlight   = config->readBoolEntry(attrHighlight, highlight);
            transparent = config->readBoolEntry(attrTrans, transparent);

            // read in the IconText property
            if ( config->hasKey( attrIconText ) ) {
                iconText = config->readEntry(attrIconText);
                applyIconText = true;
                //kdDebug(220) << name() << " read icontext=" << d->IconTextDefault << ", that will be the default" << endl;
            }

            // now get the size
            if ( config->hasKey( attrIconSize ) ) {
                iconSize = config->readNumEntry(attrIconSize);
                applyIconSize = true;
            }
        }

        // revert back to the old group
    } // end block for KConfigGroupSaver

    bool doUpdate = false;

    IconText icon_text;
    if ( iconText == "IconTextRight" )
        icon_text = IconTextRight;
    else if ( iconText == "IconTextBottom" )
        icon_text = IconTextBottom;
    else if ( iconText == "TextOnly" )
        icon_text = TextOnly;
    else
        icon_text = IconOnly;

    // check if the icon/text has changed
    if (icon_text != d->m_iconText && applyIconText) {
        //kdDebug(220) << name() << " applyAppearanceSettings setIconText " << icon_text << endl;
        setIconText(icon_text, false);
        doUpdate = true;
    }

    // ...and check if the icon size has changed
    if (iconSize != d->m_iconSize && applyIconSize) {
        setIconSize(iconSize, false);
        doUpdate = true;
    }

    QMainWindow *mw = mainWindow();

    // ...and if we should highlight
    if ( highlight != d->m_highlight ) {
        d->m_highlight = highlight;
        doUpdate = true;
    }

    // ...and if we should move transparently
    if ( mw && transparent != (!mw->opaqueMoving()) ) {
        mw->setOpaqueMoving( !transparent );
    }

    if (doUpdate)
        doModeChange(); // tell buttons what happened

    if (isVisible ())
        updateGeometry();
}

void KToolBar::applySettings(KConfig *config, const QString &_configGroup)
{
    return applySettings(config,_configGroup,false);
}

void KToolBar::applySettings(KConfig *config, const QString &_configGroup, bool force)
{
    //kdDebug(220) << name() << " applySettings group=" << _configGroup << endl;

    QString configGroup = _configGroup.isEmpty() ? settingsGroup() : _configGroup;

    /*
      Let's explain this a bit more in details.
      The order in which we apply settings is :
       Global config / <appnamerc> user settings                        if no XMLGUI is used
       Global config / App-XML attributes / <appnamerc> user settings   if XMLGUI is used

      So in the first case, we simply read everything from KConfig as below,
      but in the second case we don't do anything here if there is no app-specific config,
      and the XMLGUI-related code (loadState()) uses the static methods of this class
      to get the global defaults.

      Global config doesn't include position (index, offset, newline and hidden/shown).
    */

    // First the appearance stuff - the one which has a global config
    applyAppearanceSettings( config, configGroup );

    // ...and now the position stuff
    if ( config->hasGroup(configGroup) || force )
    {
        KConfigGroupSaver cgs(config, configGroup);

        static const QString &attrPosition  = KGlobal::staticQString("Position");
        static const QString &attrIndex  = KGlobal::staticQString("Index");
        static const QString &attrOffset  = KGlobal::staticQString("Offset");
        static const QString &attrNewLine  = KGlobal::staticQString("NewLine");
        static const QString &attrHidden  = KGlobal::staticQString("Hidden");

        QString position = config->readEntry(attrPosition, d->PositionDefault);
        int index = config->readNumEntry(attrIndex, -1);
        int offset = config->readNumEntry(attrOffset, d->OffsetDefault);
        bool newLine = config->readBoolEntry(attrNewLine, d->NewLineDefault);
        bool hidden = config->readBoolEntry(attrHidden, d->HiddenDefault);

        Dock pos(DockTop);
        if ( position == "Top" )
            pos = DockTop;
        else if ( position == "Bottom" )
            pos = DockBottom;
        else if ( position == "Left" )
            pos = DockLeft;
        else if ( position == "Right" )
            pos = DockRight;
        else if ( position == "Floating" )
            pos = DockTornOff;
        else if ( position == "Flat" )
            pos = DockMinimized;

        //kdDebug(220) << name() << " applySettings hidden=" << hidden << endl;
        if (hidden)
            hide();
        else
            show();

        if ( mainWindow() )
        {
            //kdDebug(220) << name() << " applySettings updating ToolbarInfo" << endl;
            d->toolBarInfo = KToolBarPrivate::ToolBarInfo( pos, index, newLine, offset );
            positionYourself( true );
        }
        if (isVisible ())
            updateGeometry();
    }
}

bool KToolBar::event( QEvent *e )
{
    if ( (e->type() == QEvent::LayoutHint) && isUpdatesEnabled() )
       d->repaintTimer.start( 100, true );

    if (e->type() == QEvent::ChildInserted )
    {
       // Bypass QToolBar::event,
       // it will show() the inserted child and we don't want to
       // do that until we have rebuilt the layout.
       childEvent((QChildEvent *)e);
       return true;
    }

    return QToolBar::event( e );
}

void KToolBar::slotRepaint()
{
    setUpdatesEnabled( false );
    // Send a resizeEvent to update the "toolbar extension arrow"
    // (The button you get when your toolbar-items don't fit in
    // the available space)
    QResizeEvent ev(size(), size());
    resizeEvent(&ev);
    QApplication::sendPostedEvents( this, QEvent::LayoutHint );
    setUpdatesEnabled( true );
    repaint( true );
}

void KToolBar::toolBarPosChanged( QToolBar *tb )
{
    if ( tb != this )
        return;
    if ( d->oldPos == DockMinimized )
        rebuildLayout();
    d->oldPos = (QMainWindow::ToolBarDock)barPos();
    KMainWindow *kmw = dynamic_cast<KMainWindow *>(mainWindow());
    if ( kmw )
        kmw->setSettingsDirty();
}

static KToolBar::Dock stringToDock( const QString& attrPosition )
{
    KToolBar::Dock dock = KToolBar::DockTop;
    if ( !attrPosition.isEmpty() ) {
        if ( attrPosition == "top" )
            dock = KToolBar::DockTop;
        else if ( attrPosition == "left" )
            dock = KToolBar::DockLeft;
        else if ( attrPosition == "right" )
            dock = KToolBar::DockRight;
        else if ( attrPosition == "bottom" )
            dock = KToolBar::DockBottom;
        else if ( attrPosition == "floating" )
            dock = KToolBar::DockTornOff;
        else if ( attrPosition == "flat" )
            dock = KToolBar::DockMinimized;
    }
    return dock;
}


void KToolBar::loadState( const QDomElement &element )
{
    QMainWindow *mw = mainWindow();

    if ( !mw )
        return;

    {
        QCString text = element.namedItem( "text" ).toElement().text().utf8();
        if ( text.isEmpty() )
            text = element.namedItem( "Text" ).toElement().text().utf8();
        if ( !text.isEmpty() )
            setText( i18n( text ) );
    }

    {
        QCString attrFullWidth = element.attribute( "fullWidth" ).lower().latin1();
        if ( !attrFullWidth.isEmpty() )
            setFullSize( attrFullWidth == "true" );
    }

    /*
      This method is called in order to load toolbar settings from XML.
      However this can be used in two rather different cases:
      - for the initial loading of the app's XML. In that case the settings
        are only the defaults, the user's KConfig settings will override them
        (KDE4 TODO: how about saving those user settings into the local XML file instead?
        Then this whole thing would be simpler, no KConfig settings to apply afterwards.
        OTOH we'd have to migrate those settings when the .rc version increases,
        like we do for shortcuts)

      - for later re-loading when switching between parts in KXMLGUIFactory.
        In that case the XML contains the final settings, not the defaults.
        We do need the defaults, and the toolbar might have been completely
        deleted and recreated meanwhile. So we store the app-default settings
        into the XML.
     */
    bool loadingAppDefaults = true;
    if ( element.hasAttribute( "offsetDefault" ) )
    {
        // this isn't the first time, so the defaults have been saved into the (in-memory) XML
        loadingAppDefaults = false;
        d->OffsetDefault = element.attribute( "offsetDefault" ).toInt();
        d->NewLineDefault = element.attribute( "newlineDefault" ) == "true";
        d->HiddenDefault = element.attribute( "hiddenDefault" ) == "true";
        d->IconSizeDefault = element.attribute( "iconSizeDefault" ).toInt();
        d->PositionDefault = element.attribute( "positionDefault" );
        d->IconTextDefault = element.attribute( "iconTextDefault" );
    }
    //kdDebug(220) << name() << " loadState loadingAppDefaults=" << loadingAppDefaults << endl;

    Dock dock = stringToDock( element.attribute( "position" ).lower() );

    {
        QCString attrIconText = element.attribute( "iconText" ).lower().latin1();
        if ( !attrIconText.isEmpty() ) {
            //kdDebug(220) << name() << " loadState attrIconText=" << attrIconText << endl;
            if ( attrIconText == "icontextright" )
                setIconText( KToolBar::IconTextRight );
            else if ( attrIconText == "textonly" )
                setIconText( KToolBar::TextOnly );
            else if ( attrIconText == "icontextbottom" )
                setIconText( KToolBar::IconTextBottom );
            else if ( attrIconText == "icononly" )
                setIconText( KToolBar::IconOnly );
        } else
	{
	    //kdDebug(220) << name() << " loadState no iconText attribute in XML, using iconTextSetting=" << iconTextSetting() << endl;
            // Use global setting
            if (d->m_honorStyle)
                setIconText( iconTextSetting() );
            else
                setIconText( d->IconTextDefault );
	}
    }

    QString attrIconSize = element.attribute( "iconSize" ).lower();
    int iconSize = d->IconSizeDefault;
    if ( !attrIconSize.isEmpty() )
        iconSize = attrIconSize.toInt();
    setIconSize( iconSize );

    int index = -1; // append by default. This is very important, otherwise
    // with all 0 indexes, we keep reversing the toolbars.
    {
        QString attrIndex = element.attribute( "index" ).lower();
        if ( !attrIndex.isEmpty() )
            index = attrIndex.toInt();
    }

    int offset = d->OffsetDefault;
    bool newLine = d->NewLineDefault;
    bool hidden = d->HiddenDefault;

    {
        QString attrOffset = element.attribute( "offset" );
        if ( !attrOffset.isEmpty() )
            offset = attrOffset.toInt();
    }

    {
        QString attrNewLine = element.attribute( "newline" ).lower();
        if ( !attrNewLine.isEmpty() )
            newLine = attrNewLine == "true";
    }

    {
        QString attrHidden = element.attribute( "hidden" ).lower();
        if ( !attrHidden.isEmpty() ) {
            hidden = attrHidden  == "true";
        }
    }

    d->toolBarInfo = KToolBarPrivate::ToolBarInfo( dock, index, newLine, offset );
    mw->addDockWindow( this, dock, newLine );
    mw->moveDockWindow( this, dock, newLine, index, offset );

    // Apply the highlight button setting
    d->m_highlight = highlightSetting();

    if ( hidden )
        hide();
    else
        show();

    if ( loadingAppDefaults )
    {
        getAttributes( d->PositionDefault, d->IconTextDefault, index );
        //kdDebug(220) << name() << " loadState IconTextDefault=" << d->IconTextDefault << endl;
        d->OffsetDefault = offset;
        d->NewLineDefault = newLine;
        d->HiddenDefault = hidden;
        d->IconSizeDefault = iconSize;
    }
    //kdDebug(220) << name() << " loadState hidden=" << hidden << endl;

    // Apply transparent-toolbar-moving setting (ok, this is global to the mainwindow,
    // but we do it only if there are toolbars...)
    // KDE4: move to KMainWindow
    if ( transparentSetting() != !mw->opaqueMoving() )
        mw->setOpaqueMoving( !transparentSetting() );
}

int KToolBar::dockWindowIndex()
{
    int index = 0;
    Q_ASSERT( mainWindow() );
    if ( mainWindow() ) {
        QMainWindow::ToolBarDock dock;
        bool newLine;
        int offset;
        mainWindow()->getLocation( this, dock, index, newLine, offset );
    }
    return index;
}

void KToolBar::getAttributes( QString &position, QString &icontext, int &index )
{
    // get all of the stuff to save
    switch ( barPos() ) {
    case KToolBar::Flat:
        position = "Flat";
        break;
    case KToolBar::Bottom:
        position = "Bottom";
        break;
    case KToolBar::Left:
        position = "Left";
        break;
    case KToolBar::Right:
        position = "Right";
        break;
    case KToolBar::Floating:
        position = "Floating";
        break;
    case KToolBar::Top:
    default:
        position = "Top";
        break;
    }

    index = dockWindowIndex();

    switch (d->m_iconText) {
    case KToolBar::IconTextRight:
        icontext = "IconTextRight";
        break;
    case KToolBar::IconTextBottom:
        icontext = "IconTextBottom";
        break;
    case KToolBar::TextOnly:
        icontext = "TextOnly";
        break;
    case KToolBar::IconOnly:
    default:
        icontext = "IconOnly";
        break;
    }
    //kdDebug(220) << name() << " getAttributes: icontext=" << icontext << endl;
}

void KToolBar::saveState( QDomElement &current )
{
    Q_ASSERT( !current.isNull() );
    QString position, icontext;
    int index = -1;
    getAttributes( position, icontext, index );

    current.setAttribute( "noMerge", "1" );
    current.setAttribute( "position", position );
    current.setAttribute( "iconText", icontext );
    current.setAttribute( "index", index );
    current.setAttribute( "offset", offset() );
    current.setAttribute( "newline", newLine() );
    if ( isHidden() )
        current.setAttribute( "hidden", "true" );
    d->modified = true;

    // TODO if this method is used by more than KXMLGUIBuilder, e.g. to save XML settings to *disk*,
    // then the stuff below shouldn't always be done.
    current.setAttribute( "offsetDefault", d->OffsetDefault );
    current.setAttribute( "newlineDefault", d->NewLineDefault );
    current.setAttribute( "hiddenDefault", d->HiddenDefault ? "true" : "false" );
    current.setAttribute( "iconSizeDefault", d->IconSizeDefault );
    current.setAttribute( "positionDefault", d->PositionDefault );
    current.setAttribute( "iconTextDefault", d->IconTextDefault );

    //kdDebug(220) << name() << " saveState: saving index=" << index << " iconText=" << icontext << " hidden=" << isHidden() << endl;
}

// Called by KMainWindow::finalizeGUI
void KToolBar::positionYourself( bool force )
{
    if (force)
        d->positioned = false;

    if ( d->positioned || !mainWindow() )
    {
        //kdDebug(220) << name() << " positionYourself d->positioned=true  ALREADY DONE" << endl;
        return;
    }
    // we can't test for ForceHide after moveDockWindow because QDockArea
    // does a reparent() with showIt == true
    bool hidden = isHidden();
    //kdDebug(220) << name() << " positionYourself  dock=" << d->toolBarInfo.dock << " newLine=" << d->toolBarInfo.newline << " index=" << d->toolBarInfo.index << " offset=" << d->toolBarInfo.offset << endl;
    mainWindow()->moveDockWindow( this, d->toolBarInfo.dock,
                                  d->toolBarInfo.newline,
                                  d->toolBarInfo.index,
                                  d->toolBarInfo.offset );

    //kdDebug(220) << name() << " positionYourself dockWindowIndex=" << dockWindowIndex() << endl;
    if ( hidden )
        hide();
    else
        show();
    // This method can only have an effect once - unless force is set
    d->positioned = true;
}

KPopupMenu *KToolBar::contextMenu()
{
  if ( context )
    return context;
  // Construct our context popup menu. Name it qt_dockwidget_internal so it
  // won't be deleted by QToolBar::clear().
  context = new KPopupMenu( this, "qt_dockwidget_internal" );
  context->insertTitle(i18n("Toolbar Menu"));

  KPopupMenu *orient = new KPopupMenu( context, "orient" );
  orient->insertItem( i18n("toolbar position string","Top"),  CONTEXT_TOP );
  orient->insertItem( i18n("toolbar position string","Left"), CONTEXT_LEFT );
  orient->insertItem( i18n("toolbar position string","Right"), CONTEXT_RIGHT );
  orient->insertItem( i18n("toolbar position string","Bottom"), CONTEXT_BOTTOM );
  orient->insertSeparator(-1);
  orient->insertItem( i18n("toolbar position string","Floating"), CONTEXT_FLOAT );
  orient->insertItem( i18n("min toolbar", "Flat"), CONTEXT_FLAT );

  KPopupMenu *mode = new KPopupMenu( context, "mode" );
  mode->insertItem( i18n("Icons Only"), CONTEXT_ICONS );
  mode->insertItem( i18n("Text Only"), CONTEXT_TEXT );
  mode->insertItem( i18n("Text Alongside Icons"), CONTEXT_TEXTRIGHT );
  mode->insertItem( i18n("Text Under Icons"), CONTEXT_TEXTUNDER );

  KPopupMenu *size = new KPopupMenu( context, "size" );
  size->insertItem( i18n("Default"), CONTEXT_ICONSIZES );
  // Query the current theme for available sizes
  KIconTheme *theme = KGlobal::instance()->iconLoader()->theme();
  QValueList<int> avSizes;
  if (theme)
  {
      if (!::qstrcmp(QObject::name(), "mainToolBar"))
          avSizes = theme->querySizes( KIcon::MainToolbar);
      else
          avSizes = theme->querySizes( KIcon::Toolbar);
  }

  d->iconSizes = avSizes;
  qHeapSort(avSizes);

  QValueList<int>::Iterator it;
  if (avSizes.count() < 10) {
      // Fixed or threshold type icons
	  QValueList<int>::Iterator end(avSizes.end());
      for (it=avSizes.begin(); it!=end; ++it) {
          QString text;
          if ( *it < 19 )
              text = i18n("Small (%1x%2)").arg(*it).arg(*it);
          else if (*it < 25)
              text = i18n("Medium (%1x%2)").arg(*it).arg(*it);
          else if (*it < 35)
              text = i18n("Large (%1x%2)").arg(*it).arg(*it);
          else
              text = i18n("Huge (%1x%2)").arg(*it).arg(*it);
          //we use the size as an id, with an offset
          size->insertItem( text, CONTEXT_ICONSIZES + *it );
      }
  }
  else {
      // Scalable icons.
      const int progression[] = {16, 22, 32, 48, 64, 96, 128, 192, 256};

      it = avSizes.begin();
      for (uint i = 0; i < 9; i++) {
          while (it++ != avSizes.end()) {
              if (*it >= progression[i]) {
                  QString text;
                  if ( *it < 19 )
                      text = i18n("Small (%1x%2)").arg(*it).arg(*it);
                  else if (*it < 25)
                      text = i18n("Medium (%1x%2)").arg(*it).arg(*it);
                  else if (*it < 35)
                      text = i18n("Large (%1x%2)").arg(*it).arg(*it);
                  else
                      text = i18n("Huge (%1x%2)").arg(*it).arg(*it);
                  //we use the size as an id, with an offset
                  size->insertItem( text, CONTEXT_ICONSIZES + *it );
                  break;
              }
          }
      }
  }

  context->insertItem( i18n("Orientation"), orient );
  orient->setItemChecked(CONTEXT_TOP, true);
  context->insertItem( i18n("Text Position"), mode );
  context->setItemChecked(CONTEXT_ICONS, true);
  context->insertItem( i18n("Icon Size"), size );

  connect( context, SIGNAL( aboutToShow() ), this, SLOT( slotContextAboutToShow() ) );
  // Unplugging a submenu from abouttohide leads to the popupmenu floating around
  // So better simply call that code from after exec() returns (DF)
  //connect( context, SIGNAL( aboutToHide() ), this, SLOT( slotContextAboutToHide() ) );
  return context;
}

void KToolBar::slotContextAboutToShow()
{
  // The idea here is to reuse the "static" part of the menu to save time.
  // But the "Toolbars" action is dynamic (can be a single action or a submenu)
  // and ToolBarHandler::setupActions() deletes it, so better not keep it around.
  // So we currently plug/unplug the last two actions of the menu.
  // Another way would be to keep around the actions and plug them all into a (new each time) popupmenu.
  KMainWindow *kmw = dynamic_cast<KMainWindow *>(mainWindow());
  if ( kmw ) {
      kmw->setupToolbarMenuActions();
      // Only allow hiding a toolbar if the action is also plugged somewhere else (e.g. menubar)
      KAction *tbAction = kmw->toolBarMenuAction();
      if ( tbAction && tbAction->containerCount() > 0 )
          tbAction->plug(context);
  }

  // try to find "configure toolbars" action
  KAction *configureAction = 0;
  const char* actionName = KStdAction::name(KStdAction::ConfigureToolbars);
  if ( d->m_xmlguiClient )
    configureAction = d->m_xmlguiClient->actionCollection()->action(actionName);
  if ( !configureAction && kmw )
    configureAction = kmw->actionCollection()->action(actionName);
  if ( configureAction )
    configureAction->plug(context);
  KEditToolbar::setDefaultToolbar(QObject::name());

  for(int i = CONTEXT_ICONS; i <= CONTEXT_TEXTUNDER; ++i)
    context->setItemChecked(i, false);

  switch( d->m_iconText )
  {
        case IconOnly:
        default:
            context->setItemChecked(CONTEXT_ICONS, true);
            break;
        case IconTextRight:
            context->setItemChecked(CONTEXT_TEXTRIGHT, true);
            break;
        case TextOnly:
            context->setItemChecked(CONTEXT_TEXT, true);
            break;
        case IconTextBottom:
            context->setItemChecked(CONTEXT_TEXTUNDER, true);
            break;
  }

  QValueList<int>::ConstIterator iIt = d->iconSizes.begin();
  QValueList<int>::ConstIterator iEnd = d->iconSizes.end();
  for (; iIt != iEnd; ++iIt )
      context->setItemChecked( CONTEXT_ICONSIZES + *iIt, false );

  context->setItemChecked( CONTEXT_ICONSIZES, false );

  context->setItemChecked( CONTEXT_ICONSIZES + d->m_iconSize, true );

  for ( int i = CONTEXT_TOP; i <= CONTEXT_FLAT; ++i )
      context->setItemChecked( i, false );

  switch ( barPos() )
  {
  case KToolBar::Flat:
      context->setItemChecked( CONTEXT_FLAT, true );
      break;
  case KToolBar::Bottom:
      context->setItemChecked( CONTEXT_BOTTOM, true );
      break;
  case KToolBar::Left:
      context->setItemChecked( CONTEXT_LEFT, true );
      break;
  case KToolBar::Right:
      context->setItemChecked( CONTEXT_RIGHT, true );
      break;
  case KToolBar::Floating:
      context->setItemChecked( CONTEXT_FLOAT, true );
      break;
  case KToolBar::Top:
      context->setItemChecked( CONTEXT_TOP, true );
      break;
  default: break;
  }
}

void KToolBar::slotContextAboutToHide()
{
  // We have to unplug whatever slotContextAboutToShow plugged into the menu.
  // Unplug the toolbar menu action
  KMainWindow *kmw = dynamic_cast<KMainWindow *>(mainWindow());
  if ( kmw && kmw->toolBarMenuAction() )
    if ( kmw->toolBarMenuAction()->containerCount() > 1 )
      kmw->toolBarMenuAction()->unplug(context);

  // Unplug the configure toolbars action too, since it's afterwards anyway
  KAction *configureAction = 0;
  const char* actionName = KStdAction::name(KStdAction::ConfigureToolbars);
  if ( d->m_xmlguiClient )
    configureAction = d->m_xmlguiClient->actionCollection()->action(actionName);
  if ( !configureAction && kmw )
    configureAction = kmw->actionCollection()->action(actionName);
  if ( configureAction )
    configureAction->unplug(context);

  QPtrListIterator<QWidget> it( widgets );
  QWidget *wdg;
  while ( ( wdg = it.current() ) != 0 ) {
    if ( wdg->inherits( "QToolButton" ) )
      static_cast<QToolButton*>( wdg )->setDown( false );
    ++it;
  }
}

void KToolBar::widgetDestroyed()
{
    removeWidgetInternal( (QWidget*)sender() );
}

void KToolBar::removeWidgetInternal( QWidget * w )
{
    widgets.removeRef( w );
    QMap< QWidget*, int >::Iterator it = widget2id.find( w );
    if ( it == widget2id.end() )
        return;
    id2widget.remove( *it );
    widget2id.remove( it );
}

void KToolBar::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ktoolbar.moc"

