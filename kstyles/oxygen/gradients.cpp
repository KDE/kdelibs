/***************************************************************************
 *   Copyright (C) 2006-2007 by Thomas L�bking                             *
 *   thomas.luebking@web.de                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QCache>
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <cmath>

#include "gradients.h"

static QPixmap nullPix;
static Gradients::BgMode _mode;

static inline int
colorValue(const QColor &c) {
   int v = c.red();
   if (c.green() > v) v = c.green();
   if (c.blue() > v) v = c.blue();
   return v;
}

/* ========= MAGIC NUMBERS ARE COOL ;) =================
Ok, we want to cache the gradients, but unfortunately we have no idea about
what kind of gradients will be demanded in the future
Thus creating a 2 component map (uint for color and uint for size)
would be some overhead and cause a lot of unused space in the dictionaries -
while hashing by a string is stupid slow ;)

So we store all the gradients by a uint index
Therefore we substitute the alpha component (byte << 24) of the demanded color
with the demanded size
As this would limit the size to 255/256 pixels we'll be a bit sloppy,
depending on the users resolution (e.g. use 0 to store a gradient with 2px,
usefull for demand of 1px or 2px) and shift the index
(e.g. gradients from 0 to 6 px size will hardly be needed -
maybe add statistics on this)
So the handled size is actually demandedSize + (demandedSize % sizeSloppyness),
beeing at least demanded size and the next sloppy size above at max
====================================================== */
static inline uint
hash(int size, const QColor &c, int *sloppyAdd) {
   
   uint magicNumber = 0;
   int sizeSloppyness = 1, frameBase = 0, frameSize = 20;
   while ((frameBase += frameSize) < size) {
      ++sizeSloppyness;
      frameSize += 20;
   }
      
   frameBase -=frameSize; frameSize -= 20;
   
   *sloppyAdd = size % sizeSloppyness;
   if (!*sloppyAdd)
      *sloppyAdd = sizeSloppyness;

   // first 11 bits to store the size, remaining 21 bits for the color (7bpc)
   magicNumber =  (((frameSize + (size - frameBase)/sizeSloppyness) & 0xff) << 21) |
      (((c.red() >> 1) & 0x7f) << 14) |
      (((c.green() >> 1) & 0x7f) << 7 ) |
      ((c.blue() >> 1) & 0x7f);
   
   return magicNumber;
}

static QPixmap*
newPix(int size, Qt::Orientation o, QPoint *start, QPoint *stop) {
   QPixmap *pix;
   if (o == Qt::Horizontal) {
      pix = new QPixmap(size, 32);
      *start = QPoint(0,32); *stop = QPoint(pix->width(),32);
   }
   else {
      pix = new QPixmap(32, size);
      *start = QPoint(32, 0); *stop = QPoint(32, pix->height());
   }
   return pix;
}


static inline QPixmap*
simpleGradient(const QColor &c, int size, Qt::Orientation o) {
   QPoint start, stop;
   QPixmap *pix = newPix(size, o, &start, &stop);
   QLinearGradient lg(start, stop);
   lg.setColorAt(0, c.light(112));
   lg.setColorAt(1, c.dark(110));
   QPainter p(pix); p.fillRect(pix->rect(), lg); p.end();
   return pix;
}

static inline QPixmap *
sunkenGradient(const QColor &c, int size, Qt::Orientation o) {
   QPoint start, stop;
   QPixmap *pix = newPix(size, o, &start, &stop);
   QLinearGradient lg(start, stop);
   lg.setColorAt(0, c.dark(110));
   lg.setColorAt(1, c.light(112));
   QPainter p(pix); p.fillRect(pix->rect(), lg); p.end();
   return pix;
}

static inline QPixmap *
buttonGradient(const QColor &c, int size, Qt::Orientation o) {
   int h,s,v, inc, dec;
   c.getHsv(&h,&s,&v);
   
   // calc difference
   inc = 15; dec = 6;
   if (v+15 > 255) {
      inc = 255-v; dec += (15-inc);
   }
   QPoint start, stop;
   QPixmap *pix = newPix(size, o, &start, &stop);
   QLinearGradient lg(start, stop);
   QColor ic; ic.setHsv(h,s,v+inc);
   lg.setColorAt(0, ic);
   ic.setHsv(h,s,v-dec);
   lg.setColorAt(0.75, ic);
   QPainter p(pix); p.fillRect(pix->rect(), lg); p.end();
   return pix;
}

