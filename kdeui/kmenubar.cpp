/*

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    */


#ifndef INCLUDE_MENUITEM_DEF
#define INCLUDE_MENUITEM_DEF
#endif

#include <qevent.h>
#include <qobjectlist.h>
#include <qaccel.h>

#include <kconfig.h>
#include <kglobalsettings.h>
#include <kmenubar.h>
#include <kapplication.h>
#include <kwin.h>
#include <kwinmodule.h>
#include <kglobal.h>
#include <kdebug.h>
#include <qxembed.h>
#include <kmanagerselection.h>
#include <qtimer.h>

#ifndef Q_WS_QWS
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
 - Fitts' Law

*/

class KMenuBar::KMenuBarPrivate
{
public:
    KMenuBarPrivate()
	:   forcedTopLevel( false ),
	    topLevel( false ),
	    wasTopLevel( false ),
            selection( NULL )
	{
	}
    ~KMenuBarPrivate()
        {
        delete selection;
        }
    bool forcedTopLevel;
    bool topLevel;
    bool wasTopLevel; // when TLW is fullscreen, remember state
    int frameStyle; // only valid in toplevel mode
    int lineWidth;  // dtto
    int margin;     // dtto
    bool fallback_mode; // dtto
    KSelectionWatcher* selection;
    QTimer selection_timer;
    static Atom makeSelectionAtom();
};

static Atom selection_atom = None;
static Atom msg_type_atom = None;

static
void initAtoms()
{
    char nm[ 100 ];
    sprintf( nm, "_KDE_TOPMENU_OWNER_S%d", DefaultScreen( qt_xdisplay()));
    char nm2[] = "_KDE_TOPMENU_MINSIZE";
    char* names[ 2 ] = { nm, nm2 };
    Atom atoms[ 2 ];
    XInternAtoms( qt_xdisplay(), names, 2, False, atoms );
    selection_atom = atoms[ 0 ];
    msg_type_atom = atoms[ 1 ];
}

Atom KMenuBar::KMenuBarPrivate::makeSelectionAtom()
{
    if( selection_atom == None )
	initAtoms();
    return selection_atom;
}

