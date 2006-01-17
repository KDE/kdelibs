/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
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

#ifndef DRIVERVIEW_H
#define DRIVERVIEW_H

#include <qwidget.h>
#include <klistview.h>
#include <qmap.h>

#include <kdelibs_export.h>

class DrOptionView;
class DrMain;

class KDEPRINT_EXPORT DrListView : public KListView
{
public:
	DrListView(QWidget *parent = 0);
};

class KDEPRINT_EXPORT DriverView : public QWidget
{
	Q_OBJECT
public:
	DriverView(QWidget *parent = 0);
	~DriverView();

	void setDriver(DrMain*);
	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);
	void setAllowFixed(bool on);
	bool hasConflict() const 	{ return (m_conflict != 0); }

protected Q_SLOTS:
	void slotChanged();

private:
	DrListView	*m_view;
	DrOptionView	*m_optview;
	DrMain		*m_driver;
	int 		m_conflict;
};

#endif
