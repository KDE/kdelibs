/* This file is part of the KDE libraries
    Copyright
    (C) 2000 Reinald Stadlbauer (reggie@kde.org)
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

#define private public
// Reggie: Bad hack but we need to access QToolBar::bl (QBoxLayout*)
// to be able to do some KDE specific toolbar stuff
// In Qt 2.2 this will be "fixed", so that we don't need this very ugly hack anymore.
// Please be careful and don't use any other private QToolBar memebers!!!!
#include <qtoolbar.h>
#undef private

#include <ktmainwindow.h>

#include "ktoolbar.h"

#include <string.h>

#include <qpainter.h>
#include <qtooltip.h>
#include <qdrawutil.h>
#include <qstring.h>
#include <qrect.h>
#include <qobjectlist.h>

#include <config.h>

#include "klineedit.h"
#include "kseparator.h"
#include <klocale.h>
#include <kapp.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <kstyle.h>
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
	hasRealPos = FALSE;
    }

    int m_iconSize;
    KToolBar::IconText m_iconText;
    bool m_highlight;
    bool m_transparent;
    bool m_honorStyle;
    bool m_isHorizontal;

    QWidget *m_parent;

    bool m_enableContext;
    bool hasRealPos;
    QMainWindow::ToolBarDock realPos;
    int realIndex, realOffset;
    bool realNl;

    KXMLGUIClient *m_xmlguiClient;
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

KToolBar::KToolBar( QWidget *parent, const char *name, bool b )
    : QToolBar( QString( name ), parent && parent->inherits( "QMainWindow" ) ? (QMainWindow*)parent : 0, parent, FALSE, name )
{
    init();
    d->m_honorStyle = b;
}

void KToolBar::init()
{
    d = new KToolBarPrivate;
    layoutTimer = new QTimer( this );
    connect( layoutTimer, SIGNAL( timeout() ),
	     this, SLOT( rebuildLayout() ) );
    // construct our context popup menu
    context = new KPopupMenu( 0, "context" );
    context->insertTitle(i18n("Toolbar Menu"));

    QPopupMenu *orient = new QPopupMenu( context, "orient" );
    orient->insertItem( i18n("Top"),  CONTEXT_TOP );
    orient->insertItem( i18n("Left"), CONTEXT_LEFT );
    orient->insertItem( i18n("Right"), CONTEXT_RIGHT );
    orient->insertItem( i18n("Bottom"), CONTEXT_BOTTOM );
    orient->insertSeparator(-1);
    //orient->insertItem( i18n("Floating"), CONTEXT_FLOAT );
    orient->insertItem( i18n("min toolbar", "Flat"), CONTEXT_FLAT );

    QPopupMenu *mode = new QPopupMenu( context, "mode" );
    mode->insertItem( i18n("Icons only"), CONTEXT_ICONS );
    mode->insertItem( i18n("Text only"), CONTEXT_TEXT );
    mode->insertItem( i18n("Text aside icons"), CONTEXT_TEXTRIGHT );
    mode->insertItem( i18n("Text under icons"), CONTEXT_TEXTUNDER );

    QPopupMenu *size = new QPopupMenu( context, "size" );
    size->insertItem( i18n("Default"), CONTEXT_ICONSIZES );
    // Query the current theme for available sizes
    KIconTheme *theme = KGlobal::instance()->iconLoader()->theme();
    QValueList<int> avSizes;
    if (!strcmp(QObject::name(), "mainToolBar"))
	avSizes = theme->querySizes( KIcon::MainToolbar);
    else
	avSizes = theme->querySizes( KIcon::Toolbar);

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

    context->setFont(KGlobalSettings::menuFont());

    context->insertItem( i18n("Orientation"), orient );
    orient->setItemChecked(CONTEXT_TOP, true);
    context->insertItem( i18n("Text position"), mode );
    context->setItemChecked(CONTEXT_ICONS, true);
    context->insertItem( i18n("Icon size"), size );

  connect(kapp, SIGNAL(appearanceChanged()), this, SLOT(slotReadConfig()));
  // request notification of changes in icon style
  kapp->addKipcEventMask(KIPC::IconChanged);
  connect(kapp, SIGNAL(iconChanged(int)), this, SLOT(slotIconChanged(int)));

  // finally, read in our configurable settings
  slotReadConfig();
}


int KToolBar::insertButton(const QString& icon, int id, bool enabled,
			    const QString& text, int index, KInstance *_instance )
{
    KToolBarButton *button = new KToolBarButton( icon, id, this, 0, text, _instance );
    inserted.removeRef( button );
    insertWidgetInternal( button, index, id );
    inserted.append( button );
    button->setEnabled( enabled );
    doConnections( button );
    return *widget2id.find( button );
}


int KToolBar::insertButton(const QString& icon, int id, const char *signal,
			    const QObject *receiver, const char *slot,
			    bool enabled, const QString& text, int index, KInstance *_instance )
{
    KToolBarButton *button = new KToolBarButton( icon, id, this, 0, text, _instance);
    inserted.removeRef( button );
    insertWidgetInternal( button, index, id );
    inserted.append( button );
    button->setEnabled( enabled );
    connect( button, signal, receiver, slot );
    doConnections( button );
    return index;
}


int KToolBar::insertButton(const QPixmap& pixmap, int id, bool enabled,
			    const QString& text, int index )
{
    KToolBarButton *button = new KToolBarButton( pixmap, id, this, 0, text);
    inserted.removeRef( button );
    insertWidgetInternal( button, index, id );
    inserted.append( button );
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
    inserted.removeRef( button );
    insertWidgetInternal( button, index, id );
    inserted.append( button );
    button->setEnabled( enabled );
    connect( button, signal, receiver, slot );
    doConnections( button );
    return index;
}


int KToolBar::insertButton(const QPixmap& pixmap, int id, QPopupMenu *popup,
			    bool enabled, const QString &text, int index )
{
    KToolBarButton *button = new KToolBarButton( pixmap, id, this, 0, text );
    inserted.removeRef( button );
    insertWidgetInternal( button, index, id );
    inserted.append( button );
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
    inserted.removeRef( lined );
    insertWidgetInternal( lined, index, id );
    inserted.append( lined );
    connect( lined, signal, receiver, slot );
    lined->setText(text);
    lined->setEnabled( enabled );
    return index;
}


int KToolBar::insertCombo (QStrList *list, int id, bool writable,
			    const char *signal, const QObject *receiver,
			    const char *slot, bool enabled ,
			    const QString& tooltiptext,
			    int size, int index,
			    QComboBox::Policy policy )
{
    KComboBox *combo = new KComboBox ( writable, this );
    inserted.removeRef( combo );
    insertWidgetInternal( combo, index, id );
    inserted.append( combo );
    combo->insertStrList (list);
    combo->setEnabled( enabled );
    if ( !tooltiptext.isEmpty() )
	QToolTip::add( combo, tooltiptext );
    if ( size > 0 )
	combo->setMinimumWidth( size );
    combo->setInsertionPolicy(policy);
    if (!tooltiptext.isNull())
	QToolTip::add( combo, tooltiptext );
    connect ( combo, signal, receiver, slot );
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
    inserted.removeRef( combo );
    insertWidgetInternal( combo, index, id );
    inserted.append( combo );
    combo->insertStringList (list);
    combo->setInsertionPolicy(policy);
    combo->setEnabled( enabled );
    if ( !tooltiptext.isEmpty() )
	QToolTip::add( combo, tooltiptext );
    if ( size > 0 )
	combo->setMinimumWidth( size );
    if (!tooltiptext.isNull())
	QToolTip::add( combo, tooltiptext );
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
    inserted.removeRef( combo );
    insertWidgetInternal( combo, index, id );
    inserted.append( combo );
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

int KToolBar::insertSeparator(int index)
{
    QWidget *w = new KToolBarSeparator( orientation(), FALSE, this, "tool bar separator" );
    insertWidgetInternal( w, index, -1 );
    inserted.append( w );
    return index;
}


int KToolBar::insertLineSeparator(int index)
{
    QWidget *w = new KToolBarSeparator( orientation(), TRUE, this, "tool bar separator" );
    insertWidgetInternal( w, index, -1 );
    inserted.append( w );
    return index;
}


int KToolBar::insertWidget(int id, int /*width*/, QWidget *widget, int index)
{
    inserted.removeRef( widget );
    insertWidgetInternal( widget, index, id );
    inserted.append( widget );
    return index;
}


