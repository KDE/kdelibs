/* This file is part of the KDE libraries
   Copyright (c) 1999 Daniel Duley <mosfet@kde.org>

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
#include <kstyle.h>
#include <klocale.h>
#include <kiconloader.h>
#include <qapplication.h>
#include <qdrawutil.h>

#ifndef INCLUDE_MENUITEM_DEF
#define INCLUDE_MENUITEM_DEF
#endif

#include <qmenudata.h>

// lame default implementations of KStyle drawing methods. These are meant
// to be overridden in new styles.

void KStyle::drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, KToolBarPos type,
                            QBrush *fill)
{
#if QT_VERSION < 300
    bool horizontal = type != Left && type != Right;
    int stipple_height;
    
    if(horizontal){
        qDrawShadePanel(p, x, y, w, h, g , false, 1, fill);
        p->setPen(g.light());
        stipple_height = 3;
        while(stipple_height < h-4) {
            p->drawPoint(x+1, y+stipple_height+1);
            p->drawPoint(x+4, y+stipple_height);
        stipple_height+=3;
        }
        p->setPen(g.dark());
        stipple_height = 4;
        while(stipple_height < h-4) {
            p->drawPoint(x+2, y+stipple_height+1);
            p->drawPoint(x+5, y+stipple_height);
            stipple_height+=3;
        }
    }
    else{
        qDrawShadePanel(p, x, y, w, h, g , false, 1, fill);
        p->setPen(g.light());
        stipple_height = 3;
        while(stipple_height < w-4) {
            p->drawPoint(x+stipple_height+1, y+1);
            p->drawPoint(x+stipple_height, y+4);
            stipple_height+=3;
        }
        p->setPen(g.dark());
        stipple_height = 4;
        while(stipple_height < w-4) {
            p->drawPoint(x+stipple_height+1, y+2);
            p->drawPoint(x+stipple_height, y+5);
            stipple_height+=3;
        }
    }
#endif
}

void KStyle::drawKToolBar(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, KToolBarPos pos,
                          QBrush *)
{
#if QT_VERSION < 300
    if(pos != Floating)
        qDrawShadePanel(p, x, y, w, h, g , false, 1);
#endif
}

void KStyle::drawKToolBarButton(QPainter *p, int x, int y, int w, int h, const
                                QColorGroup &g, bool sunken, bool raised,
                                bool enabled, bool popup,
                                KToolButtonType icontext, const QString& btext,
                                const QPixmap *pixmap,
                                QFont *font, QWidget *)
{
#if QT_VERSION < 300
    if ( sunken )
        qDrawWinButton(p, x, y, w, h, g, true );
    else if ( raised )
        qDrawWinButton(p, x, y, w, h, g, false );

    int dx, dy;

    if (icontext == Icon){ // icon only
        if (pixmap){
            dx = ( w - pixmap->width() ) / 2;
            dy = ( h - pixmap->height() ) / 2;
            if ( sunken )
            {
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
    }
    else if (icontext == IconTextRight){ // icon and text (if any)
        if (pixmap){
            dx = 1;
            dy = ( h - pixmap->height() ) / 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
        if (!btext.isNull()){
            int tf = AlignVCenter|AlignLeft;
            if (!enabled)
                p->setPen(g.dark());
            if (pixmap)
                dx= pixmap->width();
            else
                dx= 1;
            dy = 0;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(Qt::blue);
            p->drawText(x+dx, y+dy, w-dx, h, tf, btext);
        }
    }
    else if (icontext == Text){ // only text, even if there is a icon
        if (!btext.isNull()){
            int tf = AlignVCenter|AlignLeft;
            if (!enabled)
                p->setPen(g.dark());
            dx= 1;
            dy= 0;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(Qt::blue);
            p->drawText(x+dx, y+dy, w-dx, h, tf, btext);
        }
    }
    else if (icontext == IconTextBottom){
        if (pixmap){
            dx = (w - pixmap->width()) / 2;
            dy = 1;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
        if (!btext.isNull()){
            int tf = AlignBottom|AlignHCenter;
            if (!enabled)
                p->setPen(g.dark());
            dy= pixmap->height();
            dx = 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(Qt::blue);
            p->drawText(x, y, w, h-4, tf, btext);
        }
    }
    //#warning What about Icontext=3

    if (popup){
        if (enabled)
            qDrawArrow (p, DownArrow, WindowsStyle, false, w-5, h-5, 0, 0,
                        g, true);
        else
            qDrawArrow (p, DownArrow, WindowsStyle, false, w-5, h-5,
                        0, 0, g, false);
    }
#endif
}

void KStyle::drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, bool, QBrush *fill)
{
#if QT_VERSION < 300
    QBrush brush = fill ? *fill : g.brush(QColorGroup::Background);
    qDrawWinPanel(p, x, y, w, h, g, false, fill ? fill : &brush);
#endif
}

void KStyle::drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                           const QColorGroup &g, bool active, QMenuItem *mi,
                           QBrush *)
{
#if QT_VERSION < 300
    QColor btext = g.buttonText();
    if(active)
        qDrawShadePanel(p, x, y, w, h, g, false, 1);
    
    QApplication::style().drawItem(p, x, y, w, h,
                                   AlignCenter|ShowPrefix|DontClip|SingleLine,
                                   g, mi->isEnabled(), mi->pixmap(), mi->text(),
                                   -1, &btext );;
#endif
}

void KStyle::drawKProgressBlock(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, QBrush *fill)
{
#if QT_VERSION < 300
    p->fillRect(x, y, w, h, fill ? *fill : g.brush(QColorGroup::Highlight));
#endif
}

void KStyle::getKProgressBackground(const QColorGroup &g, QBrush &bg)
{
#if QT_VERSION < 300
    bg = g.brush(QColorGroup::Base);
#endif
}


void KStyle::drawKickerHandle(QPainter *, int, int, int, int,
                              const QColorGroup &, QBrush *)
{
    ;
}

void KStyle::drawKickerAppletHandle(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, QBrush *)
{
#if QT_VERSION < 300
    if (h > w){
        for(y+= 2; y < h - 2; y++)
        {
            p->setPen(g.light());
            p->drawPoint(0, y++);
            p->setPen(g.dark());
            p->drawPoint(1, y++);
            y++;
            p->setPen(g.light());
            p->drawPoint(3, y++);
            p->setPen(g.dark());
            p->drawPoint(4, y);
        }
    }
    else
    {
        for(x+=2; x < w - 2; x++)
        {
            p->setPen(g.light());
            p->drawPoint(x++, 0);
            p->setPen(g.dark());
            p->drawPoint(x++, 1);
            x++;
            p->setPen(g.light());
            p->drawPoint(x++, 3);
            p->setPen(g.dark());
            p->drawPoint(x, 4);
        }
    }
#endif        
}

void KStyle::drawKickerTaskButton(QPainter *p, int x, int y, int w, int h,
                                  const QColorGroup &g,
                                  const QString &text, bool sunken,
                                  QPixmap *pixmap, QBrush *)
{
#if QT_VERSION < 300
  int x2 = x + w - 1;
  int y2 = y + h - 1;

  p->fillRect(
    x + 1, y + 1,
    w - 2, h - 2,
    sunken ? g.brush(QColorGroup::Mid) : g.brush(QColorGroup::Button)
  );

  p->setPen(sunken ? Qt::black : g.light());
  p->drawLine(x, y, x2 - 1, y);
  p->drawLine(x, y,      x, y2 - 1);

  p->setPen(sunken ? g.midlight() : g.mid());
  p->drawLine(x  + 1, y2 - 1, x2 - 1, y2 - 1);
  p->drawLine(x2 - 1, y  + 1, x2 - 1, y2 - 1);

  p->setPen(sunken ? g.light() : Qt::black);
  p->drawLine(x, y2, x2, y2);
  p->drawLine(x2, y, x2, y2);

  if(text.isNull() && !pixmap)
      return;
  const int pxWidth = 20;
  int textPos = pxWidth;

  QRect br(buttonRect(x, y, w, h));

  if (sunken)
    p->translate(1,1);
  
  if ((NULL != pixmap) && (!pixmap->isNull())) {

    int dx = (pxWidth - pixmap->width())  / 2;
    int dy = (h - pixmap->height())       / 2;

    p->drawPixmap(br.x() + dx, dy, *pixmap);
  }

  QString s(text);

  static QString modStr =
    QString::fromUtf8("[") + i18n("modified") + QString::fromUtf8("]");
  
  int modStrPos = s.find(modStr);

  if (-1 != modStrPos) {

    // +1 because we include a space after the closing brace.
    s.remove(modStrPos, modStr.length()+1);

    QPixmap modPixmap = SmallIcon("modified");

    int dx = (pxWidth - modPixmap.width())  / 2;
    int dy = (h       - modPixmap.height()) / 2;

    p->drawPixmap(br.x() + textPos + dx, dy, modPixmap);

    textPos += pxWidth;
  }

  if (!s.isEmpty())
  {
    if (p->fontMetrics().width(s) > br.width() - textPos) {
    
      int maxLen = br.width() - textPos - p->fontMetrics().width("...");

      while ((!s.isEmpty()) && (p->fontMetrics().width(s) > maxLen))
        s.truncate(s.length() - 1);

      s.append("...");
    }

    p->setPen(sunken ? g.light() : g.buttonText());

    p->drawText(
      br.x() + textPos, -1,
      w - textPos, h,
      AlignLeft | AlignVCenter,
      s
    );
  }
#endif
}

void KStyle::getKickerBackground(int, int, Orientation,
                                 const QColorGroup &, QBrush &)
{
    ;
}


#include "kstyle.moc"

