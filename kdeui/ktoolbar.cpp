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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifdef KDE_USE_FINAL
#undef Always
#include <qdockwindow.h>
#endif
#include "ktoolbar.h"
#include "kmainwindow.h"

#include <string.h>

#include <qpainter.h>
#include <qtooltip.h>
#include <qdrawutil.h>
#include <qstring.h>
#include <qrect.h>
#include <qobjectlist.h>
#include <qtimer.h>
#include <qstyle.h>

#include <config.h>

#include "klineedit.h"
#include "kseparator.h"
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
#include <kipc.h>
#include <kwin.h>
#include <kdebug.h>
#include <qlayout.h>

#include "ktoolbarbutton.h"


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
        m_configurePlugged = false;

        oldPos = QMainWindow::DockUnmanaged;

        modified = m_isHorizontal = positioned = FALSE;
    }

    int m_iconSize;
    KToolBar::IconText m_iconText;
    bool m_highlight : 1;
    bool m_transparent : 1;
    bool m_honorStyle : 1;
    bool m_isHorizontal : 1;
    bool m_enableContext : 1;
    bool m_configurePlugged : 1;
    bool modified : 1;
    bool positioned : 1;

    QWidget *m_parent;

    QMainWindow::ToolBarDock oldPos;

    KXMLGUIClient *m_xmlguiClient;

    struct ToolBarInfo
    {
        ToolBarInfo() : index( 0 ), offset( -1 ), newline( FALSE ), dock( QMainWindow::DockTop ) {}
        ToolBarInfo( QMainWindow::ToolBarDock d,
                     int i, bool n, int o ) : index( i ), offset( o ), newline( n ), dock( d ) {
        }
        int index, offset;
        bool newline;
        QMainWindow::ToolBarDock dock;
    };

    ToolBarInfo toolBarInfo;
    QValueList<int> iconSizes;
    QTimer repaintTimer;
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
    if ( line ) {
        if ( orientation() == Vertical )
            setFrameStyle( HLine + Sunken );
        else
            setFrameStyle( VLine + Sunken );
    } else {
            setFrameStyle( NoFrame );
    }
}

void KToolBarSeparator::styleChange( QStyle& )
{
    setOrientation( orient );
}

QSize KToolBarSeparator::sizeHint() const
{
    return orientation() == Vertical ? QSize( 0, 6 ) : QSize( 6, 0 );
}

QSizePolicy KToolBarSeparator::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
}

KToolBar::KToolBar( QWidget *parent, const char *name, bool honorStyle, bool readConfig )
    : QToolBar( QString::fromLatin1( name ),
      parent && parent->inherits( "QMainWindow" ) ? (QMainWindow*)parent : 0,
      parent, FALSE,
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
    delete d;
}

void KToolBar::init( bool readConfig, bool honorStyle )
{
    d = new KToolBarPrivate;
    setFullSize( TRUE );
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
    // make the combo shrinkable even if the contents are longer than the
    // combo width
    combo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
				       QSizePolicy::Fixed ));

    insertWidgetInternal( combo, index, id );
    combo->insertStringList (list);
    combo->setInsertionPolicy(policy);
    combo->setEnabled( enabled );
    if ( !tooltiptext.isEmpty() )
        QToolTip::add( combo, tooltiptext );
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
    // make the combo shrinkable even if the contents are longer than the
    // combo width
    combo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
				       QSizePolicy::Fixed ));

    insertWidgetInternal( combo, index, id );
    combo->insertItem (text);
    combo->setInsertionPolicy(policy);
    combo->setEnabled( enabled );
    if ( !tooltiptext.isEmpty() )
        QToolTip::add( combo, tooltiptext );
    if ( size > 0 )
        combo->setMinimumWidth( size );
    if (!tooltiptext.isNull())
        QToolTip::add( combo, tooltiptext );
    connect (combo, signal, receiver, slot);
    return index;
}

int KToolBar::insertSeparator(int index, int id)
{
    QWidget *w = new KToolBarSeparator( orientation(), FALSE, this, "tool bar separator" );
    insertWidgetInternal( w, index, id );
    return index;
}

int KToolBar::insertLineSeparator(int index, int id)
{
    QWidget *w = new KToolBarSeparator( orientation(), TRUE, this, "tool bar separator" );
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
    if ( (*it) && (*it)->inherits( "KAnimWidget" ) )
        return (KAnimWidget*)(*it);
    QObjectList *l = queryList( "KAnimWidget" );
    if ( !l || !l->first() ) {
        delete l;
        return 0;
    }

    for ( QObject *o = l->first(); o; o = l->next() ) {
        if ( o->inherits( "KAnimWidget" ) )
        {
            delete l;
            return (KAnimWidget*)o;
        }
    }

    delete l;
    return 0;
}