KMenuBar::KMenuBar(QWidget *parent, const char *name)
  : QMenuBar(parent, name)
{
    QXEmbed::initialize();

    d = new KMenuBarPrivate;
    connect( &d->selection_timer, SIGNAL( timeout()),
        this, SLOT( selectionTimeout()));
    
#ifdef XRANDR_SUPPORT
    connect( qApp->desktop(), SIGNAL( resized( int )), SLOT( updateFallbackSize()));
#endif
    
    if ( kapp )
        // toolbarAppearanceChanged(int) is sent when changing macstyle
        connect( kapp, SIGNAL(toolbarAppearanceChanged(int)),
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

  if( parentWidget()
      && parentWidget()->topLevelWidget()->isFullScreen())
  {
    d->wasTopLevel = top_level;
    top_level = false;
  }

  if ( isTopLevelMenu() == top_level )
    return;
  d->topLevel = top_level;
  if ( isTopLevelMenu() )
  {
      d->selection = new KSelectionWatcher( KMenuBarPrivate::makeSelectionAtom(),
          DefaultScreen( qt_xdisplay()));
      connect( d->selection, SIGNAL( newOwner( Window )),
          this, SLOT( updateFallbackSize()));
      connect( d->selection, SIGNAL( lostOwner()),
          this, SLOT( updateFallbackSize()));
      kapp->installX11EventFilter( this );
      d->frameStyle = frameStyle();
      d->lineWidth = lineWidth();
      d->margin = margin();
      d->fallback_mode = false;
      bool wasShown = !isHidden();
      reparent( parentWidget(), WType_TopLevel | WType_Dialog | WStyle_NoBorder, QPoint(0,0), false );
#ifndef Q_WS_QWS //FIXME
      KWin::setType( winId(), NET::TopMenu );
#endif
      QMenuBar::setFrameStyle( NoFrame );
      QMenuBar::setLineWidth( 0 );
      QMenuBar::setMargin( 0 );
      updateFallbackSize();
      if ( wasShown )
          show();
  } else
  {
      delete d->selection;
      d->selection = NULL;
      kapp->removeX11EventFilter( this );
      setBackgroundMode( PaletteButton );
      setFrameStyle( d->frameStyle );
      setLineWidth( d->lineWidth );
      setMargin( d->margin );
      setMinimumSize( 0, 0 );
      setMaximumSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
      menuContentsChanged(); // trigger invalidating calculated size
      resize( sizeHint());   // and resize to preferred size
      if ( parentWidget() )
          reparent( parentWidget(), QPoint(0,0), !isHidden());
  }
}

bool KMenuBar::isTopLevelMenu() const
{
  return d->topLevel;
}

// KDE4 remove
void KMenuBar::show()
{
    QMenuBar::show();
}

void KMenuBar::slotReadConfig()
{
  KConfig *config = KGlobal::config();
  KConfigGroupSaver saver( config, "KDE" );
  setTopLevelMenuInternal( config->readBoolEntry( "macStyle", false ) );
}

bool KMenuBar::eventFilter(QObject *obj, QEvent *ev)
{
    if ( d->topLevel )
    {
	if ( parentWidget() && obj == parentWidget()->topLevelWidget()  )
        {
	    if( ev->type() == QEvent::Resize )
		return false; // ignore resizing of parent, QMenuBar would try to adjust size
	    if ( ev->type() == QEvent::Accel || ev->type() == QEvent::AccelAvailable )
            {
		if ( QApplication::sendEvent( topLevelWidget(), ev ) )
		    return TRUE;
	    }
            if(ev->type() == QEvent::ShowFullScreen )
                // will update the state properly
                setTopLevelMenuInternal( d->topLevel );
	}
    }
    else
    {
        if( topLevelWidget() && obj == topLevelWidget())
        {
            if( ev->type() == QEvent::ShowNormal )
                setTopLevelMenuInternal( d->wasTopLevel );
        }
    }
    return QMenuBar::eventFilter( obj, ev );
}

// KDE4 remove
void KMenuBar::showEvent( QShowEvent *e )
{
    QMenuBar::showEvent(e);
}

void KMenuBar::updateFallbackSize()
{
    if( !d->topLevel )
	return;
    if( d->selection->owner() != None )
    { // somebody is managing us, don't mess anything, undo changes
      // done in fallback mode if needed
        d->selection_timer.stop();
        if( d->fallback_mode )
        {
            d->fallback_mode = false;
//            KWin::setStrut( winId(), 0, 0, 0, 0 ); KWin will set strut as it will see fit
            setMaximumSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
            menuContentsChanged();
            resize( sizeHint());
        }
	return;
    }
    if( d->selection_timer.isActive())
	return;
    d->selection_timer.start( 100, true );
}

void KMenuBar::selectionTimeout()
{ // nobody is managing us, handle resizing
    if ( d->topLevel )
    {
        d->fallback_mode = true; // KMenuBar is handling its position itself
        KConfigGroup xineramaConfig(KGlobal::config(),"Xinerama");
        int screen = xineramaConfig.readNumEntry("MenubarScreen",
            QApplication::desktop()->screenNumber(QPoint(0,0)) );
        QRect area = QApplication::desktop()->screenGeometry(screen);
#if QT_VERSION < 0x030200
        int margin = frameWidth() + 2;
#else  // hopefully I'll manage to persuade TT on Fitts' Law for QMenuBar for Qt-3.2
        int margin = 0;
#endif
	move(area.left() - margin, area.top() - margin); 
        setFixedSize(area.width() + 2* margin , heightForWidth( area.width() + 2 * margin ) );
#ifndef Q_WS_QWS //FIXME
        int strut_height = height() - margin;
        if( strut_height < 0 )
            strut_height = 0;
        KWin::setStrut( winId(), 0, 0, strut_height, 0 );
#endif
    }
}

// KDE4 remove
void KMenuBar::resizeEvent( QResizeEvent *e )
{
    QMenuBar::resizeEvent(e);
}

bool KMenuBar::block_resize = false;

void KMenuBar::setGeometry( int x, int y, int w, int h )
{
    if( block_resize )
    {
	QMenuBar::setGeometry( x, y, width(), height());
	return;
    }
    checkResizingToParent( w, h );
    QMenuBar::setGeometry( x, y, w, h );
}

void KMenuBar::resize( int w, int h )
{
    if( block_resize )
	return;
    checkResizingToParent( w, h );
    QMenuBar::resize( w, h );
//    kdDebug() << "RS:" << w << ":" << h << ":" << width() << ":" << height() << ":" << minimumWidth() << ":" << minimumHeight() << endl;
}

void KMenuBar::checkResizingToParent( int& w, int& h )
{
    if( !d->topLevel || d->selection->owner() == None )
	return;
    if( parentWidget() && parentWidget()->width() == w )
    { // Menubar is possibly being attempted to be resized to match
      // mainwindow size. Resize to sizeHint() instead. Since
      // sizeHint() may indirectly call resize(), avoid infinite
      // recursion.
	block_resize = true;
	QSize s = sizeHint();
	w = s.width();
	h = s.height();
	block_resize = false;
    }
}

bool KMenuBar::x11Event( XEvent* ev )
{
    if( ev->type == ClientMessage && ev->xclient.message_type == msg_type_atom
        && ev->xclient.window == winId())
    {
        // QMenuBar is trying really hard to keep the size it deems right.
        // Forcing minimum size and blocking resizing to match parent size
        // in checkResizingToParent() seem to be the only way to make
        // KMenuBar keep the size it wants
	setMinimumSize( ev->xclient.data.l[ 1 ], ev->xclient.data.l[ 2 ] );
        menuContentsChanged();
        resize( sizeHint());
	return true;
    }
    return QMenuBar::x11Event( ev );
}

void KMenuBar::setFrameStyle( int style )
{
    if( d->topLevel )
	d->frameStyle = style;
    else
	QMenuBar::setFrameStyle( style );
}

void KMenuBar::setLineWidth( int width )
{
    if( d->topLevel )
	d->lineWidth = width;
    else
	QMenuBar::setLineWidth( width );
}

void KMenuBar::setMargin( int margin )
{
    if( d->topLevel )
	d->margin = margin;
    else
	QMenuBar::setMargin( margin );
}

void KMenuBar::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kmenubar.moc"
