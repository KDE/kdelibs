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
    }
    bool topLevel;
    int frameStyle;
};

KMenuBar::KMenuBar(QWidget *parent, const char *name)
  : QMenuBar(parent, name)
{
    d = new KMenuBarPrivate;

    mouseActive = false;

    connect( kapp, SIGNAL(appearanceChanged()), this, SLOT(slotReadConfig()));

    slotReadConfig();
}

KMenuBar::~KMenuBar()
{
  delete d; d = 0;
}

void KMenuBar::setTopLevelMenu(bool top_level)
{
    if ( isTopLevelMenu() == top_level )
        return;
  d->topLevel = top_level;
  if ( isTopLevelMenu() ) {
      bool wasVisible = isVisibleTo(0);
      d->frameStyle = frameStyle();
      removeEventFilter( topLevelWidget() );
      reparent( parentWidget(), WType_TopLevel | WType_Dialog | WStyle_NoBorder, QPoint(0,0), false  );
      hide(); // worakround for a qt < 2.2.2  bug
#ifndef Q_WS_QWS //FIXME
      KWin::setType( winId(), NET::Menu );
      KWin::setOnAllDesktops( winId(), true );
      KWin::setState( winId(), NET::StaysOnTop );
#endif
      setFrameStyle( MenuBarPanel );
      installEventFilter( parentWidget()->topLevelWidget() );
      if ( wasVisible )
          show();
  } else {
      if ( parentWidget() ) {
          reparent( parentWidget(), QPoint(0,0), TRUE );
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

void KMenuBar::slotReadConfig()
{
  static const QString &grpKDE = KGlobal::staticQString("KDE");
  static const QString &keyMac = KGlobal::staticQString("macStyle");

  KConfig *config = KGlobal::config();
  KConfigGroupSaver saver( config, grpKDE );
  setTopLevelMenu( config->readBoolEntry( keyMac, false ) );
}

void KMenuBar::drawContents(QPainter *p)
{
    QMenuBar::drawContents(p);
}

void KMenuBar::enterEvent(QEvent *ev)
{
    mouseActive = true;
    QMenuBar::enterEvent(ev);
}

void KMenuBar::leaveEvent(QEvent *ev)
{
    mouseActive = false;
    QMenuBar::leaveEvent(ev);
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

	    if ( ev->type() == QEvent::Show && isHidden() )
		show();
	    else if ( ev->type() == QEvent::WindowActivate )
		raise();
	}
    }
    return QMenuBar::eventFilter( obj, ev );
}


void KMenuBar::show()
{
    // work around a Qt bug
    if ( !isVisible() )
	QMenuBar::show();
}

void KMenuBar::showEvent( QShowEvent* )
{
    if ( d->topLevel ) {
        QRect area = QApplication::desktop()->screenGeometry();
        setGeometry(area.left(), area.top(), area.width(), heightForWidth( area.width() ) );
#ifndef Q_WS_QWS //FIXME
        KWin::setStrut( winId(), 0, 0, height(), 0 );
#endif
    }
}

#include "kmenubar.moc"