void KToolBar::addConnection (int id, const char *signal,
                               const QObject *receiver, const char *slot)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    if ( (*it) )
        connect( (*it), signal, receiver, slot );
}

void KToolBar::setItemEnabled( int id, bool enabled )
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    if ( (*it) )
        (*it)->setEnabled( enabled );
}


void KToolBar::setButtonPixmap( int id, const QPixmap& _pixmap )
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    KToolBarButton * button = dynamic_cast<KToolBarButton *>( *it );
    if ( button )
        button->setPixmap( _pixmap );
}


void KToolBar::setButtonIcon( int id, const QString& _icon )
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    KToolBarButton * button = dynamic_cast<KToolBarButton *>( *it );
    if ( button )
        button->setIcon( _icon );
}

void KToolBar::setButtonIconSet( int id, const QIconSet& iconset )
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    KToolBarButton * button = dynamic_cast<KToolBarButton *>( *it );
    if ( button )
        button->setIconSet( iconset );
}


void KToolBar::setDelayedPopup (int id , QPopupMenu *_popup, bool toggle )
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    KToolBarButton * button = dynamic_cast<KToolBarButton *>( *it );
    if ( button )
        button->setDelayedPopup( _popup, toggle );
}


void KToolBar::setAutoRepeat (int id, bool flag)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    KToolBarButton * button = dynamic_cast<KToolBarButton *>( *it );
    if ( button )
        button->setAutoRepeat( flag );
}


void KToolBar::setToggle (int id, bool flag )
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    KToolBarButton * button = dynamic_cast<KToolBarButton *>( *it );
    if ( button )
        button->setToggle( flag );
}


void KToolBar::toggleButton (int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    KToolBarButton * button = dynamic_cast<KToolBarButton *>( *it );
    if ( button )
        button->toggle();
}


void KToolBar::setButton (int id, bool flag)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    KToolBarButton * button = dynamic_cast<KToolBarButton *>( *it );
    if ( button )
        button->on( flag );
}


bool KToolBar::isButtonOn (int id) const
{
    Id2WidgetMap::ConstIterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return false;
    KToolBarButton * button = dynamic_cast<KToolBarButton *>( *it );
    return button ? button->isOn() : false;
}


void KToolBar::setLinedText (int id, const QString& text)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    QLineEdit * lineEdit = dynamic_cast<QLineEdit *>( *it );
    if ( lineEdit )
        lineEdit->setText( text );
}


QString KToolBar::getLinedText (int id) const
{
    Id2WidgetMap::ConstIterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return QString::null;
    QLineEdit * lineEdit = dynamic_cast<QLineEdit *>( *it );
    return lineEdit ? lineEdit->text() : QString::null;
}


void KToolBar::insertComboItem (int id, const QString& text, int index)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    QComboBox * comboBox = dynamic_cast<QComboBox *>( *it );
    if (comboBox)
        comboBox->insertItem( text, index );
}

void KToolBar::insertComboList (int id, const QStringList &list, int index)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    QComboBox * comboBox = dynamic_cast<QComboBox *>( *it );
    if (comboBox)
        comboBox->insertStringList( list, index );
}


void KToolBar::removeComboItem (int id, int index)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    QComboBox * comboBox = dynamic_cast<QComboBox *>( *it );
    if (comboBox)
        comboBox->removeItem( index );
}


void KToolBar::setCurrentComboItem (int id, int index)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    QComboBox * comboBox = dynamic_cast<QComboBox *>( *it );
    if (comboBox)
        comboBox->setCurrentItem( index );
}


void KToolBar::changeComboItem  (int id, const QString& text, int index)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    QComboBox * comboBox = dynamic_cast<QComboBox *>( *it );
    if (comboBox)
        comboBox->changeItem( text, index );
}


void KToolBar::clearCombo (int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    QComboBox * comboBox = dynamic_cast<QComboBox *>( *it );
    if (comboBox)
        comboBox->clear();
}


QString KToolBar::getComboItem (int id, int index) const
{
    Id2WidgetMap::ConstIterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return QString::null;
    QComboBox * comboBox = dynamic_cast<QComboBox *>( *it );
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
    QToolBar::clear();
    widget2id.clear();
    id2widget.clear();
}


