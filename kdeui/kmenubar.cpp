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
#include <kstyle.h>
#include <kapp.h>
#include <kwin.h>
#include <kwinmodule.h>
#include <kglobal.h>

#ifndef _WS_QWS_
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
      reparent( parentWidget(), WType_TopLevel | WStyle_Dialog | WStyle_NoBorderEx, QPoint(0,0), false  );
      hide(); // worakround for a qt < 2.2.2  bug
#ifndef _WS_QWS_ //FIXME
      KWin::setType( winId(), NET::Menu );
      KWin::setOnAllDesktops( winId(), true );
      KWin::setState( winId(), NET::StaysOnTop );
#endif
      setFrameStyle( StyledPanel | Raised );
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
// From Qt's spacing
static const int motifItemHMargin       = 5;    // menu item hor text margin
static const int motifItemVMargin       = 4;    // menu item ver text margin
    KStyle *stylePtr = kapp->kstyle();
    if(!stylePtr)
        QMenuBar::drawContents(p);
    else{
        int i, x, y, nlitems;
        int dw = stylePtr->defaultFrameWidth();
        QFontMetrics fm = fontMetrics();
        stylePtr->drawKMenuBar(p, 0, 0, width(), height(), colorGroup(),
                               d->topLevel, NULL);

        for(i=0, nlitems=0, x=dw, y=dw; i < (int)mitems->count(); ++i, ++nlitems)
        {
            int h=0;
            int w=0;
            QMenuItem *mi = mitems->at(i);
            if(mi->pixmap()){
                w = QMAX(mi->pixmap()->width() + 4, QApplication::globalStrut().width());
                h = QMAX(mi->pixmap()->height() + 4, QApplication::globalStrut().height());
            }
            else if(!mi->text().isEmpty()){
                QString s = mi->text();
                w = fm.boundingRect(s).width() + 2*motifItemHMargin;
                w -= s.contains('&')*fm.width('&');
                w += s.contains(QString::fromLatin1("&&"))*fm.width('&');
                w = QMAX(w, QApplication::globalStrut().width());
                h = QMAX(fm.height() + motifItemVMargin, QApplication::globalStrut().height());
            }

	    else if (mi->widget()) {
		// For a Widget in the menubar space is needed.
		w = QMAX(mi->widget()->width() + 2, QApplication::globalStrut().width());
		h = QMAX(mi->widget()->height() +2, QApplication::globalStrut().height());
	    }

            if (!mi->isSeparator()){
                if (x + w + dw - width() > 0 && nlitems > 0 ){
                    nlitems = 0;
                    x = dw;
                    y += h;
                }
            }

            stylePtr->drawKMenuItem(p, x, y, w, h, mi->isEnabled()  ?
                                    palette().normal() : palette().disabled(),
                                    i == actItem,
                                    mi, NULL);

            x += w;
        }
    }
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
        QRect area = QApplication::desktop()->geometry();
        setGeometry(area.left(), -frameWidth()-2, area.width(), heightForWidth( area.width() ) );
#ifndef _WS_QWS_ //FIXME
        KWin::setStrut( winId(), 0, 0, height() - frameWidth() - 2, 0 );
#endif
    }
}

#include "kmenubar.moc"
