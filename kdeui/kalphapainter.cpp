/* vi: ts=4 sts=2 sw=2
 *
 * This file is part of the KDE project, module kdeui.
 * Copyright (C) 2000 Antonio Larrosa <larrosa@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 * kalphapainter.cpp: A helper class to draw pixmaps with an alpha channel.
 */ 

#include "kalphapainter.h"
#include <kimageeffect.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>

bool KAlphaPainter::draw(QPainter *p, const QImage &icon, QImage &bg,
	int x, int y, bool copyOnBg, int bgx, int bgy)
{
  if (!icon.hasAlphaBuffer() || icon.depth()!=32)
  {
    if (p) p->drawImage(x,y,icon);
    return true;
  }

  int rx=x-bgx;
  int ry=y-bgy;

  if (copyOnBg)
  {
    if (!KImageEffect::blendOnLower(rx, ry, icon, bg)) return false;
    if (p) p->drawImage(bgx,bgy,bg); 
    // Note that in general, p is NULL when copyOnBg is true
  }
  else
  {
	QImage output;
	if (!KImageEffect::blend(rx, ry, icon, bg, output)) return false;
	if (p) p->drawImage(x,y,output);
  }

  return true;
}

bool KAlphaPainter::draw(QPainter *p, const QImage &icon, const QPixmap &bg,
	int x, int y, bool copyOnBg, int bgx, int bgy)
{
  if (copyOnBg) kdWarning() << "copyOnBg not implemented for Pixmap backgrounds";
  if (!icon.hasAlphaBuffer() || icon.depth()!=32)
  {
    if (p) p->drawImage(x,y,icon);
    return true;
  }

  QImage bgImg(bg.convertToImage());
  return draw( p, icon, bgImg, x, y, false, bgx, bgy);
}

bool KAlphaPainter::draw(QPainter *p, const QPixmap &icon, QImage &bg,
	int x, int y, bool copyOnBg, int bgx, int bgy )
{
  QImage *iconImg=KGlobal::iconLoader()->image(icon.serialNumber());
  if (!iconImg) 
  {
	if (p) p->drawPixmap(x,y,icon);
    if (copyOnBg) KImageEffect::paint(x-bgx,y-bgy,bg,icon.convertToImage());
    return true;
  }

  return draw( p, *iconImg, bg, x, y, copyOnBg, bgx, bgy);
}

bool KAlphaPainter::draw(QPainter *p, const QPixmap &icon, const QPixmap &bg,
	int x, int y, int bgx, int bgy)
{
  QImage *iconImg=KGlobal::iconLoader()->image(icon.serialNumber());
  if (!iconImg) 
  {
	if (p) p->drawPixmap(x,y,icon); return true;
  }

  QImage bgImg(bg.convertToImage());
  return draw( p, *iconImg, bgImg, x, y, false, bgx, bgy);
}
