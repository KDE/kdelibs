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

#include "cupsdserveridentitypage.h"

#include <qlineedit.h>
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupsdoption.h"

CupsdServerIdentityPage::CupsdServerIdentityPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Server"));
	//path_.append(i18n("Identity"));
	header_ = i18n("Server general configuration");

	for (int i=0;i<5;i++)
		opt_[i] = new CupsdOption(this);

	servername_ = new QLineEdit(opt_[0]);
	serveradmin_ = new QLineEdit(opt_[1]);
	user_ = new QLineEdit(opt_[2]);
	group_ = new QLineEdit(opt_[3]);
	remroot_ = new QLineEdit(opt_[4]);

	QLabel	*l1 = new QLabel(i18n("Server name:"), this);
	QLabel	*l2 = new QLabel(i18n("Administrator's email:"), this);
	QLabel	*l3 = new QLabel(i18n("Server user:"), this);
	QLabel	*l4 = new QLabel(i18n("Server group:"), this);
	QLabel	*l5 = new QLabel(i18n("Remote user name:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 9, 2, 10, 10);
	main_->addWidget(deflabel_, 0, 1, Qt::AlignRight|Qt::AlignVCenter);
	main_->addWidget(opt_[0], 1, 1);
	main_->addWidget(opt_[1], 2, 1);
	main_->addWidget(opt_[2], 4, 1);
	main_->addWidget(opt_[3], 5, 1);
	main_->addWidget(opt_[4], 7, 1);
	main_->addWidget(l1, 1, 0);
	main_->addWidget(l2, 2, 0);
	main_->addWidget(l3, 4, 0);
	main_->addWidget(l4, 5, 0);
	main_->addWidget(l5, 7, 0);
	main_->setRowStretch(8, 1);
	main_->addRowSpacing(3, 20);
	main_->addRowSpacing(6, 20);
}

CupsdServerIdentityPage::~CupsdServerIdentityPage()
{
}

bool CupsdServerIdentityPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	if (!conf->servername_.isNull())
	{
		opt_[0]->setDefault(false);
		servername_->setText(conf->servername_);
	}
	if (!conf->serveradmin_.isNull())
	{
		opt_[1]->setDefault(false);
		serveradmin_->setText(conf->serveradmin_);
	}
	if (!conf->user_.isNull())
	{
		opt_[2]->setDefault(false);
		user_->setText(conf->user_);
	}
	if (!conf->group_.isNull())
	{
		opt_[3]->setDefault(false);
		group_->setText(conf->group_);
	}
	if (!conf->remroot_.isNull())
	{
		opt_[4]->setDefault(false);
		remroot_->setText(conf->remroot_);
	}
	return true;
}

bool CupsdServerIdentityPage::saveConfig(CupsdConf *conf, QString&)
{
	if (!opt_[0]->isDefault() && !servername_->text().isNull()) conf->servername_ = servername_->text();
	if (!opt_[1]->isDefault() && !serveradmin_->text().isNull()) conf->serveradmin_ = serveradmin_->text();
	if (!opt_[2]->isDefault() && !user_->text().isNull()) conf->user_ = user_->text();
	if (!opt_[3]->isDefault() && !group_->text().isNull()) conf->group_ = group_->text();
	if (!opt_[4]->isDefault() && !remroot_->text().isNull()) conf->remroot_ = remroot_->text();
	return true;
}

void CupsdServerIdentityPage::setDefaults()
{
	servername_->setText("localhost");
	serveradmin_->setText("root@localhost");
	user_->setText("lp");
	group_->setText("sys");
	remroot_->setText("remroot");
}

void CupsdServerIdentityPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(servername_, conf->comments_.toolTip(SERVERNAME_COMM));
        QWhatsThis::add(serveradmin_, conf->comments_.toolTip(SERVERADMIN_COMM));
        QWhatsThis::add(user_, conf->comments_.toolTip(USER_COMM));
        QWhatsThis::add(group_, conf->comments_.toolTip(GROUP_COMM));
        QWhatsThis::add(remroot_, conf->comments_.toolTip(REMOTEROOT_COMM));
}
