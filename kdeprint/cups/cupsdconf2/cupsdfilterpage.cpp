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

#include "cupsdfilterpage.h"
#include "cupsdconf.h"
#include "sizewidget.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qcombobox.h>

#include <klocale.h>
#include <knuminput.h>

CupsdFilterPage::CupsdFilterPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	setPageLabel(i18n("Filter"));
	setHeader(i18n("Filter settings"));
	setPixmap("filter");

	user_ = new QLineEdit(this);
	group_ = new QLineEdit(this);
	ripcache_ = new SizeWidget(this);
	filterlimit_ = new KIntNumInput(this);

	filterlimit_->setRange(0, 1000, 1, true);
	filterlimit_->setSpecialValueText(i18n("Unlimited"));
	filterlimit_->setSteps(1, 10);

	QLabel *l1 = new QLabel(i18n("User:"), this);
	QLabel *l2 = new QLabel(i18n("Group:"), this);
	QLabel *l3 = new QLabel(i18n("RIP cache:"), this);
	QLabel *l4 = new QLabel(i18n("Filter limit:"), this);

	QGridLayout	*m1 = new QGridLayout(this, 5, 2, 10, 7);
	m1->setRowStretch(4, 1);
	m1->setColStretch(1, 1);
	m1->addWidget(l1, 0, 0, Qt::AlignRight);
	m1->addWidget(l2, 1, 0, Qt::AlignRight);
	m1->addWidget(l3, 2, 0, Qt::AlignRight);
	m1->addWidget(l4, 3, 0, Qt::AlignRight);
	m1->addWidget(user_, 0, 1);
	m1->addWidget(group_, 1, 1);
	m1->addWidget(ripcache_, 2, 1);
	m1->addWidget(filterlimit_, 3, 1);
}

bool CupsdFilterPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	user_->setText(conf_->user_);
	group_->setText(conf_->group_);
	ripcache_->setSizeString(conf_->ripcache_);
	filterlimit_->setValue(conf_->filterlimit_);

	return true;
}

bool CupsdFilterPage::saveConfig(CupsdConf *conf, QString&)
{
	conf->user_ = user_->text();
	conf->group_ = group_->text();
	conf->ripcache_ = ripcache_->sizeString();
	conf->filterlimit_ = filterlimit_->value();

	return true;
}

void CupsdFilterPage::setInfos(CupsdConf *conf)
{
	QWhatsThis::add(user_, conf->comments_.toolTip("user"));
	QWhatsThis::add(group_, conf->comments_.toolTip("group"));
	QWhatsThis::add(ripcache_, conf->comments_.toolTip("ripcache"));
	QWhatsThis::add(filterlimit_, conf->comments_.toolTip("filterlimit"));
}