inline static void
gl_ssColors(const QColor &c, QColor *bb, QColor *dd, bool glass = false) {
   
   int h,s,v, ch,cs,cv, delta, add;
   
   c.getHsv(&h,&s,&v);

   // calculate the variation
   add = ((180-qGray(c.rgb()))>>1);
   if (add < 0) add = -add/2;
   if (glass)
      add = add>>4;

   // the brightest color (top)
   cv = v+27+add;
   if (cv > 255) {
      delta = cv-255; cv = 255;
      cs = s - delta; if (cs < 0) cs = 0;
      ch = h - delta/6; if (ch < 0) ch = 360+ch;
   }
   else {
      ch = h; cs = s;
   }
   bb->setHsv(ch,cs,cv);
   
   // the darkest color (lower center)
   cv = v - 14-add; if (cv < 0) cv = 0;
   cs = s*13/7; if (cs > 255) cs = 255;
   dd->setHsv(h,cs,cv);
}

static inline QPixmap *
gl_ssGradient(const QColor &c, int size, Qt::Orientation o, bool glass = false) {
   QColor bb,dd; // b = d = c;
   gl_ssColors(c, &bb, &dd, glass);
   QPoint start, stop;
   QPixmap *pix = newPix(size, o, &start, &stop);
   QLinearGradient lg(start, stop);
   lg.setColorAt(0,bb); lg.setColorAt(0.5,c);
   lg.setColorAt(0.5, dd); lg.setColorAt(glass ? 1 : .90, bb);
   QPainter p(pix); p.fillRect(pix->rect(), lg); p.end();
   return pix;
}

static inline QPixmap *
rGlossGradient(const QColor &c, int size) {
   QColor bb,dd; // b = d = c;
   gl_ssColors(c, &bb, &dd);
   QPixmap *pix = new QPixmap(size, size);
   QRadialGradient rg(2*pix->width()/3, pix->height(), pix->height());
   rg.setColorAt(0,c); rg.setColorAt(0.8,dd);
   rg.setColorAt(0.8, c); rg.setColorAt(1, bb);
   QPainter p(pix); p.fillRect(pix->rect(), rg); p.end();
   return pix;
}

static inline uint costs(QPixmap *pix) {
   return ((pix->width()*pix->height()*pix->depth())>>3);
}

typedef QCache<uint, QPixmap> PixmapCache;
static PixmapCache gradients[2][Gradients::TypeAmount];
static PixmapCache _btnAmbient, _tabShadow, _groupLight;
static PixmapCache _bg[2];

const QPixmap&
Gradients::pix(const QColor &c, int size, Qt::Orientation o, Gradients::Type type) {
   // validity check
   if (size <= 0) {
      qWarning("NULL Pixmap requested, size was %d",size);
      return nullPix;
   }
   else if (size > 105883) { // this is where our dictionary reaches - should be enough for the moment ;)
      qWarning("gradient with more than 105883 steps requested, returning NULL pixmap");
      return nullPix;
   }
   
   // very dark colors won't make nice buttons =)
   QColor iC = c;
   int v = colorValue(c);
   if (v < 80) {
      int h,s;
      c.getHsv(&h,&s,&v);
      iC.setHsv(h,s,80);
   }

   // hash 
   int sloppyAdd = 1;
   uint magicNumber = hash(size, iC, &sloppyAdd);

   PixmapCache *cache = &gradients[o == Qt::Horizontal][type];
   QPixmap *pix = cache->object(magicNumber);
   if (pix)
      return *pix;
   
   // no cache entry found, so let's create one
   size += sloppyAdd; // rather to big than to small ;)
   switch (type) {
   case Gradients::Button:
      pix = buttonGradient(iC, size, o);
      break;
   case Gradients::Glass:
      pix = gl_ssGradient(iC, size, o, true);
      break;
   case Gradients::Simple:
   default:
      pix = simpleGradient(iC, size, o);
      break;
   case Gradients::Sunken:
      pix = sunkenGradient(iC, size, o);
      break;
   case Gradients::Gloss:
      pix = gl_ssGradient(iC, size, o);
      break;
   case Gradients::RadialGloss:
      pix = rGlossGradient(iC, size);
      break;
   }

   // cache for later
   cache->insert(magicNumber, pix, costs(pix));
   return *pix;
}

const QPixmap &Gradients::light(int height) {
   if (height <= 0) {
      qWarning("NULL Pixmap requested, height was %d",height);
      return nullPix;
   }
   QPixmap *pix = _groupLight.object(height);
   if (pix)
      return *pix;
      
   pix = new QPixmap(32, height); //golden mean relations
   pix->fill(Qt::transparent);
   QPoint start(0,0), stop(0,height);
   QLinearGradient lg(start, stop);
   lg.setColorAt(0, QColor(255,255,255,134));
   lg.setColorAt(1, QColor(255,255,255,0));
   QPainter p(pix); p.fillRect(pix->rect(), lg); p.end();
   
   // cache for later ;)
   _groupLight.insert(height, pix, costs(pix));
   return *pix;
}