int KToolBar::insertAnimatedWidget(int id, QObject *receiver, const char *slot,
				    const QStringList& icons, int index )
{
    KAnimWidget *anim = new KAnimWidget( icons, d->m_iconSize, this );
    inserted.removeRef( anim );
    insertWidgetInternal( anim, index, id );
    inserted.append( anim );

    if ( receiver )
	connect( anim, SIGNAL(clicked()), receiver, slot);

    return *widget2id.find( anim );

}


KAnimWidget *KToolBar::animatedWidget( int id )
{
    if ( id2widget.find( id ) == id2widget.end() )
	return 0;
    QWidget *w = *id2widget.find( id );
    if ( w && w->inherits( "KAnimWidget" ) )
	return (KAnimWidget*)w;
    QObjectList *l = queryList( "KAnimWidget" );
    if ( !l || !l->first() ) {
	delete l;
	return 0;
    }

    for ( QObject *o = l->first(); o; o = l->next() ) {
	delete l;
	return (KAnimWidget*)o;
    }

    return 0;
}


void KToolBar::addConnection (int id, const char *signal,
			       const QObject *receiver, const char *slot)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w )
	return;
    connect( w, signal, receiver, slot );
}

void KToolBar::setItemEnabled( int id, bool enabled )
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w )
	return;
    w->setEnabled( enabled );
}


