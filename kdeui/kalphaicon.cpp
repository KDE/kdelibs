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
#include <kdebug.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>

bool KAlphaIcon::draw(QPainter *p, const QImage &icon, QImage &bg,
	int x, int y, bool copyOnBg)
{
  if (!icon.hasAlphaBuffer() || icon.depth()!=32)
  {
    p->drawImage(x,y,icon);
    return true;
  }

  QImage output;

  if (!KImageEffect::blend(x, y, icon, bg, output)) return false;

  p->drawImage(x,y,output);

  if (copyOnBg) 
  {
	register short *i, *o;
	register int row=output.height()-1;
	register int col, col2;
	int w = output.width();

	do {

	  i = reinterpret_cast<short *>(output.scanLine(row));
	  o = reinterpret_cast<short *>(bg.scanLine(row+y));

	  col = w << 1;
	  col2 = (w+x) << 1;

	  do {

		--col; --col2;
		o[col2] = i[col];

		--col; --col2;
		o[col2] = i[col];

	  } while (col);

	} while (row--);
  }
  return true;
}

bool KAlphaIcon::draw(QPainter *p, const QImage &icon, const QPixmap &bg,
	int x, int y, bool copyOnBg)
{
  if (copyOnBg) kdWarning() << "copyOnBg not implemented for Pixmap backgrounds";
  if (!icon.hasAlphaBuffer() || icon.depth()!=32)
  {
    p->drawImage(x,y,icon);
    return true;
  }

  QImage bgImg(bg.convertToImage());
  return draw( p, icon, bgImg, x, y, false);
}

bool KAlphaIcon::draw(QPainter *p, const QPixmap &icon, QImage &bg,
	int x, int y, bool copyOnBg)
{
  QImage iconImg(icon.convertToImage());
  return draw( p, iconImg, bg, x, y, copyOnBg);
}
