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

#include "cupsdserverlogpage.h"

#include "qdirlineedit.h"
#include <qlineedit.h>
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupsdoption.h"

CupsdServerLogPage::CupsdServerLogPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Server"));
	path_.append(i18n("Log"));
	header_ = i18n("Server logging configuration");

	for (int i=0;i<5;i++)
		opt_[i] = new CupsdOption(this);

	accesslog_ = new QDirLineEdit(opt_[0]);
	accesslog_->setFileEdit(true);
	errorlog_ = new QDirLineEdit(opt_[1]);
	errorlog_->setFileEdit(true);
	pagelog_ = new QDirLineEdit(opt_[2]);
	pagelog_->setFileEdit(true);
	loglevel_ = new QComboBox(opt_[3]);
	loglevel_->insertItem(i18n("Complete debug (everything)"));
	loglevel_->insertItem(i18n("Partial debug (almost everything)"));
	loglevel_->insertItem(i18n("Info (normal)"));
	loglevel_->insertItem(i18n("Warning (errors and warnings)"));
	loglevel_->insertItem(i18n("Error (errors only)"));
	loglevel_->insertItem(i18n("None"));
	maxlogsize_ = new QLineEdit(opt_[4]);

	QLabel	*l1 = new QLabel(i18n("Access log file:"), this);
	QLabel	*l2 = new QLabel(i18n("Error log file:"), this);
	QLabel	*l3 = new QLabel(i18n("Page log file:"), this);
	QLabel	*l4 = new QLabel(i18n("Log level:"), this);
	QLabel	*l5 = new QLabel(i18n("Max log file size:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 8, 2, 10, 10);
	main_->addWidget(deflabel_, 0, 1, Qt::AlignRight|Qt::AlignVCenter);
	main_->addWidget(opt_[0], 1, 1);
	main_->addWidget(opt_[1], 2, 1);
	main_->addWidget(opt_[2], 3, 1);
	main_->addWidget(opt_[3], 5, 1);
	main_->addWidget(opt_[4], 6, 1);
	main_->addWidget(l1, 1, 0);
	main_->addWidget(l2, 2, 0);
	main_->addWidget(l3, 3, 0);
	main_->addWidget(l4, 5, 0);
	main_->addWidget(l5, 6, 0);
	main_->setRowStretch(7, 1);
	main_->addRowSpacing(4, 20);

}

CupsdServerLogPage::~CupsdServerLogPage()
{
}

bool CupsdServerLogPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	if (!conf->accesslog_.isNull())
	{
		opt_[0]->setDefault(false);
		accesslog_->setText(conf->accesslog_);
	}
	if (!conf->errorlog_.isNull())
	{
		opt_[1]->setDefault(false);
		errorlog_->setText(conf->errorlog_);
	}
	if (!conf->pagelog_.isNull())
	{
		opt_[2]->setDefault(false);
		pagelog_->setText(conf->pagelog_);
	}
	if (conf->maxlogsize_ != -1)
	{
		opt_[4]->setDefault(false);
		maxlogsize_->setText(QString::number(conf->maxlogsize_));
	}
	if (conf->loglevel_ != -1)
	{
		opt_[3]->setDefault(false);
		loglevel_->setCurrentItem(conf->loglevel_);
	}
	return true;
}

bool CupsdServerLogPage::saveConfig(CupsdConf *conf, QString& msg)
{
	if (!opt_[0]->isDefault() && !accesslog_->text().isNull()) conf->accesslog_ = accesslog_->text();
	if (!opt_[1]->isDefault() && !errorlog_->text().isNull()) conf->errorlog_ = errorlog_->text();
	if (!opt_[2]->isDefault() && !pagelog_->text().isNull()) conf->pagelog_ = pagelog_->text();
	if (!opt_[4]->isDefault() && !maxlogsize_->text().isNull())
	{
		bool	ok;
		conf->maxlogsize_ = maxlogsize_->text().toInt(&ok);
		if (!ok)
		{
			msg = i18n("Max log size: wrong argument");
			return false;
		}
	}
	if (!opt_[3]->isDefault()) conf->loglevel_ = loglevel_->currentItem();
	return true;
}

void CupsdServerLogPage::setDefaults()
{
	accesslog_->setText("/var/log/cups/access_log");
	errorlog_->setText("/var/log/cups/error_log");
	pagelog_->setText("/var/log/cups/page_log");
	maxlogsize_->setText("1048576");
	loglevel_->setCurrentItem(1);
}

void CupsdServerLogPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(accesslog_, conf->comments_.toolTip(ACCESSLOG_COMM));
        QWhatsThis::add(errorlog_, conf->comments_.toolTip(ERRORLOG_COMM));
        QWhatsThis::add(pagelog_, conf->comments_.toolTip(PAGELOG_COMM));
        QWhatsThis::add(maxlogsize_, conf->comments_.toolTip(MAXLOGSIZE_COMM));
        QWhatsThis::add(loglevel_, conf->comments_.toolTip(LOGLEVEL_COMM));
}