void KToolBar::setButtonPixmap( int id, const QPixmap& _pixmap )
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "KToolBarButton" ) )
	return;
    ( (KToolBarButton*)w )->setPixmap( _pixmap );
}


void KToolBar::setButtonIcon( int id, const QString& _icon )
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "KToolBarButton" ) )
	return;
    ( (KToolBarButton*)w )->setIcon( _icon );
}


void KToolBar::setDelayedPopup (int id , QPopupMenu *_popup, bool toggle )
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "KToolBarButton" ) )
	return;
    ( (KToolBarButton*)w )->setDelayedPopup( _popup, toggle );
}


void KToolBar::setAutoRepeat (int id, bool flag)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "KToolBarButton" ) )
	return;
    ( (KToolBarButton*)w )->setAutoRepeat( flag );
}


void KToolBar::setToggle (int id, bool flag )
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "KToolBarButton" ) )
	return;
    ( (KToolBarButton*)w )->setToggle( flag );
}


void KToolBar::toggleButton (int id)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "KToolBarButton" ) )
	return;
    ( (KToolBarButton*)w )->toggle();
}


void KToolBar::setButton (int id, bool flag)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "KToolBarButton" ) )
	return;
    ( (KToolBarButton*)w )->on( flag );
}


bool KToolBar::isButtonOn (int id)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return FALSE;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "KToolBarButton" ) )
	return FALSE;
    return ( (KToolBarButton*)w )->isOn();
}


void KToolBar::setLinedText (int id, const QString& text)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "QLineEdit" ) )
	return;
    ( (QLineEdit*)w )->setText( text );
}


QString KToolBar::getLinedText (int id)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return QString::null;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "QLineEdit" ) )
	return QString::null;
    return ( (QLineEdit*)w )->text();
}


void KToolBar::insertComboItem (int id, const QString& text, int index)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "QComboBox" ) )
	return;
    ( (QComboBox*)w )->insertItem( text, index );
}


