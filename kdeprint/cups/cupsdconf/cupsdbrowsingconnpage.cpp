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

#include "cupsdbrowsingconnpage.h"

#include <klocale.h>
#include <kseparator.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupsdoption.h"
#include "cupslist.h"

CupsdBrowsingConnPage::CupsdBrowsingConnPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Browsing"));
	path_.append(i18n("Connection"));
	header_ = i18n("Browsing connection configuration");

	for (int i=0;i<3;i++)
		opt_[i] = new CupsdOption(this);

	browseaddress_ = new CupsListBox(opt_[0]);
	browseport_ = new QLineEdit(opt_[1]);
	browsepoll_ = new CupsListBox(opt_[2]);

        KSeparator* sep = new KSeparator( KSeparator::HLine, this);

	QLabel	*l1 = new QLabel(i18n("Broadcast addresses:"), this);
	QLabel	*l2 = new QLabel(i18n("Broadcast port:"), this);
	QLabel	*l3 = new QLabel(i18n("Poll addresses:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 8, 2, 10, 10);
	main_->addWidget(deflabel_, 0, 1, Qt::AlignRight|Qt::AlignVCenter);
	main_->addMultiCellWidget(opt_[0], 1, 2, 1, 1);
	main_->addMultiCellWidget(opt_[2], 5, 6, 1, 1);
	main_->addWidget(opt_[1], 3, 1);
	main_->addWidget(sep, 4, 1);
        main_->addRowSpacing(4, 10);
	main_->addWidget(l1, 1, 0, Qt::AlignLeft|Qt::AlignTop);
	main_->addWidget(l2, 3, 0);
	main_->addWidget(l3, 5, 0, Qt::AlignLeft|Qt::AlignTop);
	main_->setRowStretch(7, 1);
}

CupsdBrowsingConnPage::~CupsdBrowsingConnPage()
{
}

bool CupsdBrowsingConnPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	QStringList::Iterator	it;
	if (conf->browseaddress_.count() > 0)
	{
		opt_[0]->setDefault(false);
		for (it=conf->browseaddress_.begin();it!=conf->browseaddress_.end();++it)
			browseaddress_->insertItem(*it);
	}
	if (conf->browseport_ != -1)
	{
		opt_[1]->setDefault(false);
		browseport_->setText(QString::number(conf->browseport_));
	}
	if (conf->browsepoll_.count() > 0)
	{
		opt_[2]->setDefault(false);
		for (it=conf->browsepoll_.begin();it!=conf->browsepoll_.end();++it)
			browsepoll_->insertItem(*it);
	}
	return true;
}

bool CupsdBrowsingConnPage::saveConfig(CupsdConf *conf, QString& msg)
{
	if (!opt_[0]->isDefault() && browseaddress_->count() > 0)
	{
		conf->browseaddress_.clear();
		for (int i=0;i<browseaddress_->count();i++)
			conf->browseaddress_.append(browseaddress_->text(i));
	}
	if (!opt_[1]->isDefault() && !browseport_->text().isNull())
	{
		bool	ok;
		int	val = browseport_->text().toInt(&ok);
		if (ok) conf->browseport_ = val;
		else
		{
			msg = i18n("%1 wrong argument").arg(i18n("Browse port:"));
			return false;
		}
	}
	if (!opt_[2]->isDefault() && browsepoll_->count() > 0)
	{
		conf->browsepoll_.clear();
		for (int i=0;i<browsepoll_->count();i++)
			conf->browsepoll_.append(browsepoll_->text(i));
	}
	return true;
}

void CupsdBrowsingConnPage::setDefaults()
{
	browseport_->setText(QString::number(631));
}

void CupsdBrowsingConnPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(browseport_, conf->comments_.toolTip(BROWSEPORT_COMM));
        QWhatsThis::add(browseaddress_, conf->comments_.toolTip(BROWSEADDRESS_COMM));
        QWhatsThis::add(browsepoll_, conf->comments_.toolTip(BROWSEPOLL_COMM));
}
