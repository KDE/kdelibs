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

#include "cupsdservermiscpage.h"

#include "qdirlineedit.h"
#include <qlineedit.h>
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupsdoption.h"

CupsdServerMiscPage::CupsdServerMiscPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Server"));
	path_.append(i18n("Misc"));
	header_ = i18n("Server miscellaneous configuration");

	for (int i=0;i<2;i++)
		opt_[i] = new CupsdOption(this);

	printcap_ = new QDirLineEdit(opt_[0]);
	printcap_->setFileEdit(true);
	ripcache_ = new QLineEdit(opt_[1]);

	QLabel	*l1 = new QLabel(i18n("Printcap file:"), this);
	QLabel	*l2 = new QLabel(i18n("RIP cache:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 4, 2, 10, 10);
	main_->addWidget(deflabel_, 0, 1, Qt::AlignRight|Qt::AlignVCenter);
	main_->addWidget(opt_[0], 1, 1);
	main_->addWidget(opt_[1], 2, 1);
	main_->addWidget(l1, 1, 0);
	main_->addWidget(l2, 2, 0);
	main_->setRowStretch(3, 1);
}

CupsdServerMiscPage::~CupsdServerMiscPage()
{
}

bool CupsdServerMiscPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	if (!conf->printcap_.isNull())
	{
		opt_[0]->setDefault(false);
		printcap_->setText(conf->printcap_);
	}
	if (!conf->ripcache_.isNull())
	{
		opt_[1]->setDefault(false);
		ripcache_->setText(conf->ripcache_);
	}
	return true;
}

bool CupsdServerMiscPage::saveConfig(CupsdConf *conf, QString& msg)
{
	if (!opt_[0]->isDefault() && !printcap_->text().isNull()) conf->printcap_ = printcap_->text();
	if (!opt_[1]->isDefault() && !ripcache_->text().isNull()) conf->ripcache_ = ripcache_->text();
	return true;
}

void CupsdServerMiscPage::setDefaults()
{
	printcap_->setText("/etc/printcap");
	ripcache_->setText("8m");
}

void CupsdServerMiscPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(printcap_, conf->comments_.toolTip(PRINTCAP_COMM));
        QWhatsThis::add(ripcache_, conf->comments_.toolTip(RIPCACHE_COMM));
}
