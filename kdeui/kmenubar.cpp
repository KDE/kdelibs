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
#include <kglobal.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

// From Qt's spacing
static const int motifBarFrame          = 2;    // menu bar frame width

static const int motifBarHMargin        = 2;    // menu bar hor margin to item
#ifndef KTHEMESTYLE_CONSTANTS
static const int motifBarVMargin        = 1;    // menu bar ver margin to item
static const int motifItemFrame         = 2;    // menu item frame width

static const int motifItemHMargin       = 5;    // menu item hor text marginstatic const int motifItemVMargin       = 4;    // menu item ver text margin
static const int motifItemVMargin       = 4;    // menu item ver text margin

#define KTHEMESTYLE_CONSTANTS
#endif

class KMenuBar::KMenuBarPrivate
{
public:
    KMenuBarPrivate(QWidget *parent)
    {
      m_macMode = false;
      m_parent  = parent;
    }
    bool m_macMode;
    QWidget *m_parent;
};

KMenuBar::KMenuBar(QWidget *parent, const char *name)
  : QMenuBar(parent, name)
{
    d = new KMenuBarPrivate(parent);

    mouseActive = false;
    setFont(KGlobalSettings::menuFont());

    connect( kapp, SIGNAL(appearanceChanged()), this, SLOT(slotReadConfig()));

    slotReadConfig();
}

KMenuBar::~KMenuBar()
{
  delete d; d = 0;
}

void KMenuBar::setTopLevelMenu(bool top_level)
{
  d->m_macMode = top_level;
}

bool KMenuBar::isTopLevelMenu() const
{
  return d->m_macMode;
}

void KMenuBar::slotReadConfig()
{
  static QString grpKDE = QString::fromLatin1("KDE");
  static QString keyMac = QString::fromLatin1("macStyle");

  KConfig *config = KGlobal::config();
  KConfigGroupSaver saver( config, grpKDE );
  d->m_macMode = config->readBoolEntry( keyMac, false );
  d->m_macMode = FALSE; // disabled until somebody implements it
 // correctly. If you have questions about thxis, write to
 // ettrich@kde.org, subject "macMode".
}

void KMenuBar::drawContents(QPainter *p)
{
    KStyle *stylePtr = kapp->kstyle();
    if(!stylePtr)
        QMenuBar::drawContents(p);
    else{
        int i, x, y, nlitems;
        QFontMetrics fm = fontMetrics();
        stylePtr->drawKMenuBar(p, 0, 0, width(), height(), colorGroup(),
                               d->m_macMode, NULL);

        for(i=0, nlitems=0, x=2, y=2; i < (int)mitems->count(); ++i, ++nlitems)
        {
            int h=0;
            int w=0;
            QMenuItem *mi = mitems->at(i);
            if(mi->pixmap()){
                w = mi->pixmap()->width();
                h = mi->pixmap()->height();
            }
            else if(!mi->text().isEmpty()){
                QString s = mi->text();
                w = fm.boundingRect(s).width() + 2*motifItemHMargin;
                w -= s.contains('&')*fm.width('&');
                w += s.contains(QString::fromLatin1("&&"))*fm.width('&');
                h = fm.height() + motifItemVMargin;
            }

            if (!mi->isSeparator()){
                if (x + w + motifBarFrame - width() > 0 && nlitems > 0 ){
                    nlitems = 0;
                    x = motifBarFrame + motifBarHMargin;
                    y += h + motifBarHMargin;
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
  // we only do this if we are in Mac mode
  if ( d->m_macMode == false )
    return false;

  // we also demand that the object be our parent
  if ( obj != d->m_parent )
    return false;


  // finally, ensure that this is a Show event
  if ( ev->type() != QEvent::Show )
    return false;

  hide();

  QString title(d->m_parent->caption());
  title.append(" [menu]");
  setCaption( title );

  reparent(0, 0, mapToGlobal(QPoint(0,0)), false);
  XSetTransientForHint( qt_xdisplay(), winId(), d->m_parent->
                        topLevelWidget()->winId());
  KWin::setType( winId(), NET::Menu );
  
//###   QRect r =  KWM::windowRegion(KWM::currentDesktop());
  QRect r = qApp->desktop()->geometry();
  
  setGeometry(r.x(),(r.y()-1)<=0?-2:r.y()-1, r.width(),
              heightForWidth(r.width()) - 9);
  
  setFixedWidth(r.width());

  setFrameStyle( NoFrame );

  // Tell the window manager we want to be avoided.
  KWin::setStrut( winId(), 0, 0, height(), 0 );
  
  
  show();

  // in theory, this should reassign our top-level accelerators to our
  // parent.  in practice, this doesn't seem to do anything.  however,
  // I'm keeping it here since it *did* work in KDE 1.x and the
  // accelerators *don't* work now :-(
  QObjectList   *accelerators = queryList( "QAccel" );
  QObjectListIt it( *accelerators );
  for ( ; it.current(); ++it )
  {
    QObject *obj = it.current();
    removeEventFilter(obj);
    d->m_parent->installEventFilter(obj);
  }

  return false;
}
#include "kmenubar.moc"
