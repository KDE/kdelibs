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

#ifndef	MARGINPREVIEW_H
#define	MARGINPREVIEW_H

#include <qwidget.h>

void draw3DPage(QPainter *p, QRect r);

class MarginPreview : public QWidget
{
public:
	MarginPreview(QWidget *parent = 0, const char *name = 0);
	~MarginPreview();
	// note : unit -> points
	void setPageSize(int w, int h);
	void setMargins(int t, int b, int l, int r);
	void setNoPreview(bool on);
protected:
	void paintEvent(QPaintEvent *);
	void resizeEvent(QResizeEvent *);
private:
	int	width_, height_;
	int	top_, bottom_, left_, right_;
	QRect	box_;
	float	zoom_;
	bool nopreview_;
};

#endif
