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

#ifndef CUPSDOPTION_H
#define	CUPSDOPTION_H

#include <qwidget.h>

class QCheckBox;
class QHBoxLayout;
class QFrame;

class CupsdOption : public QWidget
{
	Q_OBJECT;
public:
	CupsdOption(QWidget *parent = 0, const char *name = 0);
	~CupsdOption();

	QSize sizeHint() const;
	void setDefault(bool on = true);
	bool isDefault() const;

protected slots:
	void checkClicked();

protected:
	void childEvent(QChildEvent *e);

private:
	QCheckBox	*cb_;
	QWidget		*cbw_;
	QHBoxLayout		*layout_;
	QFrame		*fr_;

	QWidget		*widget_;
};

#endif
