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

#include "cupsdnetworkgeneralpage.h"

#include <klocale.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupsdoption.h"
#include "cupslist.h"

CupsdNetworkGeneralPage::CupsdNetworkGeneralPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Network"));
	header_ = i18n("Network general configuration");

	for (int i=0;i<4;i++)
		opt_[i] = new CupsdOption(this);

	port_ = new CupsListBox(opt_[0]);
	hostnamelookups_ = new QCheckBox(i18n("Look for hostname on IP addresses"), opt_[1]);
	maxrequestsize_ = new QLineEdit(opt_[2]);
	timeout_ = new QLineEdit(opt_[3]);

	QLabel	*l1 = new QLabel(i18n("Port:"), this);
	QLabel	*l3 = new QLabel(i18n("Max request size:"), this);
	QLabel	*l4 = new QLabel(i18n("Timeout:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 8, 2, 10, 10);
	main_->addWidget(deflabel_, 0, 1, Qt::AlignRight|Qt::AlignVCenter);
	main_->addMultiCellWidget(opt_[1], 1, 1, 0, 1);
	main_->addMultiCellWidget(opt_[0], 2, 3, 1, 1);
	main_->addWidget(opt_[2], 5, 1);
	main_->addWidget(opt_[3], 6, 1);
	main_->addWidget(l1, 2, 0, Qt::AlignLeft|Qt::AlignTop);
	main_->addWidget(l3, 5, 0);
	main_->addWidget(l4, 6, 0);
	main_->addRowSpacing(4, 20);
	main_->setRowStretch(7, 1);
}

CupsdNetworkGeneralPage::~CupsdNetworkGeneralPage()
{
}

bool CupsdNetworkGeneralPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	QValueList<int>::Iterator	it;
	if (conf->port_.count() > 0)
	{
		opt_[0]->setDefault(false);
		for (it=conf->port_.begin();it!=conf->port_.end();++it)
			port_->insertItem(QString::number(*it));
	}
	if (conf->hostnamelookups_ != -1)
	{
		opt_[1]->setDefault(false);
		hostnamelookups_->setChecked(conf->hostnamelookups_ == 1);
	}
	if (conf->maxrequestsize_ != -1)
	{
		opt_[2]->setDefault(false);
		maxrequestsize_->setText(QString::number(conf->maxrequestsize_));
	}
	if (conf->timeout_ != -1)
	{
		opt_[3]->setDefault(false);
		timeout_->setText(QString::number(conf->timeout_));
	}
	return true;
}

bool CupsdNetworkGeneralPage::saveConfig(CupsdConf *conf, QString& msg)
{
	bool	ok;
	int	p;
	if (!opt_[0]->isDefault() && port_->count() > 0)
	{
		conf->port_.clear();
		for (int i=0;i<port_->count();i++)
		{
			p = port_->text(i).toInt(&ok);
			if (ok) conf->port_.append(p);
			else
			{
				msg = i18n("%1 wrong argument").arg(i18n("Port:"));
				return false;
			}
		}
	}
	if (!opt_[1]->isDefault()) conf->hostnamelookups_ = (hostnamelookups_->isChecked() ? 1 : 0);
	if (!opt_[2]->isDefault() && !maxrequestsize_->text().isNull())
	{
		p = maxrequestsize_->text().toInt(&ok);
		if (ok) conf->maxrequestsize_ = p;
		else
		{
			msg = i18n("%1 wrong argument").arg(i18n("Max request size:"));
			return false;
		}
	}
	if (!opt_[3]->isDefault() && !timeout_->text().isNull())
	{
		p = timeout_->text().toInt(&ok);
		if (ok) conf->timeout_ = p;
		else
		{
			msg = i18n("%1 wrong argument").arg(i18n("Timeout:"));
			return false;
		}
	}
	return true;
}

void CupsdNetworkGeneralPage::setDefaults()
{
	hostnamelookups_->setChecked(false);
	maxrequestsize_->setText(QString::number(0));
	timeout_->setText(QString::number(300));
}

void CupsdNetworkGeneralPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(hostnamelookups_, conf->comments_.toolTip(HOSTNAMELOOKUPS_COMM));
        QWhatsThis::add(maxrequestsize_, conf->comments_.toolTip(MAXREQUESTSIZE_COMM));
        QWhatsThis::add(timeout_, conf->comments_.toolTip(TIMEOUT_COMM));
        QWhatsThis::add(port_, conf->comments_.toolTip(PORT_COMM));
}
