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
#include <kdebug.h>
#include <qpainter.h>
#include <qcursor.h>

#define	A4_WIDTH	595
#define	A4_HEIGHT	842
#define	A4_TOP		36
#define	A4_BOTTOM	806
#define	A4_LEFT		18
#define	A4_RIGHT	577

#define	SCALE(d,z)	((int)(float(d)*z+0.5))
#define	UNSCALE(d,z)	((int)(float(d)/z+0.5))

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
	state_ = Fixed;
	oldpos_ = -1;
	symetric_ = false;

	setMouseTracking(true);
}

MarginPreview::~MarginPreview()
{
}

void MarginPreview::setPageSize(int w, int h)
{
	setNoPreview(w <= 0 && h <= 0);
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

void MarginPreview::setSymetric(bool on)
{
	symetric_ = on;
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
		p.setPen(DotLine);
		int	m = box_.left()+SCALE(left_,zoom_);
		margbox_.setLeft(m+1);
		p.drawLine(m,box_.top(),m,box_.bottom());
		m = box_.left()+SCALE(right_,zoom_);
		margbox_.setRight(m-1);
		p.drawLine(m,box_.top(),m,box_.bottom());
		m = box_.top()+SCALE(top_,zoom_);
		margbox_.setTop(m+1);
		p.drawLine(box_.left(),m,box_.right(),m);
		m = box_.top()+SCALE(bottom_,zoom_);
		margbox_.setBottom(m-1);
		p.drawLine(box_.left(),m,box_.right(),m);

		// fill usefull area
		p.fillRect(margbox_,QColor(220,220,220));
	}
}

void MarginPreview::setNoPreview(bool on)
{
	nopreview_ = on;
	update();
}

// 0: nothing
// 1: top
// 2: bottom
// 3: left
// 4: right
int MarginPreview::locateMouse(const QPoint& p)
{
	int	tol = 2;
	if (p.x() <= margbox_.left()+tol && p.x() >= margbox_.left()-tol)
		return LMoving;
	else if (p.x() <= margbox_.right()+tol && p.x() >= margbox_.right()-tol)
		return RMoving;
	else if (p.y() <= margbox_.top()+tol && p.y() >= margbox_.top()-tol)
		return TMoving;
	else if (p.y() <= margbox_.bottom()+tol && p.y() >= margbox_.bottom()-tol)
		return BMoving;
	else
		return 0;
}

void MarginPreview::mouseMoveEvent(QMouseEvent *e)
{
	if (nopreview_ || state_ == Fixed)
		return;
	int	pos = locateMouse(e->pos());
	if (state_ == None && e->button() == Qt::NoButton)
	{
		switch (pos)
		{
			case 1:
			case 2:
				setCursor(Qt::SplitHCursor);
				break;
			case 3:
			case 4:
				setCursor(Qt::SplitVCursor);
				break;
			default:
				setCursor(Qt::ArrowCursor);
				break;
		}
	}
	else if (state_ > None)
	{
		int newpos = -1;
		switch (state_)
		{
			case TMoving:
				newpos = QMIN(QMAX(e->pos().y(), box_.top()), (symetric_ ? (box_.top()+box_.bottom())/2 : margbox_.bottom()+1));
				break;
			case BMoving:
				newpos = QMIN(QMAX(e->pos().y(), (symetric_? (box_.top()+box_.bottom()+1)/2 : margbox_.top()-1)), box_.bottom());
				break;
			case LMoving:
				newpos = QMIN(QMAX(e->pos().x(), box_.left()), (symetric_ ? (box_.left()+box_.right())/2 : margbox_.right()+1));
				break;
			case RMoving:
				newpos = QMIN(QMAX(e->pos().x(), (symetric_ ? (box_.left()+box_.right()+1)/2 : margbox_.left()-1)), box_.right());
				break;
		}
		if (newpos != oldpos_)
		{
			QPainter	p(this);
			p.setRasterOp(Qt::XorROP);
			p.setPen(gray);
			for (int i=0; i<2; i++, oldpos_ = newpos)
			{
				if (oldpos_ >= 0)
					drawTempLine(&p);
			}
		}
	}
}

void MarginPreview::drawTempLine(QPainter *p)
{
	if (state_ >= LMoving)
	{
		p->drawLine(oldpos_, box_.top(), oldpos_, box_.bottom());
		if (symetric_)
		{
			int	mirror = box_.left()+box_.right()-oldpos_;
			p->drawLine(mirror, box_.top(), mirror, box_.bottom());
		}
	}
	else
	{
		p->drawLine(box_.left(), oldpos_, box_.right(), oldpos_);
		if (symetric_)
		{
			int	mirror = box_.top()+box_.bottom()-oldpos_;
			p->drawLine(box_.left(), mirror, box_.right(), mirror);
		}
	}
}

void MarginPreview::mousePressEvent(QMouseEvent *e)
{
	if (e->button() != Qt::LeftButton || state_ != None)
		return;
	int	mpos = locateMouse(e->pos());
	if (mpos)
	{
		state_ = mpos;
	}
}

void MarginPreview::mouseReleaseEvent(QMouseEvent *e)
{
	if (state_ > None)
	{
		QPainter	p(this);
		p.setRasterOp(Qt::XorROP);
		p.setPen(gray);
		if (oldpos_ >= 0)
		{
			drawTempLine(&p);
			if (e)
			{
				int	val = 0, st(state_);
				if (symetric_ && (st == BMoving || st == RMoving))
					st--;
				switch (st)
				{
					case TMoving:
						val = top_ = UNSCALE(oldpos_-box_.top(), zoom_);
						if (symetric_)
							bottom_ = height_-top_;
						break;
					case BMoving:
						bottom_ = UNSCALE(oldpos_-box_.top(), zoom_);
						val = UNSCALE(box_.bottom()-oldpos_, zoom_);
						break;
					case LMoving:
						val = left_ = UNSCALE(oldpos_-box_.left(), zoom_);
						if (symetric_)
							right_ = width_-left_;
						break;
					case RMoving:
						right_ = UNSCALE(oldpos_-box_.left(), zoom_);
						val = UNSCALE(box_.right()-oldpos_, zoom_);
						break;
				}
				update();
				emit marginChanged(st, val);
			}
		}
		state_ = 0;
		oldpos_ = -1;
	}
}

void MarginPreview::enableRubberBand(bool on)
{
	if (on && state_ == Fixed)
		state_ = None;
	else if (!on && state_ > Fixed)
	{
		mouseReleaseEvent(NULL);
		state_ = Fixed;
	}
}

#include "marginpreview.moc"
