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

#ifndef	CUPSLOCATIONACCESS_H
#define	CUPSLOCATIONACCESS_H

#include <qwidget.h>

class CupsListBox;
class QComboBox;
struct CupsLocation;
struct CupsdConf;

class CupsLocationAccess : public QWidget
{
public:
	CupsLocationAccess(QWidget *parent = 0, const char *name = 0);
	~CupsLocationAccess();

	void loadLocation(CupsLocation *loc);
	void saveLocation(CupsLocation *loc);
        void setInfos(CupsdConf*);

private:
	CupsListBox	*allow_;
	CupsListBox	*deny_;
	QComboBox	*order_;
};

#endif
