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

#include "cupsdlogpage.h"
#include "cupsdconf.h"
#include "qdirlineedit.h"
#include "sizewidget.h"

#include <qlabel.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <klocale.h>
#include <kfiledialog.h>

CupsdLogPage::CupsdLogPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	setPageLabel(i18n("Log"));
	setHeader(i18n("Log settings"));
	setPixmap("contents");

	accesslog_ = new QDirLineEdit(true, this);
	errorlog_ = new QDirLineEdit(true, this);
	pagelog_ = new QDirLineEdit(true, this);
	maxlogsize_ = new SizeWidget(this);
	loglevel_ = new QComboBox(this);

	loglevel_->insertItem(i18n("Detailed Debugging"));
	loglevel_->insertItem(i18n("Debug Information"));
	loglevel_->insertItem(i18n("General Information"));
	loglevel_->insertItem(i18n("Warnings"));
	loglevel_->insertItem(i18n("Errors"));
	loglevel_->insertItem(i18n("No Logging"));

	/*maxlogsize_->setRange(0, 100, 1, true);
	maxlogsize_->setSteps(1, 5);
	maxlogsize_->setSpecialValueText(i18n("Unlimited"));
	maxlogsize_->setSuffix(i18n("MB"));*/

	QLabel *l1 = new QLabel(i18n("Access log:"), this);
	QLabel *l2 = new QLabel(i18n("Error log:"), this);
	QLabel *l3 = new QLabel(i18n("Page log:"), this);
	QLabel *l4 = new QLabel(i18n("Max log size:"), this);
	QLabel *l5 = new QLabel(i18n("Log level:"), this);

	loglevel_->setCurrentItem(2);

	QGridLayout	*m1 = new QGridLayout(this, 6, 2, 10, 7);
	m1->setRowStretch(5, 1);
	m1->setColStretch(1, 1);
	m1->addWidget(l1, 0, 0, Qt::AlignRight);
	m1->addWidget(l2, 1, 0, Qt::AlignRight);
	m1->addWidget(l3, 2, 0, Qt::AlignRight);
	m1->addWidget(l4, 3, 0, Qt::AlignRight);
	m1->addWidget(l5, 4, 0, Qt::AlignRight);
	m1->addWidget(accesslog_, 0, 1);
	m1->addWidget(errorlog_, 1, 1);
	m1->addWidget(pagelog_, 2, 1);
	m1->addWidget(maxlogsize_, 3, 1);
	m1->addWidget(loglevel_, 4, 1);
}

bool CupsdLogPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	accesslog_->setURL(conf_->accesslog_);
	errorlog_->setURL(conf_->errorlog_);
	pagelog_->setURL(conf_->pagelog_);
	maxlogsize_->setSizeString(conf_->maxlogsize_);
	loglevel_->setCurrentItem(conf_->loglevel_);

	return true;
}

bool CupsdLogPage::saveConfig(CupsdConf *conf, QString&)
{
	conf->accesslog_ = accesslog_->url();
	conf->errorlog_ = errorlog_->url();
	conf->pagelog_ = pagelog_->url();
	conf->maxlogsize_ = maxlogsize_->sizeString();
	conf->loglevel_ = loglevel_->currentItem();

	return true;
}

void CupsdLogPage::setInfos(CupsdConf *conf)
{
	QWhatsThis::add(accesslog_, conf->comments_.toolTip("accesslog"));
	QWhatsThis::add(errorlog_, conf->comments_.toolTip("errorlog"));
	QWhatsThis::add(pagelog_, conf->comments_.toolTip("pagelog"));
	QWhatsThis::add(maxlogsize_, conf->comments_.toolTip("maxlogsize"));
	QWhatsThis::add(loglevel_, conf->comments_.toolTip("loglevel"));
}
