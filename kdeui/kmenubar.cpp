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

#include "kmenubar.h"
#include <kstyle.h>
#include <kapp.h>

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

void KMenuBar::drawContents(QPainter *p)
{

    KStyle *stylePtr = kapp->kstyle();
    if(!stylePtr)
        QMenuBar::drawContents(p);
    else{
        int i, x, y, nlitems;
        bool popupshown;
        QFontMetrics fm = fontMetrics();
        stylePtr->drawKMenuBar(p, 0, 0, width(), height(), colorGroup(),
                               NULL);

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
            popupshown = mi->popup() ? mi->popup()->isVisible() : false;
            stylePtr->drawKMenuItem(p, x, y, w, h, mi->isEnabled()  ?
                                    palette().normal() : palette().disabled(),
                                    i == actItem && (hasFocus() || mouseActive
                                                     || popupshown),
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

#include "kmenubar.moc"
