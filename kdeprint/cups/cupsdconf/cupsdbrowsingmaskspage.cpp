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

#include "cupsdbrowsingmaskspage.h"

#include <klocale.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupsdoption.h"
#include "cupslist.h"

CupsdBrowsingMasksPage::CupsdBrowsingMasksPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Browsing"));
	path_.append(i18n("Masks"));
	header_ = i18n("Browsing masks configuration");

	for (int i=0;i<3;i++)
		opt_[i] = new CupsdOption(this);

	browseallow_ = new CupsListBox(opt_[0]);
	browsedeny_ = new CupsListBox(opt_[1]);
	browseorder_ = new QComboBox(opt_[2]);
	browseorder_->insertItem(i18n("Allow, Deny"));
	browseorder_->insertItem(i18n("Deny, Allow"));

	QLabel	*l1 = new QLabel(i18n("Browse allow:"), this);
	QLabel	*l2 = new QLabel(i18n("Browse deny:"), this);
	QLabel	*l3 = new QLabel(i18n("Browse order:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 7, 2, 10, 10);
	main_->addWidget(deflabel_, 0, 1, Qt::AlignRight|Qt::AlignVCenter);
	main_->addMultiCellWidget(opt_[0], 1, 2, 1, 1);
	main_->addMultiCellWidget(opt_[1], 3, 4, 1, 1);
	main_->addWidget(opt_[2], 5, 1);
	main_->addWidget(l1, 1, 0, Qt::AlignLeft|Qt::AlignTop);
	main_->addWidget(l2, 3, 0, Qt::AlignLeft|Qt::AlignTop);
	main_->addWidget(l3, 5, 0);
	main_->setRowStretch(6, 1);
}

CupsdBrowsingMasksPage::~CupsdBrowsingMasksPage()
{
}

bool CupsdBrowsingMasksPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	QStringList::Iterator	it;
	if (conf->browseallow_.count() > 0)
	{
		opt_[0]->setDefault(false);
		for (it=conf->browseallow_.begin();it!=conf->browseallow_.end();++it)
			browseallow_->insertItem(*it);
	}
	if (conf->browsedeny_.count() > 0)
	{
		opt_[1]->setDefault(false);
		for (it=conf->browsedeny_.begin();it!=conf->browsedeny_.end();++it)
			browsedeny_->insertItem(*it);
	}
	if (conf->browseorder_ != -1)
	{
		opt_[2]->setDefault(false);
		browseorder_->setCurrentItem(conf->browseorder_);
	}
	return true;
}

bool CupsdBrowsingMasksPage::saveConfig(CupsdConf *conf, QString&)
{
	if (!opt_[0]->isDefault() && browseallow_->count() > 0)
	{
		conf->browseallow_.clear();
		for (int i=0;i<browseallow_->count();i++)
			conf->browseallow_.append(browseallow_->text(i));
	}
	if (!opt_[1]->isDefault() && browsedeny_->count() > 0)
	{
		conf->browsedeny_.clear();
		for (int i=0;i<browsedeny_->count();i++)
			conf->browsedeny_.append(browsedeny_->text(i));
	}
	if (!opt_[2]->isDefault()) conf->browseorder_ = browseorder_->currentItem();
	return true;
}

void CupsdBrowsingMasksPage::setDefaults()
{
	browseorder_->setCurrentItem(ORDER_DENY_ALLOW);
}

void CupsdBrowsingMasksPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(browseorder_, conf->comments_.toolTip(BROWSEORDER_COMM));
        QWhatsThis::add(browseallow_, conf->comments_.toolTip(BROWSEALLOWDENY_COMM));
        QWhatsThis::add(browsedeny_, conf->comments_.toolTip(BROWSEALLOWDENY_COMM));
}
