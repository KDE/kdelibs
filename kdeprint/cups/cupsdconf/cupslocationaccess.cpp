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

#include "cupslocationaccess.h"

#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <klocale.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupslist.h"

CupsLocationAccess::CupsLocationAccess(QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	allow_ = new CupsListBox(this);
	deny_ = new CupsListBox(this);
	order_ = new QComboBox(this);
	order_->insertItem(i18n("Allow, Deny"));
	order_->insertItem(i18n("Deny, Allow"));
	order_->setCurrentItem(1);

	QLabel	*l1 = new QLabel(i18n("Allow from:"), this);
	QLabel	*l2 = new QLabel(i18n("Deny from:"), this);
	QLabel	*l3 = new QLabel(i18n("Order:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 6, 2, 10, 10);
	main_->addWidget(l1, 0, 0, Qt::AlignTop|Qt::AlignLeft);
	main_->addWidget(l2, 2, 0, Qt::AlignTop|Qt::AlignLeft);
	main_->addWidget(l3, 4, 0);
	main_->addMultiCellWidget(allow_, 0, 1, 1, 1);
	main_->addMultiCellWidget(deny_, 2, 3, 1, 1);
	main_->addWidget(order_, 4, 1);
	main_->setRowStretch(5, 1);
}

CupsLocationAccess::~CupsLocationAccess()
{
}

void CupsLocationAccess::loadLocation(CupsLocation *loc)
{
	if (loc->order_ != -1) order_->setCurrentItem(loc->order_);
	QStringList::Iterator	it;
	for (it=loc->allow_.begin();it!=loc->allow_.end();++it)
		allow_->insertItem(*it);
	for (it=loc->deny_.begin();it!=loc->deny_.end();++it)
		deny_->insertItem(*it);
}

void CupsLocationAccess::saveLocation(CupsLocation *loc)
{
	if (allow_->count() > 0 || deny_->count() > 0) loc->order_ = order_->currentItem();
	else loc->order_ = -1;
	loc->allow_.clear();
	for (int i=0;i<allow_->count();i++)
		loc->allow_.append(allow_->text(i));
	loc->deny_.clear();
	for (int i=0;i<deny_->count();i++)
		loc->deny_.append(deny_->text(i));
}

void CupsLocationAccess::setInfos(CupsdConf *conf)
{
	QWhatsThis::add(allow_, conf->comments_.toolTip(LOCAUTHALLOW_COMM));
	QWhatsThis::add(deny_, conf->comments_.toolTip(LOCAUTHDENY_COMM));
	QWhatsThis::add(order_, conf->comments_.toolTip(LOCAUTHORDER_COMM));
}