void KToolBar::insertComboList (int id, QStrList *list, int index)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "QComboBox" ) )
	return;
    ( (QComboBox*)w )->insertStrList( list, index );
}


void KToolBar::insertComboList (int id, const QStringList &list, int index)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "QComboBox" ) )
	return;
    ( (QComboBox*)w )->insertStringList( list, index );
}


void KToolBar::removeComboItem (int id, int index)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "QComboBox" ) )
	return;
    ( (QComboBox*)w )->removeItem( index );
}


void KToolBar::setCurrentComboItem (int id, int index)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "QComboBox" ) )
	return;
    ( (QComboBox*)w )->setCurrentItem( index );
}


void KToolBar::changeComboItem  (int id, const QString& text, int index)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "QComboBox" ) )
	return;
    ( (QComboBox*)w )->changeItem( text, index );
}


void KToolBar::clearCombo (int id)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "QComboBox" ) )
	return;
    ( (QComboBox*)w )->clear();
}


QString KToolBar::getComboItem (int id, int index)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return QString::null;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "QComboBox" ) )
	return QString::null;
    return ( (QComboBox*)w )->text( index );
}


KComboBox * KToolBar::getCombo(int id)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return 0;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "KComboBox" ) )
	return 0;
    return (KComboBox*)w;
}


KLineEdit * KToolBar::getLined (int id)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return 0;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "KLineEdit" ) )
	return 0;
    return (KLineEdit*)w;
}


KToolBarButton * KToolBar::getButton (int id)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return 0;
    QWidget *w = *id2widget.find( id );
    if ( !w || !w->inherits( "KToolBarButton" ) )
	return 0;
    return (KToolBarButton*)w;
}


void KToolBar::alignItemRight (int id, bool right )
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( rightAligned && !right && w == rightAligned )
	rightAligned = 0;
    if ( w && right )
	rightAligned = w;
}


QWidget *KToolBar::getWidget (int id)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return 0;
    return *id2widget.find( id );
}


void KToolBar::setItemAutoSized (int id, bool yes )
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w )
	return;
    if ( yes )
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
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( !w )
	return;
    id2widget.remove( id );
    widget2id.remove( w );
    widgets.removeRef( w );
}


void KToolBar::hideItem (int id)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( w )
	w->hide();
}


void KToolBar::showItem (int id)
{
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
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
    if ( !parentWidget() || !parentWidget()->inherits( "QMainWindow" ) )
	return;
    ( (QMainWindow*)parentWidget() )->setToolBarsMovable( flag );
}


void KToolBar::setBarPos (BarPosition bpos)
{
    if ( !parentWidget() || !parentWidget()->inherits( "QMainWindow" ) )
	return;
    ( (QMainWindow*)parentWidget() )->moveToolBar( this, (QMainWindow::ToolBarDock)bpos );
}


KToolBar::BarPosition KToolBar::barPos() const
{
    if ( !parentWidget() || !parentWidget()->inherits( "QMainWindow" ) )
	return KToolBar::Top;
    QMainWindow::ToolBarDock dock;
    int dm1, dm2;
    bool dm3;
    ( (const QMainWindow*)parentWidget() )->getLocation( (QToolBar*)this, dock, dm1, dm3, dm2 );
    if ( dock == QMainWindow::Unmanaged )
	return (KToolBar::BarPosition)QMainWindow::Top;
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

    if (context) {
	for(int i = CONTEXT_ICONS; i <= CONTEXT_TEXTUNDER; ++i)
	    context->setItemChecked(i, false);
	
	switch (icontext) {
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
    }	

    if (update == false)
	return;

    if (doUpdate)
	emit modechange(); // tell buttons what happened
    updateGeometry();
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
	    // Uncheck old item
	    if (context)
		context->setItemChecked( d->m_iconSize + CONTEXT_ICONSIZES, false );
	    d->m_iconSize = size;
	    doUpdate=true;
    }

    if (context)
	context->setItemChecked( d->m_iconSize + CONTEXT_ICONSIZES, true );

    if (update == false)
	return;

    if (doUpdate)
	emit modechange(); // tell buttons what happened
    updateGeometry();
}