void KToolBar::removeItem (int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
    {
        kdDebug(220) << "KToolBar::removeItem item " << id << " not found" << endl;
        return;
    }
    QWidget * w = (*it);
    id2widget.remove( id );
    widget2id.remove( w );
    widgets.removeRef( w );
    delete w;
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
    mainWindow()->moveToolBar( this, (QMainWindow::ToolBarDock)bpos );
}


KToolBar::BarPosition KToolBar::barPos() const
{
    if ( !this->mainWindow() )
        return KToolBar::Top;
    QMainWindow::ToolBarDock dock;
    int dm1, dm2;
    bool dm3;
    this->mainWindow()->getLocation( (QToolBar*)this, dock, dm1, dm3, dm2 );
    if ( dock == QMainWindow::DockUnmanaged ) {
        return (KToolBar::BarPosition)QMainWindow::DockTop;
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
    }

    if (update == false)
        return;

    if (doUpdate)
        emit modechange(); // tell buttons what happened

    // ugly hack to force a QMainWindow::triggerLayout( TRUE )
    if ( mainWindow() ) {
        QMainWindow *mw = mainWindow();
        mw->setUpdatesEnabled( FALSE );
        mw->setToolBarsMovable( !mw->toolBarsMovable() );
        mw->setToolBarsMovable( !mw->toolBarsMovable() );
        mw->setUpdatesEnabled( TRUE );
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

    if (update == false)
        return;

    if (doUpdate)
        emit modechange(); // tell buttons what happened

    // ugly hack to force a QMainWindow::triggerLayout( TRUE )
    if ( mainWindow() ) {
        QMainWindow *mw = mainWindow();
        mw->setUpdatesEnabled( FALSE );
        mw->setToolBarsMovable( !mw->toolBarsMovable() );
        mw->setToolBarsMovable( !mw->toolBarsMovable() );
        mw->setUpdatesEnabled( TRUE );
    }
}


int KToolBar::iconSize() const
{
    if ( !d->m_iconSize ) // default value?
    {
         if (!::qstrcmp(QObject::name(), "mainToolBar"))
             return KGlobal::iconLoader()->currentSize(KIcon::MainToolbar);
         else
             return KGlobal::iconLoader()->currentSize(KIcon::Toolbar);
    }
    return d->m_iconSize;
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
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    KToolBarButton * button = dynamic_cast<KToolBarButton *>( *it );
    if (button)
        button->setNoStyle( no_style );
}


void KToolBar::setFlat (bool flag)
{
    if ( !mainWindow() )
        return;
    if ( flag )
        mainWindow()->moveToolBar( this, QMainWindow::DockMinimized );
    else
        mainWindow()->moveToolBar( this, QMainWindow::DockTop );
    // And remember to save the new look later
    if ( mainWindow()->inherits( "KMainWindow" ) )
        static_cast<KMainWindow *>(mainWindow())->setSettingsDirty();
}


int KToolBar::count() const
{
    return id2widget.count();
}


void KToolBar::saveState()
{
    QString position, icontext, index, offset, newLine;
    getAttributes( position, icontext, index, offset, newLine );

    // first, try to save to the xml file
    if ( d->m_xmlguiClient && !d->m_xmlguiClient->xmlFile().isEmpty() ) {
        // go down one level to get to the right tags
        QDomElement elem = d->m_xmlguiClient->domDocument().documentElement().toElement();
        elem = elem.firstChild().toElement();
        QString barname(!::qstrcmp(name(), "unnamed") ? "mainToolBar" : name());
        QDomElement current;
        // now try to find our toolbar
        d->modified = false;
        for( ; !elem.isNull(); elem = elem.nextSibling().toElement() ) {
            current = elem;

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
        elem = local.documentElement().toElement();
        KXMLGUIFactory::removeDOMComments( elem );
        elem = elem.firstChild().toElement();
        for( ; !elem.isNull(); elem = elem.nextSibling().toElement() ) {
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
    //kdDebug(220) << "KToolBar::saveSettings group=" << _configGroup << " -> " << configGroup << endl;

    QString position, icontext, index, offset, newLine;
    getAttributes( position, icontext, index, offset, newLine );

    //kdDebug(220) << "KToolBar::saveSettings " << name() << " newLine=" << newLine << endl;

    KConfigGroupSaver saver(config, configGroup);

    config->writeEntry("Position", position);
    config->writeEntry("IconText", icontext);
    config->writeEntry("IconSize", iconSize());
    config->writeEntry("Hidden", isHidden());

    if ( !index.isEmpty() )
        config->writeEntry( "Index", index );
    if ( !offset.isEmpty() )
        config->writeEntry( "Offset", offset );
    if ( !newLine.isEmpty() )
        config->writeEntry( "NewLine", newLine );
}


void KToolBar::setXMLGUIClient( KXMLGUIClient *client )
{
    d->m_xmlguiClient = client;
}

void KToolBar::setText( const QString & txt )
{
    setLabel( txt );
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
            int i = contextMenu()->exec( m->globalPos(), 0 );
            switch ( i ) {
            case -1:
                return; // popup cancelled
            case CONTEXT_LEFT:
                mw->moveToolBar( this, QMainWindow::DockLeft );
                break;
            case CONTEXT_RIGHT:
                mw->moveToolBar( this, QMainWindow::DockRight );
                break;
            case CONTEXT_TOP:
                mw->moveToolBar( this, QMainWindow::DockTop );
                break;
            case CONTEXT_BOTTOM:
                mw->moveToolBar( this, QMainWindow::DockBottom );
                break;
            case CONTEXT_FLOAT:
                break;
            case CONTEXT_FLAT:
                mw->moveToolBar( this, QMainWindow::DockMinimized );
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
            if ( mw->inherits("KMainWindow") )
                static_cast<KMainWindow *>(mw)->setSettingsDirty();
        }
    }
}


void KToolBar::rebuildLayout()
{
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
        if ( w->inherits( "KToolBarSeparator" ) &&
             !( (KToolBarSeparator*)w )->showLine() ) {
            l->addSpacing( 6 );
            w->hide();
            continue;
        }
        if ( w->inherits( "QPopupMenu" ) )
            continue;
        l->addWidget( w );
        w->show();
    }
    if ( rightAligned ) {
        l->addStretch();
        l->addWidget( rightAligned );
        rightAligned->show();
    }

    if ( fullSize() ) {
        if ( !stretchableWidget && widgets.last() &&
             !widgets.last()->inherits( "QButton" ) && !widgets.last()->inherits( "KAnimWidget" ) )
            setStretchableWidget( widgets.last() );
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
        QWidget * w = (QWidget*)e->child();
        if ( e->type() == QEvent::ChildInserted ) {
            if ( !e->child()->inherits( "QPopupMenu" ) &&
                 ::qstrcmp( "qt_dockwidget_internal", e->child()->name() ) != 0 ) {

                // prevent items that have been explicitly inserted by insert*() from
                // being inserted again
                if ( !widget2id.contains( w ) )
                {
                    int dummy = -1;
                    insertWidgetInternal( w, dummy, -1 );
                }
            }
        } else {
            widgets.removeRef(w);
            //removeWidgetInternal( w );
        }
        if ( isVisibleTo( 0 ) )
            layoutTimer->start( 50, TRUE );
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
        widgets.append( w );
        index = (int)widgets.count();
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
    KToolBar *this_too = (KToolBar *)this;

    this_too->polish();
    
    int margin = ((QWidget *)this)->layout()->margin();
    switch( barPos() )
    {
     case KToolBar::Top:
     case KToolBar::Bottom:
       for ( QWidget *w = this_too->widgets.first(); w; w = this_too->widgets.next() ) 
       {
          if ( w->inherits( "KToolBarSeparator" ) &&
             !( (KToolBarSeparator*)w )->showLine() ) 
          {
             minSize += QSize(6, 0);
          }
          else
          {
             QSize sh = w->sizeHint();
             if (!sh.isValid())
                sh = w->minimumSize();
             minSize = minSize.expandedTo(QSize(0, sh.height()));
             minSize += QSize(sh.width()+1, 0);
          }
       }
       minSize += QSize(QApplication::style().pixelMetric( QStyle::PM_DockWindowHandleExtent ), 0);
       minSize += QSize(margin*2, margin*2);
       break;
       
     case KToolBar::Left:
     case KToolBar::Right:
       for ( QWidget *w = this_too->widgets.first(); w; w = this_too->widgets.next() ) 
       {
          if ( w->inherits( "KToolBarSeparator" ) &&
             !( (KToolBarSeparator*)w )->showLine() ) 
          {
             minSize += QSize(0, 6);
          }
          else
          {
             QSize sh = w->sizeHint();
             if (!sh.isValid())
                sh = w->minimumSize();
             minSize = minSize.expandedTo(QSize(sh.width(), 0));
             minSize += QSize(0, sh.height()+1);
          }
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
    setUpdatesEnabled( FALSE );
    QToolBar::resizeEvent( e );
    if (b)
       d->repaintTimer.start( 100, true );
}

void KToolBar::slotIconChanged(int group)
{
    if ((group != KIcon::Toolbar) && (group != KIcon::MainToolbar))
        return;
    if ((group == KIcon::MainToolbar) != !::qstrcmp(name(), "mainToolBar"))
        return;

    emit modechange();
    if (isVisible())
        updateGeometry();
}

void KToolBar::slotReadConfig()
{
    //kdDebug(220) << "KToolBar::slotReadConfig" << endl;
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
    if ( mainWindow() && mainWindow()->inherits( "KMainWindow" ) )
        static_cast<KMainWindow *>(mainWindow())->setSettingsDirty();
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
    //kdDebug(220) << "KToolBar::applyAppearanceSettings " << configGroup << endl;
    // We have application-specific settings in the XML file,
    // and nothing in the application's config file
    // -> don't apply the global defaults, the XML ones are preferred
    // See applySettings for a full explanation
    if ( d->m_xmlguiClient && !d->m_xmlguiClient->xmlFile().isEmpty() &&
           !config->hasGroup(configGroup) )
        return;

    KConfig *gconfig = KGlobal::config();

    static const QString &attrIconText  = KGlobal::staticQString("IconText");
    static const QString &attrHighlight = KGlobal::staticQString("Highlighting");
    static const QString &attrTrans     = KGlobal::staticQString("TransparentMoving");
    static const QString &attrSize      = KGlobal::staticQString("IconSize");

    // we actually do this in two steps.
    // First, we read in the global styles [Toolbar style] (from the KControl module).
    // Then, if the toolbar is NOT 'mainToolBar', we will also try to read in [barname Toolbar style]
    bool highlight;
    int transparent;
    QString icontext;
    int iconsize = 0;

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
            icontext = gconfig->readEntry(attrIconText, "IconOnly");
        else
            icontext = "IconOnly";

        // Use the default icon size for toolbar icons.
        iconsize = gconfig->readNumEntry(attrSize, 0);

        if ( !forceGlobal && config->hasGroup(configGroup) )
        {
            config->setGroup(configGroup);

            // first, get the generic settings
            highlight   = config->readBoolEntry(attrHighlight, highlight);
            transparent = config->readBoolEntry(attrTrans, transparent);
            // now we always read in the IconText property
            icontext = config->readEntry(attrIconText, icontext);

            // now get the size
            iconsize = config->readNumEntry(attrSize, iconsize);
        }
        // revert back to the old group
    } // end block for KConfigGroupSaver

    bool doUpdate = false;

    IconText icon_text;
    if ( icontext == "IconTextRight" )
        icon_text = IconTextRight;
    else if ( icontext == "IconTextBottom" )
        icon_text = IconTextBottom;
    else if ( icontext == "TextOnly" )
        icon_text = TextOnly;
    else
        icon_text = IconOnly;

    // check if the icon/text has changed
    if (icon_text != d->m_iconText) {
        //kdDebug(220) << "KToolBar::applyAppearanceSettings setIconText" << icontext << endl;
        setIconText(icon_text, false);
        doUpdate = true;
    }

    // ...and check if the icon size has changed
    if (iconsize != d->m_iconSize) {
        setIconSize(iconsize, false);
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
        emit modechange(); // tell buttons what happened
    if (isVisible ())
        updateGeometry();
}

void KToolBar::applySettings(KConfig *config, const QString &_configGroup)
{
    //kdDebug(220) << "KToolBar::applySettings group=" << _configGroup << endl;

    QString configGroup = _configGroup.isEmpty() ? settingsGroup() : _configGroup;

    /*
      Let's explain this a bit more in details.
      The order in which we apply settings is :
       Global config / <appnamerc> user settings                        if no XMLGUI is used
       Global config / App-XML attributes / <appnamerc> user settings   if XMLGUI is used

      So in the first case, we simply read everything from KConfig as below,
      but in the second case we don't do anything here if there is no app-specific config,
      and the XMLGUI uses the static methods of this class to get the global defaults.

      Global config doesn't include position (index, offset, newline and hidden/shown).
    */

    // First the appearance stuff - the one which has a global config
    applyAppearanceSettings( config, _configGroup );

    // ...and now the position stuff
    if ( config->hasGroup(configGroup) )
    {
        KConfigGroupSaver cgs(config, configGroup);

        static const QString &attrPosition  = KGlobal::staticQString("Position");
        static const QString &attrIndex  = KGlobal::staticQString("Index");
        static const QString &attrOffset  = KGlobal::staticQString("Offset");
        static const QString &attrNewLine  = KGlobal::staticQString("NewLine");
        static const QString &attrHidden  = KGlobal::staticQString("Hidden");

        QString position = config->readEntry(attrPosition, "Top");
        int index = config->readNumEntry(attrIndex, 0 );
        int offset = config->readNumEntry(attrOffset, -1 );
        bool newLine = config->readEntry(attrNewLine).lower() == "true"; // someone used "TRUE" we can't use readBoolEntry :(
        bool hidden = config->readBoolEntry(attrHidden, false);

        BarPosition pos(Top);
        if ( position == "Top" )
            pos = Top;
        else if ( position == "Bottom" )
            pos = Bottom;
        else if ( position == "Left" )
            pos = Left;
        else if ( position == "Right" )
            pos = Right;
        else if ( position == "Floating" )
            pos = Floating;
        else if ( position == "Flat" )
            pos = Flat;

        //kdDebug(220) << "KToolBar::applySettings hidden=" << hidden << endl;
        if (hidden)
            hide();
        else
            show();

        if ( mainWindow() )
        {
            QMainWindow *mw = mainWindow();

            //kdDebug(220) << "KToolBar::applySettings updating ToolbarInfo" << endl;
            d->toolBarInfo = KToolBarPrivate::ToolBarInfo( (QMainWindow::ToolBarDock)pos, index, newLine, offset );

            // moveToolBar calls QDockArea which does a reparent() on us with
            // showIt = true, so we loose our visibility status
            bool doHide = isHidden();

            mw->moveToolBar( this, (QMainWindow::ToolBarDock)pos, newLine, index, offset );
            //kdDebug(220) << "KToolBar::applySettings " << name() << " moveToolBar with pos=" << pos << " newLine=" << newLine << " idx=" << index << " offs=" << offset << endl;
            if ( doHide )
                hide();
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
       // By pass QToolBar::event, 
       // it will show() the inserted child and we don't want to 
       // do that until we have rebuild the layout.
       childEvent((QChildEvent *)e);
       return true;
    }
        
    return QToolBar::event( e );
}

void KToolBar::slotRepaint()
{
    setUpdatesEnabled( FALSE );
    // Send a resizeEvent to update the "toolbar extension arrow" 
    // (The button you get when your toolbar-items don't fit in 
    // the available space)
    QResizeEvent ev(size(), size());
    resizeEvent(&ev);
    QApplication::sendPostedEvents( this, QEvent::LayoutHint );
    setUpdatesEnabled( TRUE );
    repaint( FALSE );
}

void KToolBar::toolBarPosChanged( QToolBar *tb )
{
    if ( tb != this )
        return;
    if ( d->oldPos == QMainWindow::DockMinimized )
        rebuildLayout();
    d->oldPos = (QMainWindow::ToolBarDock)barPos();
    if ( mainWindow() && mainWindow()->inherits( "KMainWindow" ) )
        static_cast<KMainWindow *>(mainWindow())->setSettingsDirty();
}

void KToolBar::loadState( const QDomElement &element )
{
    //kdDebug(220) << "KToolBar::loadState " << this << endl;
    if ( !mainWindow() || !mainWindow()->inherits( "KMainWindow") )
        return;
    KMainWindow *mw = static_cast<KMainWindow *>( mainWindow() );

    QCString text = element.namedItem( "text" ).toElement().text().utf8();
    if ( text.isEmpty() )
        text = element.namedItem( "Text" ).toElement().text().utf8();

    if ( !text.isEmpty() )
        setText( i18n( text ) );

    mw->addToolBar( this );
    QCString attrFullWidth = element.attribute( "fullWidth" ).lower().latin1();
    QCString attrPosition = element.attribute( "position" ).lower().latin1();
    QCString attrIconText = element.attribute( "iconText" ).lower().latin1();
    QString attrIconSize = element.attribute( "iconSize" ).lower();
    QString attrIndex = element.attribute( "index" ).lower();
    QString attrOffset = element.attribute( "offset" ).lower();
    QString attrNewLine = element.attribute( "newline" ).lower();
    QString attrHidden = element.attribute( "hidden" ).lower();

    if ( !attrFullWidth.isEmpty() ) {
        if ( attrFullWidth == "true" )
            setFullSize( TRUE );
        else
            setFullSize( FALSE );
    }

    QMainWindow::ToolBarDock dock = QMainWindow::DockTop;
    int index = -1 /*append by default*/, offset = -1;
    bool nl = FALSE;

    //kdDebug(220) << "KToolBar::loadState attrPosition=" << attrPosition << endl;
    if ( !attrPosition.isEmpty() ) {
        if ( attrPosition == "top" )
            dock = QMainWindow::DockTop;
        else if ( attrPosition == "left" )
            dock = QMainWindow::DockLeft;
        else if ( attrPosition == "right" )
            dock = QMainWindow::DockRight;
        else if ( attrPosition == "bottom" )
            dock = QMainWindow::DockBottom;
        else if ( attrPosition == "floating" )
            dock = QMainWindow::DockTornOff;
        else if ( attrPosition == "flat" )
            dock = QMainWindow::DockMinimized;
    }

    if ( !attrIndex.isEmpty() )
        index = attrIndex.toInt();
    if ( !attrOffset.isEmpty() )
        offset = attrOffset.toInt();
    if ( !attrNewLine.isEmpty() )
        nl = attrNewLine == "true" ? TRUE : FALSE;

    //kdDebug(220) << "KToolBar::loadState creating ToolBarInfo with dock=" << dock << endl;
    d->toolBarInfo = KToolBarPrivate::ToolBarInfo( dock, index, nl, offset );
    if ( mw )
    {
       mw->moveToolBar( this, dock, nl, index, offset );
       //kdDebug(220) << "moveToolBar in loadState for " << name() << " dock=" << dock << " nl=" << nl << " offset=" << offset << endl;
    }

    if ( !attrIconText.isEmpty() ) {
        //kdDebug(220) << "KToolBar::loadState attrIconText=" << attrIconText << endl;
        if ( attrIconText == "icontextright" )
            setIconText( KToolBar::IconTextRight );
        else if ( attrIconText == "textonly" )
            setIconText( KToolBar::TextOnly );
        else if ( attrIconText == "icontextbottom" )
            setIconText( KToolBar::IconTextBottom );
        else if ( attrIconText == "icononly" )
            setIconText( KToolBar::IconOnly );
    } else
        // Use global setting
        setIconText( iconTextSetting() );

    if ( !attrIconSize.isEmpty() )
        setIconSize( attrIconSize.toInt() );

    // Apply the highlight button setting
    d->m_highlight = highlightSetting();

    // Apply transparent-toolbar-moving setting (ok, this is global to the mainwindow,
    // but we do it only if there are toolbars...)
    if ( transparentSetting() != (!mw->opaqueMoving()) )
        mw->setOpaqueMoving( !transparentSetting() );

    if ( attrHidden == "true" )
        hide();
    else
        show();
}

void KToolBar::getAttributes( QString &position, QString &icontext, QString &index, QString &offset, QString &newLine )
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
    if ( mainWindow() ) {
        QMainWindow::ToolBarDock dock;
        int index_;
        bool nl;
        int offset_;
        mainWindow()->getLocation( (QToolBar*)this, dock, index_, nl, offset_ );
        index = QString::number( index_ );
        offset = QString::number( offset_ );
        newLine = nl ? "true" : "false";
    }

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
}

void KToolBar::saveState( QDomElement &current )
{
    QString position, icontext, index, offset, newLine;
    getAttributes( position, icontext, index, offset, newLine );

    current.setAttribute( "noMerge", "1" );
    current.setAttribute( "position", position );
    current.setAttribute( "iconText", icontext );
    if ( !index.isEmpty() )
        current.setAttribute( "index", index );
    if ( !offset.isEmpty() )
        current.setAttribute( "offset", offset );
    if ( !newLine.isEmpty() )
        current.setAttribute( "newline", newLine );
    if ( isHidden() )
        current.setAttribute( "hidden", "true" );
    d->modified = true;
}

void KToolBar::positionYourself( bool force )
{
    if (force)
        d->positioned = false;

    if ( d->positioned || !mainWindow() )
    {
        //kdDebug(220) << "KToolBar::positionYourself d->positioned=true  ALREADY DONE" << endl;
        return;
    }
    // we can't test for ForceHide after moveToolBar because QDockArea
    // does a reparent() with showIt == true
    bool doHide = isHidden();
    //kdDebug(220) << "positionYourself " << name() << " dock=" << d->toolBarInfo.dock << " newLine=" << d->toolBarInfo.newline << " offset=" << d->toolBarInfo.offset << endl;
    mainWindow()->moveToolBar( this, d->toolBarInfo.dock,
                               d->toolBarInfo.newline,
                               d->toolBarInfo.index,
                               d->toolBarInfo.offset );
    if ( doHide )
        hide();
    // This method can only have an effect once - unless force is set
    d->positioned = TRUE;
}

KPopupMenu *KToolBar::contextMenu()
{
  if ( context )
    return context;

  // construct our context popup menu
  context = new KPopupMenu( 0, "context" );
  context->insertTitle(i18n("Toolbar Menu"));

  KPopupMenu *orient = new KPopupMenu( context, "orient" );
  orient->insertItem( i18n("toolbar position string","Top"),  CONTEXT_TOP );
  orient->insertItem( i18n("toolbar position string","Left"), CONTEXT_LEFT );
  orient->insertItem( i18n("toolbar position string","Right"), CONTEXT_RIGHT );
  orient->insertItem( i18n("toolbar position string","Bottom"), CONTEXT_BOTTOM );
  orient->insertSeparator(-1);
  //orient->insertItem( i18n("toolbar position string","Floating"), CONTEXT_FLOAT );
  orient->insertItem( i18n("min toolbar", "Flat"), CONTEXT_FLAT );

  KPopupMenu *mode = new KPopupMenu( context, "mode" );
  mode->insertItem( i18n("Icons Only"), CONTEXT_ICONS );
  mode->insertItem( i18n("Text Only"), CONTEXT_TEXT );
  mode->insertItem( i18n("Text Aside Icons"), CONTEXT_TEXTRIGHT );
  mode->insertItem( i18n("Text Under Icons"), CONTEXT_TEXTUNDER );

  KPopupMenu *size = new KPopupMenu( context, "size" );
  size->insertItem( i18n("Default"), CONTEXT_ICONSIZES );
  // Query the current theme for available sizes
  KIconTheme *theme = KGlobal::instance()->iconLoader()->theme();
  QValueList<int> avSizes;
  if (!::qstrcmp(QObject::name(), "mainToolBar"))
      avSizes = theme->querySizes( KIcon::MainToolbar);
  else
      avSizes = theme->querySizes( KIcon::Toolbar);

  d->iconSizes = avSizes;

  QValueList<int>::Iterator it;
  for (it=avSizes.begin(); it!=avSizes.end(); it++) {
      QString text;
      if ( *it < 19 )
          text = i18n("Small (%1x%2)").arg(*it).arg(*it);
      else if (*it < 25)
          text = i18n("Medium (%1x%2)").arg(*it).arg(*it);
      else
          text = i18n("Large (%1x%2)").arg(*it).arg(*it);
      //we use the size as an id, with an offset
      size->insertItem( text, CONTEXT_ICONSIZES + *it );
  }

  context->insertItem( i18n("Orientation"), orient );
  orient->setItemChecked(CONTEXT_TOP, true);
  context->insertItem( i18n("Text Position"), mode );
  context->setItemChecked(CONTEXT_ICONS, true);
  context->insertItem( i18n("Icon Size"), size );

  if (mainWindow()->inherits("KMainWindow"))
  {
      if ( (static_cast<KMainWindow*>(mainWindow())->toolBarMenuAction()) &&
		(static_cast<KMainWindow*>(mainWindow())->hasMenuBar()) )

      (static_cast<KMainWindow*>(mainWindow()))->toolBarMenuAction()->plug(context);
  }


  connect( context, SIGNAL( aboutToShow() ), this, SLOT( slotContextAboutToShow() ) );
  return context;
}

void KToolBar::slotContextAboutToShow()
{
  if (!d->m_configurePlugged)
  {
    // try to find "configure toolbars" action
    KXMLGUIClient *xmlGuiClient = d->m_xmlguiClient;
    if ( !xmlGuiClient && mainWindow() && mainWindow()->inherits( "KMainWindow" ) )
      xmlGuiClient = (KMainWindow *)mainWindow();
    if ( xmlGuiClient )
    {
        KAction *configureAction = xmlGuiClient->actionCollection()->action(KStdAction::stdName(KStdAction::ConfigureToolbars));
        if ( configureAction )
        {
          configureAction->plug(context);
          d->m_configurePlugged = true;
        }
    }
  }

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

