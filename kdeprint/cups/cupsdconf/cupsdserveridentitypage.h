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

#ifndef CUPSDSERVERIDENTITYPAGE_H
#define CUPSDSERVERIDENTITYPAGE_H

#include "cupsdpage.h"

class QLineEdit;
class CupsdOption;

class CupsdServerIdentityPage : public CupsdPage
{
public:
	CupsdServerIdentityPage(QWidget *parent = 0, const char *name = 0);
	~CupsdServerIdentityPage();

	bool loadConfig(CupsdConf *conf, QString& msg);
	bool saveConfig(CupsdConf *conf, QString& msg);
	void setDefaults();
        void setInfos(CupsdConf*);

private:
	QLineEdit	*servername_;
	QLineEdit	*serveradmin_;
	QLineEdit	*user_;
	QLineEdit	*group_;
	QLineEdit	*remroot_;

	CupsdOption	*opt_[5];
};

#endif