const QPixmap &Gradients::ambient(int height) {
   if (height <= 0) {
      qWarning("NULL Pixmap requested, height was %d",height);
      return nullPix;
   }

   QPixmap *pix = _btnAmbient.object(height);
   if (pix)
      return *pix;

   pix = new QPixmap(16*height/9,height); //golden mean relations
   pix->fill(Qt::transparent);
   QLinearGradient lg(QPoint(pix->width(), pix->height()),
                      QPoint(pix->width()/2,pix->height()/2));
   lg.setColorAt(0, QColor(255,255,255,0));
   lg.setColorAt(0.2, QColor(255,255,255,128));
   lg.setColorAt(1, QColor(255,255,255,0));
   QPainter p(pix); p.fillRect(pix->rect(), lg); p.end();

   // cache for later ;)
   _btnAmbient.insert(height, pix, costs(pix));
   return *pix;
}

const QPixmap &Gradients::shadow(int height, bool bottom) {
   if (height <= 0) {
      qWarning("NULL Pixmap requested, height was %d",height);
      return nullPix;
   }
   uint val = height + bottom*0x80000000;
   QPixmap *pix = _tabShadow.object(val);
   if (pix)
      return *pix;
      
   pix = new QPixmap(height/3,height);
   pix->fill(Qt::transparent);
   
   float hypo = sqrt(pow(pix->width(),2)+pow(pix->height(),2));
   float cosalpha = (float)(pix->height())/hypo;
   QPoint p1, p2;
   if (bottom) {
      p1 = QPoint(0, 0);
      p2 = QPoint((int)(pix->width()*pow(cosalpha, 2)),
                  (int)(pow(pix->width(), 2)*cosalpha/hypo));
   }
   else {
      p1 = QPoint(0, pix->height());
      p2 = QPoint((int)(pix->width()*pow(cosalpha, 2)),
                  (int)pix->height() - (int)(pow(pix->width(), 2)*cosalpha/hypo));
   }
   QLinearGradient lg(p1, p2);
   lg.setColorAt(0, QColor(0,0,0,75));
   lg.setColorAt(1, QColor(0,0,0,0));
   QPainter p(pix); p.fillRect(pix->rect(), lg); p.end();
   
   // cache for later ;)
   _tabShadow.insert(val, pix, costs(pix));
   return *pix;
}

const QPixmap &Gradients::bg(const QColor &c, bool other) {
   QPixmap *pix = _bg[other].object(c.rgb());
   if (pix)
      return *pix;
   QLinearGradient lg;
   switch (_mode) {
   case BevelV: {
      pix = new QPixmap(32, 256);
      lg = QLinearGradient(QPoint(0,0), QPoint(0,256));
      if (other) {
         lg.setColorAt(0, c); lg.setColorAt(1, c.dark(110));
      }
      else {
         lg.setColorAt(0, c.light(108)); lg.setColorAt(1, c);
      }
      break;
   }
   case BevelH:
      pix = new QPixmap(256, 32);
      lg = QLinearGradient(QPoint(0,0), QPoint(256, 0));
      if (other) {
         lg.setColorAt(0, c); lg.setColorAt(1, c.dark(108));
      }
      else {
         lg.setColorAt(0, c.dark(108)); lg.setColorAt(1, c);
      }
      break;
   case LightV:
      pix = new QPixmap(32, 512);
      lg = QLinearGradient(QPoint(0,0), QPoint(0,512));
      lg.setColorAt(0, c); lg.setColorAt(0.5, c.light(108)); lg.setColorAt(1, c);
      break;
   case LightH:
      pix = new QPixmap(512, 32);
      lg = QLinearGradient(QPoint(0,0), QPoint(512,0));
      lg.setColorAt(0, c); lg.setColorAt(0.5, c.light(108)); lg.setColorAt(1, c);
      break;
   }
   QPainter p(pix); p.fillRect(pix->rect(), lg); p.end();
   _bg[other].insert(c.rgb(), pix, costs(pix));
   return *pix;
}

void Gradients::init(BgMode mode) {
   _mode = mode;
   for (int i = 0; i < 2; ++i) {
      _bg[i].setMaxCost( 128<<10 );
      for (int j = 0; j < Gradients::TypeAmount; ++j)
         gradients[i][j].setMaxCost( 1024<<10 );
   }
   _btnAmbient.setMaxCost( 64<<10 );
   _tabShadow.setMaxCost( 64<<10 );
   _groupLight.setMaxCost( 256<<10 );
}

void Gradients::wipe() {
   for (int i = 0; i < 2; ++i)
      for (int j = 0; j < Gradients::TypeAmount; ++j)
         gradients[i][j].clear();
   _btnAmbient.clear();
   _tabShadow.clear();
   _groupLight.clear();
}
