/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "imageposition.h"

#include <qpainter.h>
#include <kstandarddirs.h>

static void draw3DPage(QPainter *p, QRect r)
{
	// draw white page
	p->fillRect(r,Qt::white);
	// draw 3D border
	p->setPen(Qt::black);
	p->moveTo(r.left(),r.bottom());
	p->lineTo(r.right(),r.bottom());
	p->lineTo(r.right(),r.top());
	p->setPen(Qt::darkGray);
	p->lineTo(r.left(),r.top());
	p->lineTo(r.left(),r.bottom());
	p->setPen(Qt::gray);
	p->moveTo(r.left()+1,r.bottom()-1);
	p->lineTo(r.right()-1,r.bottom()-1);
	p->lineTo(r.right()-1,r.top()+1);
}

ImagePosition::ImagePosition(QWidget *parent, const char *name)
	: QWidget(parent,name)
{
	position_ = Center;
	setMinimumSize(sizeHint());
	setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
	pix_.load(locate("data", "kdeprint/preview-mini.png"));
}

ImagePosition::~ImagePosition()
{
}

void ImagePosition::setPosition(const char *type)
{
	int	pos(Center);
	if (strcmp(type,"top-left") == 0) pos = TopLeft;
	else if (strcmp(type,"top") == 0) pos = Top;
	else if (strcmp(type,"top-right") == 0) pos = TopRight;
	else if (strcmp(type,"left") == 0) pos = Left;
	else if (strcmp(type,"center") == 0) pos = Center;
	else if (strcmp(type,"right") == 0) pos = Right;
	else if (strcmp(type,"bottom-left") == 0) pos = BottomLeft;
	else if (strcmp(type,"bottom") == 0) pos = Bottom;
	else if (strcmp(type,"bottom-right") == 0) pos = BottomRight;
	setPosition((PositionType)pos);
}

void ImagePosition::setPosition(PositionType type)
{
	if (position_ != type) {
		position_ = type;
		update();
	}
}

void ImagePosition::setPosition(int horiz, int vert)
{
	int	type = vert*3+horiz;
	setPosition((PositionType)type);
}

QString ImagePosition::positionString() const
{
	switch (position_) {
	   case TopLeft: return "top-left";
	   case Top: return "top";
	   case TopRight: return "top-right";
	   case Left: return "left";
	   case Center: return "center";
	   case Right: return "right";
	   case BottomLeft: return "bottom-left";
	   case Bottom: return "bottom";
	   case BottomRight: return "bottom-right";
	}
	return "center";
}

void ImagePosition::paintEvent(QPaintEvent*)
{
	int	horiz, vert, x, y;
	int	margin = 5;
	int	pw(width()), ph(height()), px(0), py(0);

	if (pw > ((ph * 3) / 4))
	{
		pw = (ph * 3) / 4;
		px = (width() - pw) / 2;
	}
	else
	{
		ph = (pw * 4) / 3;
		py = (height() - ph) / 2;
	}
	QRect	page(px, py, pw, ph), img(0, 0, pix_.width(), pix_.height());

	// compute img position
	horiz = position_%3;
	vert = position_/3;
	switch (horiz) {
	   case 0: x = page.left()+margin; break;
	   default:
	   case 1: x = (page.left()+page.right()-img.width())/2; break;
	   case 2: x = page.right()-margin-img.width(); break;
	}
	switch (vert) {
	   case 0: y = page.top()+margin; break;
	   default:
	   case 1: y = (page.top()+page.bottom()-img.height())/2; break;
	   case 2: y = page.bottom()-margin-img.height(); break;
	}
	img.moveTopLeft(QPoint(x,y));

	// draw page
	QPainter	p(this);
	draw3DPage(&p,page);

	// draw img
	/*p.setPen(darkRed);
	p.drawRect(img);
	p.drawLine(img.topLeft(),img.bottomRight());
	p.drawLine(img.topRight(),img.bottomLeft());*/
	p.drawPixmap(x, y, pix_);

	p.end();
}

QSize ImagePosition::sizeHint() const
{
	return QSize(60, 80);
}