int KToolBar::iconSize() const
{
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
    if ( id2widget.find( id ) == id2widget.end() )
	return;
    QWidget *w = *id2widget.find( id );
    if ( w && w->inherits( "KToolBarButton" ) )
	( (KToolBarButton*)w )->setNoStyle( no_style );
}


void KToolBar::setFlat (bool flag)
{
    if ( !parentWidget() || !parentWidget()->inherits( "QMainWindow" ) )
	return;
    if ( flag )
	( (QMainWindow*)parentWidget() )->moveToolBar( this, QMainWindow::Minimized );
    else
	( (QMainWindow*)parentWidget() )->moveToolBar( this, QMainWindow::Top );
}


int KToolBar::count()
{
    return id2widget.count();
}


void KToolBar::saveState()
{
    // only bother if our state really did change
//     if (!d->m_stateChanged)
// 	return;

  // get all of the stuff to save
    QString position;
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

    QString icontext;
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

    // first, try to save to the xml file
    //  if ( d->m_xmlFile != QString::null )
    if ( d->m_xmlguiClient && !d->m_xmlguiClient->xmlFile().isEmpty() ) {
	QDomElement elem = d->m_xmlguiClient->domDocument().documentElement().toElement();

	    // go down one level to get to the right tags
	elem = elem.firstChild().toElement();

	// get a name we can use for this toolbar
	QString barname(!strcmp(name(), "unnamed") ? "mainToolBar" : name());

	// now try to find our toolbar
	bool modified = false;
	QDomElement current;
	for( ; !elem.isNull(); elem = elem.nextSibling().toElement() ) {
	    current = elem;

	    if ( current.tagName().lower() != "toolbar" )
		continue;

	    QString curname(current.attribute( "name" ));

	    if ( curname == barname ) {
		current.setAttribute( "noMerge", "1" );
		current.setAttribute( "position", position );
		current.setAttribute( "iconText", icontext );
		modified = true;

		break;
	    }
	}

	// if we didn't make changes, then just return
	if ( !modified )
	    return;

	    // now we load in the (non-merged) local file
	QString local_xml(KXMLGUIFactory::readConfigFile(d->m_xmlguiClient->xmlFile(), true, d->m_xmlguiClient->instance()));
	QDomDocument local;
	local.setContent(local_xml);

	// make sure we don't append if this toolbar already exists locally
	bool just_append = true;
	elem = local.documentElement().toElement();
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

	KXMLGUIFactory::saveConfigFile(local, d->m_xmlguiClient->xmlFile(), d->m_xmlguiClient->instance() );

	return;
    }

    // if that didn't work, we save to the config file
    QString grpToolbarStyle;
    if (!strcmp(name(), "unnamed") || !strcmp(name(), "mainToolBar"))
	grpToolbarStyle = "Toolbar style";
    else
	grpToolbarStyle = QString(name()) + " Toolbar style";

    KConfig *config = KGlobal::config();
    KConfigGroupSaver saver(config, grpToolbarStyle);

    config->writeEntry("Position", position);
    config->writeEntry("IconText", icontext);

    config->sync();
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
    if ( !parentWidget() || !parentWidget()->inherits( "QMainWindow" ) )
	return;
    QMainWindow *mw = (QMainWindow*)parentWidget();
    if ( mw->toolBarsMovable() && d->m_enableContext ) {
	if ( m->button() == RightButton ) {
	    int i = context->exec( m->globalPos(), 0 );
	    switch ( i ) {
	    case -1:
		return; // popup cancelled
	    case CONTEXT_LEFT:
		mw->moveToolBar( this, QMainWindow::Left );
		break;
	    case CONTEXT_RIGHT:
		mw->moveToolBar( this, QMainWindow::Right );
		break;
	    case CONTEXT_TOP:
		mw->moveToolBar( this, QMainWindow::Top );
		break;
	    case CONTEXT_BOTTOM:
		mw->moveToolBar( this, QMainWindow::Bottom );
		break;
	    case CONTEXT_FLOAT:
		break;
	    case CONTEXT_FLAT:
		mw->moveToolBar( this, QMainWindow::Minimized );
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
		    kdWarning() << "No such menu item " << i << " in toolbar context menu" << endl;
	    }
	}	
    }
}


