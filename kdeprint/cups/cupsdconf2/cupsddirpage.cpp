/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "cupsddirpage.h"
#include "cupsdconf.h"
#include "qdirlineedit.h"
#include "qdirmultilineedit.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <klocale.h>

CupsdDirPage::CupsdDirPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	setPageLabel(i18n("Folders"));
	setHeader(i18n("Folders Settings"));
	setPixmap("folder");

	datadir_ = new QDirLineEdit(false, this);
	documentdir_ = new QDirLineEdit(false, this);
	fontpath_ = new QDirMultiLineEdit(this);
	requestdir_ = new QDirLineEdit(false, this);
	serverbin_ = new QDirLineEdit(false, this);
	serverfiles_ = new QDirLineEdit(false, this);
	tmpfiles_ = new QDirLineEdit(false, this);

	QLabel *l1 = new QLabel(i18n("Data folder:"), this);
	QLabel *l2 = new QLabel(i18n("Document folder:"), this);
	QLabel *l3 = new QLabel(i18n("Font path:"), this);
	QLabel *l4 = new QLabel(i18n("Request folder:"), this);
	QLabel *l5 = new QLabel(i18n("Server binaries:"), this);
	QLabel *l6 = new QLabel(i18n("Server files:"), this);
	QLabel *l7 = new QLabel(i18n("Temporary files:"), this);

	QGridLayout	*m1 = new QGridLayout(this, 8, 2, 10, 7);
	m1->setRowStretch(7, 1);
	m1->setColStretch(1, 1);
	m1->addWidget(l1, 0, 0, Qt::AlignRight);
	m1->addWidget(l2, 1, 0, Qt::AlignRight);
	m1->addWidget(l3, 2, 0, Qt::AlignRight|Qt::AlignTop);
	m1->addWidget(l4, 3, 0, Qt::AlignRight);
	m1->addWidget(l5, 4, 0, Qt::AlignRight);
	m1->addWidget(l6, 5, 0, Qt::AlignRight);
	m1->addWidget(l7, 6, 0, Qt::AlignRight);
	m1->addWidget(datadir_, 0, 1);
	m1->addWidget(documentdir_, 1, 1);
	m1->addWidget(fontpath_, 2, 1);
	m1->addWidget(requestdir_, 3, 1);
	m1->addWidget(serverbin_, 4, 1);
	m1->addWidget(serverfiles_, 5, 1);
	m1->addWidget(tmpfiles_, 6, 1);
}

bool CupsdDirPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	datadir_->setURL(conf_->datadir_);
	documentdir_->setURL(conf_->documentdir_);
	fontpath_->setURLs(conf_->fontpath_);
	requestdir_->setURL(conf_->requestdir_);
	serverbin_->setURL(conf_->serverbin_);
	serverfiles_->setURL(conf_->serverfiles_);
	tmpfiles_->setURL(conf_->tmpfiles_);

	return true;
}

bool CupsdDirPage::saveConfig(CupsdConf *conf, QString&)
{
	conf->datadir_ = datadir_->url();
	conf->documentdir_ = documentdir_->url();
	conf->fontpath_ = fontpath_->urls();
	conf->requestdir_ = requestdir_->url();
	conf->serverbin_ = serverbin_->url();
	conf->serverfiles_ = serverfiles_->url();
	conf->tmpfiles_ = tmpfiles_->url();

	return true;
}

void CupsdDirPage::setInfos(CupsdConf *conf)
{
	QWhatsThis::add(datadir_, conf->comments_.toolTip("datadir"));
	QWhatsThis::add(documentdir_, conf->comments_.toolTip("documentroot"));
	QWhatsThis::add(fontpath_, conf->comments_.toolTip("fontpath"));
	QWhatsThis::add(requestdir_, conf->comments_.toolTip("requestroot"));
	QWhatsThis::add(serverbin_, conf->comments_.toolTip("serverbin"));
	QWhatsThis::add(serverfiles_, conf->comments_.toolTip("serverroot"));
	QWhatsThis::add(tmpfiles_, conf->comments_.toolTip("tempdir"));
}
