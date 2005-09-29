/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef	IMAGEPOSITION_H
#define	IMAGEPOSITION_H

#include <qwidget.h>
#include <qstring.h>
#include <qpixmap.h>

class ImagePosition : public QWidget
{
public:
	enum PositionType {TopLeft,
			   Top,
			   TopRight,
			   Left,
			   Center,
			   Right,
			   BottomLeft,
			   Bottom,
			   BottomRight};

	ImagePosition(QWidget *parent = 0, const char *name = 0);
	~ImagePosition();

	void setPosition(const char *type = "center");
	void setPosition(PositionType type = Center);
	void setPosition(int horiz = 1, int vert = 1);
	PositionType position() const { return (PositionType)position_; }
	QString positionString() const;
	QSize sizeHint() const;

protected:
	void paintEvent(QPaintEvent *);

private:
	int	position_;
	QPixmap	pix_;
};

#endif
