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

#include "cupsdbrowsinggeneralpage.h"

#include <klocale.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupsdoption.h"

CupsdBrowsingGeneralPage::CupsdBrowsingGeneralPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Browsing"));
	//path_.append(i18n("General"));
	header_ = i18n("Browsing general configuration");

	for (int i=0;i<3;i++)
		opt_[i] = new CupsdOption(this);

	browsing_ = new QCheckBox(i18n("Enable browsing"), opt_[0]);
	browseshortnames_ = new QCheckBox(i18n("Use short names when possible"), opt_[1]);
	implicitclasses_ = new QCheckBox(i18n("Use implicit classes"), opt_[2]);

	QVBoxLayout	*main_ = new QVBoxLayout(this, 10, 10);
	main_->addWidget(deflabel_, 0, Qt::AlignRight|Qt::AlignVCenter);
	main_->addWidget(opt_[0]);
	main_->addWidget(opt_[1]);
	main_->addWidget(opt_[2]);
	main_->addStretch(1);
}

CupsdBrowsingGeneralPage::~CupsdBrowsingGeneralPage()
{
}

bool CupsdBrowsingGeneralPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	if (conf->browsing_ != -1)
	{
		opt_[0]->setDefault(0);
		browsing_->setChecked(conf->browsing_ == 1);
	}
	if (conf->browseshortnames_ != -1)
	{
		opt_[1]->setDefault(0);
		browseshortnames_->setChecked(conf->browseshortnames_ == 1);
	}
	if (conf->implicitclasses_ != -1)
	{
		opt_[2]->setDefault(0);
		implicitclasses_->setChecked(conf->implicitclasses_ == 1);
	}
	return true;
}

bool CupsdBrowsingGeneralPage::saveConfig(CupsdConf *conf, QString&)
{
	if (!opt_[0]->isDefault()) conf->browsing_ = (browsing_->isChecked() ? 1 : 0);
	if (!opt_[1]->isDefault()) conf->browseshortnames_ = (browseshortnames_->isChecked() ? 1 : 0);
	if (!opt_[2]->isDefault()) conf->implicitclasses_ = (implicitclasses_->isChecked() ? 1 : 0);
	return true;
}

void CupsdBrowsingGeneralPage::setDefaults()
{
	browsing_->setChecked(true);
	browseshortnames_->setChecked(true);
	implicitclasses_->setChecked(true);
}

void CupsdBrowsingGeneralPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(browsing_, conf->comments_.toolTip(BROWSING_COMM));
        QWhatsThis::add(browseshortnames_, conf->comments_.toolTip(BROWSESHORTNAMES_COMM));
        QWhatsThis::add(implicitclasses_, conf->comments_.toolTip(IMPLICITCLASSES_COMM));
}
