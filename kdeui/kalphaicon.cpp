/* vi: ts=4 sts=2 sw=2
 *
 * This file is part of the KDE project, module kdeui.
 * Copyright (C) 2000 Antonio Larrosa <larrosa@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 * kalphaicon.cpp: A helper class to draw icons with an alpha channel.
 */ 

#include "kalphaicon.h"
#include <kimageeffect.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>

bool KAlphaIcon::draw(QPainter *p, const QImage &icon, const QImage &bg, int x, int y)
{
  if (!icon.hasAlphaBuffer() || icon.depth()!=32)
  {
    p->drawImage(x,y,icon);
    return;
  }

  QImage output;

  if (!KImageEffect::blend(x, y, icon, bg, output)) return false;

  p->drawImage(0,0,output);

  return true;
}

bool KAlphaIcon::draw(QPainter *p, const QImage &icon, const QPixmap &bg, int x, int y)
{
  if (!icon.hasAlphaBuffer() || icon.depth()!=32)
  {
    p->drawImage(x,y,icon);
    return;
  }

  QImage bgImg(bg.convertToImage());
  return draw( p, icon, bgImg, x, y);
}