void KToolBar::paintEvent(QPaintEvent *)
{
    if ( widgets.isEmpty() ) {
	hide();
	return;
    }
    bool moving = FALSE;
    if ( parentWidget() && parentWidget()->inherits( "QMainWindow" ) )
	moving = ( (QMainWindow*)parentWidget() )->toolBarsMovable();
    // Moved around a little to make variables available for KStyle (mosfet).

    int stipple_height;
    QColorGroup g = QWidget::colorGroup();
    // Took higlighting handle from kmenubar - sven 040198
    QBrush b;
    if ( /*mouseEntered && d->m_highlight */ FALSE ) // ##############
	b = colorGroup().highlight(); // this is much more logical then
    // the hardwired value used before!!
    else
	b = QWidget::backgroundColor();

    QPainter *paint = new QPainter();
    paint->begin( this );

    KStyle::KToolBarPos pos = KStyle::Top;
    if ( parentWidget() && parentWidget()->inherits( "QMainWindow" ) ) {
	QMainWindow::ToolBarDock dock;
	int dm1, dm2;
	bool dm3;
	( (QMainWindow*)parentWidget() )->getLocation( this, dock, dm1, dm3, dm2 );
	pos = (KStyle::KToolBarPos)dock;
    }

    if(kapp->kstyle()){
	kapp->kstyle()->drawKToolBar(paint, 0, 0, width(), height(),
				     colorGroup(), pos,
				     &b);
	if( moving ){
	    if( orientation() == Horizontal )
		kapp->kstyle()->drawKBarHandle(paint, 0, 0, 9, height(),
					       colorGroup(), pos, &b);
	    else
		kapp->kstyle()->drawKBarHandle(paint, 0, 0, width(), 9,
					       colorGroup(), pos, &b);
	}
	paint->end();
	delete paint;
	return;
    }
    if (moving) {
	// Handle point
	if ( orientation() == Horizontal ) {
	    qDrawShadePanel( paint, 0, 0, 9, height(),
			     g , false, 1, &b);
	    paint->setPen( g.light() );
	    paint->drawLine( 9, 0, 9, height());
	    stipple_height = 3;
	    while ( stipple_height < height()-4 ) {
		paint->drawPoint( 1, stipple_height+1);
		paint->drawPoint( 4, stipple_height);
		stipple_height+=3;
	    }
	    paint->setPen( g.dark() );
	    stipple_height = 4;
	    while ( stipple_height < height()-4 ) {
		paint->drawPoint( 2, stipple_height+1);
		paint->drawPoint( 5, stipple_height);
		stipple_height+=3;
	    }
	} else {
	    qDrawShadePanel( paint, 0, 0, width(), 9,
			     g , false, 1, &b);

	    paint->setPen( g.light() );
	    paint->drawLine( 0, 9, width(), 9);
	    stipple_height = 3;
	    while ( stipple_height < width()-4 ) {
		paint->drawPoint( stipple_height+1, 1);
		paint->drawPoint( stipple_height, 4 );
		stipple_height+=3;
	    }
	    paint->setPen( g.dark() );
	    stipple_height = 4;
	    while ( stipple_height < width()-4 ) {
		paint->drawPoint( stipple_height+1, 2 );
		paint->drawPoint( stipple_height, 5);
		stipple_height+=3;
	    }
	}
    } //endif moving

    qDrawShadePanel(paint, 0, 0, width(), height(), g , false, 1);

    paint->end();
    delete paint;
}

