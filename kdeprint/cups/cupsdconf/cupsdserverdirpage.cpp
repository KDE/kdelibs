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

#include "cupsdserverdirpage.h"

#include "qdirlineedit.h"
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupsdoption.h"

CupsdServerDirPage::CupsdServerDirPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Server"));
	path_.append(i18n("Directories"));
	header_ = i18n("Server directories configuration");

	for (int i=0;i<6;i++)
		opt_[i] = new CupsdOption(this);

	serverbin_ = new QDirLineEdit(opt_[0]);
	serverroot_ = new QDirLineEdit(opt_[1]);
	datadir_ = new QDirLineEdit(opt_[2]);
	tempdir_ = new QDirLineEdit(opt_[3]);
	requestroot_ = new QDirLineEdit(opt_[4]);
	fontpath_ = new QDirLineEdit(opt_[5]);

	QLabel	*l1 = new QLabel(i18n("Executables:"), this);
	QLabel	*l2 = new QLabel(i18n("Configuration:"), this);
	QLabel	*l3 = new QLabel(i18n("Data:"), this);
	QLabel	*l4 = new QLabel(i18n("Temporary files:"), this);
	QLabel	*l5 = new QLabel(i18n("Temporary requests:"), this);
	QLabel	*l6 = new QLabel(i18n("Font path:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 10, 2, 10, 10);
	main_->addWidget(deflabel_, 0, 1, Qt::AlignRight|Qt::AlignVCenter);
	main_->addWidget(opt_[0], 1, 1);
	main_->addWidget(opt_[1], 2, 1);
	main_->addWidget(opt_[2], 3, 1);
	main_->addWidget(opt_[3], 5, 1);
	main_->addWidget(opt_[4], 6, 1);
	main_->addWidget(opt_[5], 8, 1);
	main_->addWidget(l1, 1, 0);
	main_->addWidget(l2, 2, 0);
	main_->addWidget(l3, 3, 0);
	main_->addWidget(l4, 5, 0);
	main_->addWidget(l5, 6, 0);
	main_->addWidget(l6, 8, 0);
	main_->setRowStretch(9, 1);
	main_->addRowSpacing(4, 20);
	main_->addRowSpacing(7, 20);

}

CupsdServerDirPage::~CupsdServerDirPage()
{
}

bool CupsdServerDirPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	if (!conf->serverbin_.isNull())
	{
		opt_[0]->setDefault(false);
		serverbin_->setText(conf->serverbin_);
	}
	if (!conf->serverroot_.isNull())
	{
		opt_[1]->setDefault(false);
		serverroot_->setText(conf->serverroot_);
	}
	if (!conf->datadir_.isNull())
	{
		opt_[2]->setDefault(false);
		datadir_->setText(conf->datadir_);
	}
	if (!conf->tempdir_.isNull())
	{
		opt_[3]->setDefault(false);
		tempdir_->setText(conf->tempdir_);
	}
	if (!conf->requestroot_.isNull())
	{
		opt_[4]->setDefault(false);
		requestroot_->setText(conf->requestroot_);
	}
	if (!conf->fontpath_.isNull())
	{
		opt_[5]->setDefault(false);
		fontpath_->setText(conf->fontpath_);
	}
	return true;
}

bool CupsdServerDirPage::saveConfig(CupsdConf *conf, QString&)
{
	if (!opt_[0]->isDefault() && !serverbin_->text().isNull()) conf->serverbin_ = serverbin_->text();
	if (!opt_[1]->isDefault() && !serverroot_->text().isNull()) conf->serverroot_ = serverroot_->text();
	if (!opt_[2]->isDefault() && !datadir_->text().isNull()) conf->datadir_ = datadir_->text();
	if (!opt_[3]->isDefault() && !tempdir_->text().isNull()) conf->tempdir_ = tempdir_->text();
	if (!opt_[4]->isDefault() && !requestroot_->text().isNull()) conf->requestroot_ = requestroot_->text();
	if (!opt_[5]->isDefault() && !fontpath_->text().isNull()) conf->fontpath_ = fontpath_->text();
	return true;
}

void CupsdServerDirPage::setDefaults()
{
	serverbin_->setText("/usr/lib/cups");
	serverroot_->setText("/etc/cups");
	datadir_->setText("/usr/share/cups/data");
	tempdir_->setText("/var/spool/cups/tmp");
	requestroot_->setText("/var/spool/cups");
	fontpath_->setText("/usr/share/cups/fonts");
}

void CupsdServerDirPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(serverbin_, conf->comments_.toolTip(SERVERBIN_COMM));
        QWhatsThis::add(serverroot_, conf->comments_.toolTip(SERVERROOT_COMM));
        QWhatsThis::add(datadir_, conf->comments_.toolTip(DATADIR_COMM));
        QWhatsThis::add(tempdir_, conf->comments_.toolTip(TEMPDIR_COMM));
        QWhatsThis::add(requestroot_, conf->comments_.toolTip(REQUESTROOT_COMM));
        QWhatsThis::add(fontpath_, conf->comments_.toolTip(FONTPATH_COMM));
}
