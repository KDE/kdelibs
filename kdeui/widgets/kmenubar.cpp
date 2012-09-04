/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998, 1999, 2000  Sven Radej (radej@kde.org)
    Copyright (C) 1997, 1998, 1999, 2000 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 1999, 2000 Daniel "Mosfet" Duley (mosfet@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
    */


#include "kmenubar.h"

#include <config-kdeui.h>

#include <stdio.h>

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QActionEvent>
#include <QDesktopWidget>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionMenuItem>

#include <kconfig.h>
#include <kglobalsettings.h>
#include <qapplication.h>
#include <kdebug.h>
#include <kmanagerselection.h>
#include <kconfiggroup.h>
#include <kwindowsystem.h>

#if HAVE_X11
#include <qx11info_x11.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif

/*

 Toplevel menubar (not for the fallback size handling done by itself):
 - should not alter position or set strut
 - every toplevel must have at most one matching topmenu
 - embedder won't allow shrinking below a certain size
 - must have WM_TRANSIENT_FOR pointing the its mainwindow
     - the exception is desktop's menubar, which can be transient for root window
       because of using root window as the desktop window
 - Fitts' Law

*/

static int block_resize = 0;

class KMenuBar::KMenuBarPrivate
{
public:
    KMenuBarPrivate()
	:   forcedTopLevel( false ),
	    topLevel( false ),
	    wasTopLevel( false ),
#if HAVE_X11
	    selection( NULL ),
#endif
            min_size( 0, 0 )
	{
	}
    ~KMenuBarPrivate()
        {
#if HAVE_X11
        delete selection;
#endif
        }
    int frameStyle; // only valid in toplevel mode
    int lineWidth;  // dtto
    int margin;     // dtto
    bool fallback_mode : 1; // dtto

    bool forcedTopLevel : 1;
    bool topLevel : 1;
    bool wasTopLevel : 1; // when TLW is fullscreen, remember state

#if HAVE_X11
    KSelectionWatcher* selection;
#endif
    QTimer selection_timer;
    QSize min_size;
    static Atom makeSelectionAtom();
};

#if HAVE_X11
static Atom selection_atom = None;
static Atom msg_type_atom = None;

static
void initAtoms()
{
    char nm[ 100 ];
    sprintf( nm, "_KDE_TOPMENU_OWNER_S%d", DefaultScreen( QX11Info::display()));
    char nm2[] = "_KDE_TOPMENU_MINSIZE";
    char* names[ 2 ] = { nm, nm2 };
    Atom atoms[ 2 ];
    XInternAtoms( QX11Info::display(), names, 2, False, atoms );
    selection_atom = atoms[ 0 ];
    msg_type_atom = atoms[ 1 ];
}
#endif

Atom KMenuBar::KMenuBarPrivate::makeSelectionAtom()
{
#if HAVE_X11
    if( selection_atom == None )
	initAtoms();
    return selection_atom;
#else
    return 0;
#endif
}

KMenuBar::KMenuBar(QWidget *parent)
    : QMenuBar(parent), d(new KMenuBarPrivate)
{
    connect( &d->selection_timer, SIGNAL(timeout()),
        this, SLOT(selectionTimeout()));

    connect( qApp->desktop(), SIGNAL(resized(int)), SLOT(updateFallbackSize()));

    // toolbarAppearanceChanged(int) is sent when changing macstyle
    connect( KGlobalSettings::self(), SIGNAL(toolbarAppearanceChanged(int)),
        this, SLOT(slotReadConfig()));

    slotReadConfig();
}

KMenuBar::~KMenuBar()
{
  delete d;
}

void KMenuBar::setTopLevelMenu(bool top_level)
{
  d->forcedTopLevel = top_level;
  setTopLevelMenuInternal( top_level );
}

void KMenuBar::setTopLevelMenuInternal(bool top_level)
{
  if (d->forcedTopLevel)
    top_level = true;

  d->wasTopLevel = top_level;
  if( parentWidget()
      && parentWidget()->topLevelWidget()->isFullScreen())
    top_level = false;

  if ( isTopLevelMenu() == top_level )
    return;
  d->topLevel = top_level;
  if ( isTopLevelMenu() )
  {
#if HAVE_X11
      d->selection = new KSelectionWatcher( KMenuBarPrivate::makeSelectionAtom(),
          DefaultScreen( QX11Info::display()));
      connect( d->selection, SIGNAL(newOwner(Window)),
          this, SLOT(updateFallbackSize()));
      connect( d->selection, SIGNAL(lostOwner()),
          this, SLOT(updateFallbackSize()));
#endif
      d->frameStyle = 0; //frameStyle();
      d->lineWidth = 0; //lineWidth();
      d->margin = 0; //margin();
      d->fallback_mode = false;
      bool wasShown = !isHidden();
      setParent(parentWidget(), Qt::Window | Qt::Tool | Qt::FramelessWindowHint);
      setGeometry(0,0,width(),height());
#if HAVE_X11
      KWindowSystem::setType( winId(), NET::TopMenu );
#endif

      if( parentWidget())
          KWindowSystem::setMainWindow( this, parentWidget()->topLevelWidget()->winId());
      //QMenuBar::setFrameStyle( NoFrame );
      //QMenuBar::setLineWidth( 0 );
      //QMenuBar::setMargin( 0 );
      updateFallbackSize();
      d->min_size = QSize( 0, 0 );
      if( parentWidget() && !parentWidget()->isTopLevel())
          setVisible( parentWidget()->isVisible());
      else if ( wasShown )
          show();
  } else
  {
#if HAVE_X11
      delete d->selection;
      d->selection = NULL;
#endif
      setAttribute(Qt::WA_NoSystemBackground, false);
      setBackgroundRole(QPalette::Button);
      setFrameStyle( d->frameStyle );
      setLineWidth( d->lineWidth );
      setMargin( d->margin );
      setMinimumSize( 0, 0 );
      setMaximumSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
      updateMenuBarSize();
      if ( parentWidget() )
          setParent( parentWidget() );
  }
}

bool KMenuBar::isTopLevelMenu() const
{
  return d->topLevel;
}


void KMenuBar::slotReadConfig()
{
  KConfigGroup cg( KSharedConfig::openConfig(), "KDE" );
  setTopLevelMenuInternal( cg.readEntry( "macStyle", false ) );
}

bool KMenuBar::eventFilter(QObject *obj, QEvent *ev)
{
    if ( d->topLevel )
    {
	if ( parentWidget() && obj == parentWidget()->topLevelWidget()  )
        {
	    if( ev->type() == QEvent::Resize )
		return false; // ignore resizing of parent, QMenuBar would try to adjust size
#ifdef QT3_SUPPORT
	    if ( ev->type() == QEvent::Accel || ev->type() == QEvent::AccelAvailable )
            {
		if ( QApplication::sendEvent( topLevelWidget(), ev ) )
		    return true;
	    }
#endif
            /* FIXME QEvent::ShowFullScreen is no more
            if(ev->type() == QEvent::ShowFullScreen )
                // will update the state properly
                setTopLevelMenuInternal( d->topLevel );
            */
        }
        if( parentWidget() && obj == parentWidget() && ev->type() == QEvent::ParentChange )
            {
            KWindowSystem::setMainWindow( this, parentWidget()->topLevelWidget()->winId());
            setVisible( parentWidget()->isTopLevel() || parentWidget()->isVisible());
            }
        if( parentWidget() && !parentWidget()->isTopLevel() && obj == parentWidget())
        { // if the parent is not toplevel, KMenuBar needs to match its visibility status
            if( ev->type() == QEvent::Show )
                {
                KWindowSystem::setMainWindow( this, parentWidget()->topLevelWidget()->winId());
                show();
                }
            if( ev->type() == QEvent::Hide )
                hide();
	}
    }
    else
    {
        if( parentWidget() && obj == parentWidget()->topLevelWidget())
        {
            if( ev->type() == QEvent::WindowStateChange
                && !parentWidget()->topLevelWidget()->isFullScreen() )
                setTopLevelMenuInternal( d->wasTopLevel );
        }
    }
    return QMenuBar::eventFilter( obj, ev );
}


void KMenuBar::updateFallbackSize()
{
    if( !d->topLevel )
	return;
#if HAVE_X11
    if( d->selection->owner() != None )
#endif
    { // somebody is managing us, don't mess anything, undo changes
      // done in fallback mode if needed
        d->selection_timer.stop();
        if( d->fallback_mode )
        {
            d->fallback_mode = false;
            KWindowSystem::setStrut( winId(), 0, 0, 0, 0 );
            setMinimumSize( 0, 0 );
            setMaximumSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
            updateMenuBarSize();
        }
	return;
    }
    if( d->selection_timer.isActive())
	return;
    d->selection_timer.setInterval(100);
    d->selection_timer.setSingleShot(true);
    d->selection_timer.start();
}

void KMenuBar::selectionTimeout()
{ // nobody is managing us, handle resizing
    if ( d->topLevel )
    {
        d->fallback_mode = true; // KMenuBar is handling its position itself
        KConfigGroup xineramaConfig(KSharedConfig::openConfig(),"Xinerama");
        int screen = xineramaConfig.readEntry("MenubarScreen",
            QApplication::desktop()->screenNumber(QPoint(0,0)) );
        QRect area = QApplication::desktop()->screenGeometry(screen);
        int margin = 0;
	move(area.left() - margin, area.top() - margin);
        setFixedSize(area.width() + 2* margin , heightForWidth( area.width() + 2 * margin ) );
#if HAVE_X11
        int strut_height = height() - margin;
        if( strut_height < 0 )
            strut_height = 0;
        KWindowSystem::setStrut( winId(), 0, 0, strut_height, 0 );
#endif
    }
}

void KMenuBar::resizeEvent( QResizeEvent *e )
{
    if( e->spontaneous() && d->topLevel && !d->fallback_mode )
        {
        ++block_resize; // do not respond with configure request to ConfigureNotify event
        QMenuBar::resizeEvent(e); // to avoid possible infinite loop
        --block_resize;
        }
    else
        QMenuBar::resizeEvent(e);
}

void KMenuBar::setGeometry( const QRect& r )
{
    setGeometry( r.x(), r.y(), r.width(), r.height() );
}

void KMenuBar::setGeometry( int x, int y, int w, int h )
{
    if( block_resize > 0 )
    {
	move( x, y );
	return;
    }
    checkSize( w, h );
    if( geometry() != QRect( x, y, w, h ))
        QMenuBar::setGeometry( x, y, w, h );
}

void KMenuBar::resize( int w, int h )
{
    if( block_resize > 0 )
	return;
    checkSize( w, h );
    if( size() != QSize( w, h ))
        QMenuBar::resize( w, h );
//    kDebug() << "RS:" << w << ":" << h << ":" << width() << ":" << height() << ":" << minimumWidth() << ":" << minimumHeight();
}

void KMenuBar::resize( const QSize& s )
{
    QMenuBar::resize( s );
}

void KMenuBar::checkSize( int& w, int& h )
{
    if( !d->topLevel || d->fallback_mode )
	return;
    QSize s = sizeHint();
    w = s.width();
    h = s.height();
    // This is not done as setMinimumSize(), because that would set the minimum
    // size in WM_NORMAL_HINTS, and KWin would not allow changing to smaller size
    // anymore
    w = qMax( w, d->min_size.width());
    h = qMax( h, d->min_size.height());
}

// QMenuBar's sizeHint() gives wrong size (insufficient width), which causes wrapping in the kicker applet
QSize KMenuBar::sizeHint() const
{
    if( !d->topLevel || block_resize > 0 )
        return QMenuBar::sizeHint();
    // Since QMenuBar::sizeHint() may indirectly call resize(),
    // avoid infinite recursion.
    ++block_resize;
    // find the minimum useful height, and enlarge the width until the menu fits in that height (one row)
    int h = heightForWidth( 1000000 );
    int w = QMenuBar::sizeHint().width();
    // optimization - don't call heightForWidth() too many times
    while( heightForWidth( w + 12 ) > h )
        w += 12;
    while( heightForWidth( w + 4 ) > h )
        w += 4;
    while( heightForWidth( w ) > h )
        ++w;
    --block_resize;
    return QSize( w, h );
}

#pragma message("Port to Qt5 native filter")
#if 0
bool KMenuBar::x11Event( XEvent* ev )
{
    if( ev->type == ClientMessage && ev->xclient.message_type == msg_type_atom
        && ev->xclient.window == winId())
    {
        // QMenuBar is trying really hard to keep the size it deems right.
        // Forcing minimum size and blocking resizing to match parent size
        // in checkResizingToParent() seem to be the only way to make
        // KMenuBar keep the size it wants
	d->min_size = QSize( ev->xclient.data.l[ 1 ], ev->xclient.data.l[ 2 ] );
//        kDebug() << "MINSIZE:" << d->min_size;
        updateMenuBarSize();
	return true;
    }
    return QMenuBar::x11Event( ev );
}
#endif

void KMenuBar::updateMenuBarSize()
    {
    //menuContentsChanged(); // trigger invalidating calculated size
    resize( sizeHint());   // and resize to preferred size
    }

void KMenuBar::setFrameStyle( int style )
{
    if( d->topLevel )
	d->frameStyle = style;
//     else
// 	QMenuBar::setFrameStyle( style );
}

void KMenuBar::setLineWidth( int width )
{
    if( d->topLevel )
	d->lineWidth = width;
//     else
// 	QMenuBar::setLineWidth( width );
}

void KMenuBar::setMargin( int margin )
{
    if( d->topLevel )
	d->margin = margin;
//     else
// 	QMenuBar::setMargin( margin );
}

void KMenuBar::closeEvent( QCloseEvent* e )
{
    if( d->topLevel )
        e->ignore(); // mainly for the fallback mode
    else
        QMenuBar::closeEvent( e );
}

void KMenuBar::paintEvent( QPaintEvent* pe )
{
    // Closes the BR77113
    // We need to overload this method to paint only the menu items
    // This way when the KMenuBar is embedded in the menu applet it
    // integrates correctly.
    //
    // Background mode and origin are set so late because of styles
    // using the polish() method to modify these settings.
    //
    // Of course this hack can safely be removed when real transparency
    // will be available

//    if( !d->topLevel )
    {
        QMenuBar::paintEvent(pe);
    }
#if 0
    else
    {
        QPainter p(this);
        bool up_enabled = isUpdatesEnabled();
        Qt::BackgroundMode bg_mode = backgroundMode();
        BackgroundOrigin bg_origin = backgroundOrigin();

        setUpdatesEnabled(false);
        setBackgroundMode(Qt::X11ParentRelative);
        setBackgroundOrigin(WindowOrigin);

	p.eraseRect( rect() );
	erase();

        QColorGroup g = colorGroup();
        bool e;

        for ( int i=0; i<(int)count(); i++ )
        {
            QMenuItem *mi = findItem( idAt( i ) );

            if ( !mi->text().isEmpty() || !mi->icon().isNull() )
            {
                QRect r = itemRect(i);
                if(r.isEmpty() || !mi->isVisible())
                    continue;

                e = mi->isEnabled() && mi->isVisible();
                if ( e )
                    g = isEnabled() ? ( isActiveWindow() ? palette().active() :
                                        palette().inactive() ) : palette().disabled();
                else
                    g = palette().disabled();

                bool item_active = ( activeAction() ==  mi );

                p.setClipRect(r);

                if( item_active )
                {
                    QStyleOptionMenuItem miOpt;
                    miOpt.init(this);
                    miOpt.rect = r;
                    miOpt.text = mi->text();
                    miOpt.icon = mi->icon();
                    miOpt.palette = g;

                    QStyle::State flags = QStyle::State_None;
                    if (isEnabled() && e)
                        flags |= QStyle::State_Enabled;
                    if ( item_active )
                        flags |= QStyle::State_Active;
                    if ( item_active && actItemDown )
                        flags |= QStyle::State_Down;
                    flags |= QStyle::State_HasFocus;

                    mi->state = flags;


                    style()->drawControl(QStyle::CE_MenuBarItem, &miOpt, &p, this);
                }
                else
                {
                    style()->drawItem(p, r, Qt::AlignCenter | Qt::AlignVCenter | Qt::TextShowMnemonic,
                                     g, e, mi->pixmap(), mi->text());
                }
            }
        }

        setBackgroundOrigin(bg_origin);
        setBackgroundMode(bg_mode);
        setUpdatesEnabled(up_enabled);
    }
#endif
}