void KToolBar::rebuildLayout()
{
    layoutTimer->stop();
    delete bl;
    bl = new QBoxLayout( this, orientation() == Vertical
			 ? QBoxLayout::Down : QBoxLayout::LeftToRight, 2, 0 );
    bl->addSpacing( 9 );
    bl->setDirection( orientation() ==Horizontal ? QBoxLayout::LeftToRight :
		      QBoxLayout::TopToBottom );
    for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	if ( w == rightAligned )
	    continue;
	if ( w->inherits( "KToolBarSeparator" ) &&
 	     !( (KToolBarSeparator*)w )->showLine() ) {
	    bl->addSpacing( 6 );
	    w->hide();
	    continue;
	}
	if ( w->inherits( "QPopupMenu" ) )
	    continue;
	bl->addWidget( w );
    }
    if ( rightAligned ) {
	bl->addStretch();
	bl->addWidget( rightAligned );
    }
    
    if ( fullSize() ) {
	if ( !stretchableWidget && widgets.last() &&
	     !widgets.last()->inherits( "QButton" ) && !widgets.last()->inherits( "KAnimWidget" ) )
	    setStretchableWidget( widgets.last() );
  	if ( !rightAligned )
  	    bl->addStretch();
	if ( stretchableWidget )
	    bl->setStretchFactor( stretchableWidget, 10 );
    }
}

void KToolBar::childEvent( QChildEvent *e )
{
    int dummy = -1;
    if ( e->child()->isWidgetType() ) {
	if ( e->type() == QEvent::ChildInserted )
	    insertWidgetInternal( (QWidget*)e->child(), dummy, -1 );
	else
	    widgets.removeRef( (QWidget*)e->child() );
	if ( isVisibleTo( 0 ) )
	    layoutTimer->start( 0, TRUE );
    }
    QToolBar::childEvent( e );
}

