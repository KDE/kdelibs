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

#include "marginpreview.h"

#include <klocale.h>
#include <qpainter.h>

#define	A4_WIDTH	595
#define	A4_HEIGHT	842
#define	A4_TOP		36
#define	A4_BOTTOM	806
#define	A4_LEFT		18
#define	A4_RIGHT	577

#define	SCALE(d,z)	((int)(float(d)*z))

void draw3DPage(QPainter *p, QRect r)
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

MarginPreview::MarginPreview(QWidget *parent, const char *name)
	: QWidget(parent,name)
{
	width_ = A4_WIDTH;
	height_ = A4_HEIGHT;
	top_ = A4_TOP;
	bottom_ = A4_BOTTOM;
	left_ = A4_LEFT;
	right_ = A4_RIGHT;
	nopreview_ = false;

	box_ = rect();
	zoom_ = 1.0;
}

MarginPreview::~MarginPreview()
{
}

void MarginPreview::setPageSize(int w, int h)
{
	// do not change relative margins when changing page size !!
	int	old_b(height_-bottom_), old_r(width_-right_);
	width_ = w;
	height_ = h;
	resizeEvent(NULL);
	setMargins(top_,old_b,left_,old_r);
	update();
}

void MarginPreview::setMargins(int t, int b, int l, int r)
{
	top_ = t;
	left_ = l;
	bottom_ = height_-b;
	right_ = width_-r;
	update();
}

void MarginPreview::resizeEvent(QResizeEvent *)
{
	if (float(width_)/height_ > float(width())/height())
	{
		zoom_ = float(width()-3)/width_;
		box_.setLeft(1);
		box_.setRight(width()-3);
		int	m = (height()-3-SCALE(height_,zoom_))/2;
		box_.setTop(m+1);
		box_.setBottom(height()-m-3);
	}
	else
	{
		zoom_ = float(height()-3)/height_;
		box_.setTop(1);
		box_.setBottom(height()-3);
		int	m = (width()-3-SCALE(width_,zoom_))/2;
		box_.setLeft(m+1);
		box_.setRight(width()-m-3);
	}
}

void MarginPreview::paintEvent(QPaintEvent *)
{
	QPainter	p(this);

	QRect	pagebox(QPoint(box_.left()-1,box_.top()-1),QPoint(box_.right()+2,box_.bottom()+2));

	if (nopreview_)
	{
		p.drawText(pagebox,AlignCenter,i18n("No preview available"));
	}
	else
	{
		draw3DPage(&p,pagebox);

		// draw margins
		QRect	margbox;
		p.setPen(DashLine);
		int	m = box_.left()+SCALE(left_,zoom_);
		margbox.setLeft(m+1);
		p.drawLine(m,box_.top(),m,box_.bottom());
		m = box_.left()+SCALE(right_,zoom_);
		margbox.setRight(m-1);
		p.drawLine(m,box_.top(),m,box_.bottom());
		m = box_.top()+SCALE(top_,zoom_);
		margbox.setTop(m+1);
		p.drawLine(box_.left(),m,box_.right(),m);
		m = box_.top()+SCALE(bottom_,zoom_);
		margbox.setBottom(m-1);
		p.drawLine(box_.left(),m,box_.right(),m);

		// fill usefull area
		p.fillRect(margbox,QColor(220,220,220));
	}
}

void MarginPreview::setNoPreview(bool on)
{
	nopreview_ = on;
	update();
}
