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

#ifndef	CUPSLOCATIONGENERAL_H
#define	CUPSLOCATIONGENERAL_H

#include <qwidget.h>

class QLineEdit;
class QComboBox;
struct CupsLocation;
struct CupsdConf;

class CupsLocationGeneral : public QWidget
{
	Q_OBJECT;
public:
	CupsLocationGeneral(CupsdConf *conf, QWidget *parent = 0, const char *name = 0);
	~CupsLocationGeneral();

	void loadLocation(CupsLocation *loc);
	void saveLocation(CupsLocation *loc);
	bool isValid();
        void setInfos(CupsdConf*);

protected slots:
	void authClassChanged(int);
	void authTypeChanged(int);

private:
	QComboBox	*resource_;
	QComboBox	*authtype_;
	QComboBox	*authclass_;
	QLineEdit	*authgroupname_;
        QComboBox	*encryption_;

	CupsdConf	*conf_;	// only to have access to available resources.
};

#endif