void KToolBar::insertWidgetInternal( QWidget *w, int &index, int id )
{
    if ( inserted.findRef( w ) != -1 )
	return;
    if ( widgets.findRef( w ) != -1 )
	    widgets.removeRef( w );
    if ( index == -1 || index > (int)widgets.count() )
	widgets.append( w );
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

bool KToolBar::highlight() const
{
    return d->m_highlight;
}

void KToolBar::hide()
{
    // Reggie: Ugly hack, I hate it
    if ( parentWidget() && parentWidget()->inherits( "QMainWindow" ) ) {
	QMainWindow::ToolBarDock dock;
	( (QMainWindow*)parentWidget() )->getLocation( (QToolBar*)this, dock, d->realIndex, d->realNl, d->realOffset );
	d->hasRealPos = TRUE;
	( (QMainWindow*)parentWidget() )->moveToolBar( this, QMainWindow::Unmanaged );
	if ( dock != QMainWindow::Unmanaged )
	    d->realPos = dock;
	if ( d->realPos == QMainWindow::Unmanaged )
	    d->realPos = QMainWindow::Top;
    }
    QToolBar::hide();
}

void KToolBar::show()
{
    // Reggie: Ugly hack, I hate it
    if ( d->hasRealPos && d->realPos != QMainWindow::Unmanaged && parentWidget() && parentWidget()->inherits( "QMainWindow" ) ) {
	d->hasRealPos = FALSE;
	( (QMainWindow*)parentWidget() )->moveToolBar( this, d->realPos, d->realIndex, d->realNl, d->realOffset );
    }
    d->hasRealPos = FALSE;
    QObject *o = 0;
    QObjectListIt it( *children() );
    while ( ( o = it.current() ) ) {
	++it;
	if ( !o->inherits( "QWidget" ) || o->inherits( "QPopupMenu" ) )
	    continue;
	( (QWidget*)o )->show();
    }
    QToolBar::show();
}

void KToolBar::resizeEvent( QResizeEvent *e )
{
    setUpdatesEnabled( FALSE );
    QToolBar::resizeEvent( e );
    QTimer::singleShot( 100, this, SLOT( slotRepaint() ) );
}

void KToolBar::slotIconChanged(int group)
{
    if ((group != KIcon::Toolbar) && (group != KIcon::MainToolbar))
	return;
    if ((group == KIcon::MainToolbar) != !strcmp(name(), "mainToolBar"))
	return;

    emit modechange();
    if (isVisible())
	updateGeometry();
}

void KToolBar::slotReadConfig()
{
    // read in the global ('kdeglobals') config file
    KConfig *config = KGlobal::config();

    static QString grpKDE     = QString::fromLatin1("KDE");

    static QString attrIconText  = QString::fromLatin1("IconText");
    static QString attrHighlight = QString::fromLatin1("Highlighting");
    static QString attrTrans     = QString::fromLatin1("TransparentMoving");
    static QString attrIconStyle = QString::fromLatin1("KDEIconStyle");
    static QString attrSize      = QString::fromLatin1("IconSize");
    static QString attrPosition  = QString::fromLatin1("Position");

    // we actually do this in two steps.  first, we read in the global
    // styles [Toolbar style].  then, if the toolbar is NOT
    // 'mainToolBar', we will also try to read in [barname Toolbar style]
    bool highlight;
    int transparent;
    QString icontext;
    int iconsize;
    QString position;

    // this is the first iteration
    QString grpToolbar(QString::fromLatin1("Toolbar style"));
    { // start block for KConfigGroupSaver
	KConfigGroupSaver saver(config, grpToolbar);

	// first, get the generic settings
	highlight   = config->readBoolEntry(attrHighlight, true);
	transparent = config->readBoolEntry(attrTrans, true);
	
	// we read in the IconText property *only* if we intend on actually
	// honoring it
	if (d->m_honorStyle)
	    icontext = config->readEntry(attrIconText, "IconOnly");
	else
	    icontext = "IconOnly";

	// Use the default icon size for toolbar icons. This is not specified in
	// the [Toolbar style] section but in the [Icons] section.
	iconsize = 0;

	position = config->readEntry(attrPosition, "Top");

	// okay, that's done.  now we look for a toolbar specific entry
	grpToolbar = name() + QString::fromLatin1(" Toolbar style");
	if (config->hasGroup(grpToolbar)) {
	    config->setGroup(grpToolbar);

		// first, get the generic settings
	    highlight   = config->readBoolEntry(attrHighlight, highlight);
	    transparent = config->readBoolEntry(attrTrans, transparent);

		// now we always read in the IconText property
	    icontext = config->readEntry(attrIconText, "icontext");

	    // now get the size: FIXME: Sizes are not yet saved.
	    // iconsize = config->readNumEntry(attrSize, iconsize);

	    // finally, get the position
	    position = config->readEntry(attrPosition, position);
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
	setIconText(icon_text, false);
	doUpdate = true;
    }

    // ...and check if the icon size has changed
    if (iconsize != d->m_iconSize) {
	setIconSize(iconsize, false);
	doUpdate = true;
    }

    QMainWindow *mw = 0;
    if ( parentWidget() && parentWidget()->inherits( "QMainWindow" ) )
	mw = (QMainWindow*)parentWidget();

    // ...and if we should highlight
    if ( mw && highlight != d->m_highlight ) {
	d->m_highlight = highlight;
	doUpdate = true;
    }

    // ...and if we should move transparently
    if ( mw && transparent != (!mw->opaqueMoving()) ) {
	mw->setOpaqueMoving( transparent );
	doUpdate = false;
    }

    // ...and now the position stuff
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
    setBarPos( pos );

    if (doUpdate)
	emit modechange(); // tell buttons what happened
    if (isVisible ())
	updateGeometry();
}

bool KToolBar::event( QEvent *e )
{
    if ( e->type() == QEvent::LayoutHint )
	QTimer::singleShot( 100, this, SLOT( slotRepaint() ) );
    return QToolBar::event( e );
}

void KToolBar::slotRepaint()
{
    setUpdatesEnabled( TRUE );
    repaint( FALSE );
}

#include "ktoolbar.moc"

