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

#include "cupsdserverencryptpage.h"

#include "qdirlineedit.h"
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupsdoption.h"

CupsdServerEncryptPage::CupsdServerEncryptPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Server"));
	path_.append(i18n("Encryption"));
	header_ = i18n("Server encryption support configuration");

	for (int i=0;i<2;i++)
		opt_[i] = new CupsdOption(this);

	servercertificate_ = new QDirLineEdit(opt_[0]);
	serverkey_ = new QDirLineEdit(opt_[1]);

	QLabel	*l1 = new QLabel(i18n("Server certificate:"), this);
	QLabel	*l2 = new QLabel(i18n("Server key:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 4, 2, 10, 10);
	main_->addWidget(deflabel_, 0, 1, Qt::AlignRight|Qt::AlignVCenter);
	main_->addWidget(l1, 1, 0);
	main_->addWidget(l2, 2, 0);
	main_->addWidget(opt_[0], 1, 1);
	main_->addWidget(opt_[1], 2, 1);
	main_->setRowStretch(3, 1);
}

CupsdServerEncryptPage::~CupsdServerEncryptPage()
{
}

bool CupsdServerEncryptPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	if (!conf->servercertificate_.isNull())
	{
		opt_[0]->setDefault(false);
		servercertificate_->setText(conf->servercertificate_);
	}
	if (!conf->serverkey_.isNull())
	{
		opt_[1]->setDefault(false);
		serverkey_->setText(conf->serverkey_);
	}
	return true;
}

bool CupsdServerEncryptPage::saveConfig(CupsdConf *conf, QString&)
{
	if (!opt_[0]->isDefault() && !servercertificate_->text().isNull()) conf->servercertificate_ = servercertificate_->text();
	if (!opt_[1]->isDefault() && !serverkey_->text().isNull()) conf->serverkey_ = serverkey_->text();
	return true;
}

void CupsdServerEncryptPage::setDefaults()
{
	servercertificate_->setText("/etc/cups/ssl/server.crt");
	serverkey_->setText("/etc/cups/ssl/server.key");
}

void CupsdServerEncryptPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(servercertificate_, conf->comments_.toolTip(SERVERCERTIFICATE_COMM));
        QWhatsThis::add(serverkey_, conf->comments_.toolTip(SERVERKEY_COMM));
}
