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

#include "cupsddirpage.h"
#include "cupsdconf.h"
#include "qdirlineedit.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <klocale.h>

CupsdDirPage::CupsdDirPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	setPageLabel(i18n("Directories"));
	setHeader(i18n("Directories settings"));
	setPixmap("folder");

	datadir_ = new QDirLineEdit(false, this);
	documentdir_ = new QDirLineEdit(false, this);
	fontpath_ = new QDirLineEdit(false, this);
	requestdir_ = new QDirLineEdit(false, this);
	serverbin_ = new QDirLineEdit(false, this);
	serverfiles_ = new QDirLineEdit(false, this);
	tmpfiles_ = new QDirLineEdit(false, this);

	QFont	f(font());
	f.setBold(true);
	QLabel *l1 = new QLabel(i18n("Data Directory:"), this);
	QLabel *l2 = new QLabel(i18n("Document Directory:"), this);
	QLabel *l3 = new QLabel(i18n("Font Path:"), this);
	QLabel *l4 = new QLabel(i18n("Request Directory:"), this);
	QLabel *l5 = new QLabel(i18n("Server Binaries:"), this);
	QLabel *l6 = new QLabel(i18n("Server Files:"), this);
	QLabel *l7 = new QLabel(i18n("Temporary Files:"), this);
	l1->setFont(f);
	l2->setFont(f);
	l3->setFont(f);
	l4->setFont(f);
	l5->setFont(f);
	l6->setFont(f);
	l7->setFont(f);

	QGridLayout	*m1 = new QGridLayout(this, 8, 2, 10, 7);
	m1->setRowStretch(7, 1);
	m1->setColStretch(1, 1);
	m1->addWidget(l1, 0, 0, Qt::AlignRight);
	m1->addWidget(l2, 1, 0, Qt::AlignRight);
	m1->addWidget(l3, 2, 0, Qt::AlignRight);
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
	fontpath_->setURL(conf_->fontpath_);
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
	conf->fontpath_ = fontpath_->url();
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
