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

#include "cupsdserverhttppage.h"

#include "qdirlineedit.h"
#include <qlineedit.h>
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupsdoption.h"

CupsdServerHTTPPage::CupsdServerHTTPPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Server"));
	path_.append(i18n("HTTP"));
	header_ = i18n("Server HTTP configuration");

	for (int i=0;i<3;i++)
		opt_[i] = new CupsdOption(this);

	defaultcharset_ = new QLineEdit(opt_[0]);
	defaultlanguage_ = new QLineEdit(opt_[1]);
	documentroot_ = new QDirLineEdit(opt_[2]);

	QLabel	*l1 = new QLabel(i18n("Document directory:"), this);
	QLabel	*l2 = new QLabel(i18n("Default language:"), this);
	QLabel	*l3 = new QLabel(i18n("Default charset:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 5, 2, 10, 10);
	main_->addWidget(deflabel_, 0, 1, Qt::AlignRight|Qt::AlignVCenter);
	main_->addWidget(opt_[2], 1, 1);
	main_->addWidget(opt_[1], 2, 1);
	main_->addWidget(opt_[0], 3, 1);
	main_->addWidget(l1, 1, 0);
	main_->addWidget(l2, 2, 0);
	main_->addWidget(l3, 3, 0);
	main_->setRowStretch(4, 1);
}

CupsdServerHTTPPage::~CupsdServerHTTPPage()
{
}

bool CupsdServerHTTPPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	if (!conf->defaultcharset_.isNull())
	{
		opt_[0]->setDefault(0);
		defaultcharset_->setText(conf->defaultcharset_);
	}
	if (!conf->defaultlanguage_.isNull())
	{
		opt_[1]->setDefault(0);
		defaultlanguage_->setText(conf->defaultlanguage_);
	}
	if (!conf->documentroot_.isNull())
	{
		opt_[2]->setDefault(0);
		documentroot_->setText(conf->documentroot_);
	}
	return true;
}

bool CupsdServerHTTPPage::saveConfig(CupsdConf *conf, QString&)
{
	if (!opt_[0]->isDefault() && !defaultcharset_->text().isNull()) conf->defaultcharset_ = defaultcharset_->text();
	if (!opt_[1]->isDefault() && !defaultlanguage_->text().isNull()) conf->defaultlanguage_ = defaultlanguage_->text();
	if (!opt_[2]->isDefault() && !documentroot_->text().isNull()) conf->documentroot_ = documentroot_->text();
	return true;
}

void CupsdServerHTTPPage::setDefaults()
{
	defaultcharset_->setText("utf-8");
	defaultlanguage_->setText("en");
	documentroot_->setText("/usr/share/cups/doc");
}

void CupsdServerHTTPPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(defaultcharset_, conf->comments_.toolTip(DEFAULTCHARSET_COMM));
        QWhatsThis::add(defaultlanguage_, conf->comments_.toolTip(DEFAULTLANG_COMM));
        QWhatsThis::add(documentroot_, conf->comments_.toolTip(DOCUMENTROOT_COMM));
}
