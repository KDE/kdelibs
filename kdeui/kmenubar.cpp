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

#ifndef Q_WS_QWS
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif


class KMenuBar::KMenuBarPrivate
{
public:
    KMenuBarPrivate()
    {
      topLevel = false;
      forcedTopLevel = false;
      wasTopLevel = false;
    }
    bool forcedTopLevel;
    bool topLevel;
    int frameStyle;
    bool wasTopLevel; // when TLW is fullscreen, remember state
};

KMenuBar::KMenuBar(QWidget *parent, const char *name)
  : QMenuBar(parent, name)
{
    d = new KMenuBarPrivate;
    d->frameStyle = frameStyle();

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

#if QT_VERSION < 0x030100
namespace
{
class QWidgetHack
    : public QWidget
    {
    public:
        bool isFullScreen() { return isTopLevel() && topData()->fullscreen; }
    };
}
#endif

void KMenuBar::setTopLevelMenuInternal(bool top_level)
{
  if (d->forcedTopLevel)
    top_level = true;

  if( parentWidget()
#if QT_VERSION >= 0x030100
      && parentWidget()->topLevelWidget()->isFullScreen()) {
#else
      && static_cast<QWidgetHack*>(parentWidget()->topLevelWidget())->isFullScreen()) {
#endif
    d->wasTopLevel = top_level;
    top_level = false;
  }

  if ( isTopLevelMenu() == top_level )
    return;
  d->topLevel = top_level;
  if ( isTopLevelMenu() ) {
      bool wasVisible = isVisible();
      d->frameStyle = frameStyle();
      removeEventFilter( topLevelWidget() );
      reparent( parentWidget(), WType_TopLevel | WType_Dialog | WStyle_NoBorder, QPoint(0,0), false );
#ifndef Q_WS_QWS //FIXME
      KWin::setType( winId(), NET::Menu );
#endif
      setFrameStyle( MenuBarPanel );
      installEventFilter( parentWidget()->topLevelWidget() );
      if ( wasVisible )
          show();
  } else {
      if ( parentWidget() ) {
          reparent( parentWidget(), QPoint(0,0), isVisible());
          setBackgroundMode( PaletteButton );
          installEventFilter( topLevelWidget() );
          setFrameStyle( d->frameStyle );
      }
  }
}

bool KMenuBar::isTopLevelMenu() const
{
  return d->topLevel;
}

void KMenuBar::show()
{
    // work around a Qt bug
    // why is this still needed? (Simon)
    if ( d->topLevel && isVisible() )
	return;

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

    if ( d->topLevel ) {
	if ( ev->type() == QEvent::Resize )
	    return FALSE; // hinder QMenubar to adjust its size
	if ( parentWidget() && obj == parentWidget()->topLevelWidget()  ) {

	    if ( ev->type() == QEvent::Accel || ev->type() == QEvent::AccelAvailable ) {
		if ( QApplication::sendEvent( topLevelWidget(), ev ) )
		    return TRUE;
	    }

//	    if ( ev->type() == QEvent::Show && isHidden())
//		show();  doesn't seem to do anything besides breaking things
//	    else
            if ( ev->type() == QEvent::WindowActivate )
		raise();
            else if(ev->type() == QEvent::ShowFullScreen )
                // will update the state properly
                setTopLevelMenuInternal( d->topLevel );
	}
    } else {
        if( topLevelWidget() && obj == topLevelWidget()) {
            if( ev->type() == QEvent::ShowNormal )
                setTopLevelMenuInternal( d->wasTopLevel );
        }
    }
    return QMenuBar::eventFilter( obj, ev );
}


void KMenuBar::showEvent( QShowEvent *e )
{
    if ( d->topLevel ) {
        KConfigGroup xineramaConfig(KGlobal::config(),"Xinerama");
        int screen = xineramaConfig.readNumEntry("MenubarScreen",
            QApplication::desktop()->screenNumber(QPoint(0,0)) );
        QRect area = QApplication::desktop()->screenGeometry(screen);
        QMenuBar::setGeometry(area.left(), area.top()-frameWidth()-2, area.width(), heightForWidth( area.width() ) );
#ifndef Q_WS_QWS //FIXME
        KWin::setStrut( winId(), 0, 0, height()-frameWidth()-2, 0 );
#endif
    }
    QMenuBar::showEvent(e);
}

void KMenuBar::setGeometry( int x, int y, int w, int h )
{
   // With the toolbar in toplevel-mode it sometimes has the tendency to cuddle up in
   // the topleft corner due to a misguided attempt from the layout manager (?) to
   // size us. The follow line filters out any resize attempt while in toplevel-mode.
   if ( !d->topLevel )
       QMenuBar::setGeometry(x,y,w,h);
}

void KMenuBar::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }




#include "kmenubar.moc"
