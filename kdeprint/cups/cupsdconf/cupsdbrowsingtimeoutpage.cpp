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

#include "cupsdbrowsingtimeoutpage.h"

#include <klocale.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupsdoption.h"

CupsdBrowsingTimeoutPage::CupsdBrowsingTimeoutPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Browsing"));
	path_.append(i18n("Timeouts"));
	header_ = i18n("Browsing timeouts configuration");

	for (int i=0;i<2;i++)
		opt_[i] = new CupsdOption(this);

	browseinterval_ = new QLineEdit(opt_[0]);
	browsetimeout_ = new QLineEdit(opt_[1]);

	QLabel	*l1 = new QLabel(i18n("Browse interval:"), this);
	QLabel	*l2 = new QLabel(i18n("Browse timeout:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 5, 2, 10, 10);
	main_->addWidget(deflabel_, 0, 1, Qt::AlignRight|Qt::AlignVCenter);
	main_->addWidget(opt_[0], 1, 1);
	main_->addWidget(opt_[1], 3, 1);
	main_->addWidget(l1, 1, 0);
	main_->addWidget(l2, 3, 0);
	main_->setRowStretch(4, 1);
	main_->addRowSpacing(2, 20);
}

CupsdBrowsingTimeoutPage::~CupsdBrowsingTimeoutPage()
{
}

bool CupsdBrowsingTimeoutPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	if (conf->browseinterval_ != -1)
	{
		opt_[0]->setDefault(0);
		browseinterval_->setText(QString::number(conf->browseinterval_));
	}
	if (conf->browsetimeout_ != -1)
	{
		opt_[1]->setDefault(0);
		browsetimeout_->setText(QString::number(conf->browsetimeout_));
	}
	return true;
}

bool CupsdBrowsingTimeoutPage::saveConfig(CupsdConf *conf, QString& msg)
{
	bool	ok;
	int	v1(30), v2(300);
	if (!opt_[0]->isDefault())
	{
		v1 = browseinterval_->text().toInt(&ok);
		if (ok) conf->browseinterval_ = v1;
		else
		{
			msg = i18n("%1 wrong argument!").arg(i18n("Browse interval:"));
			return false;
		}
	}
	if (!opt_[1]->isDefault())
	{
		v2 = browsetimeout_->text().toInt(&ok);
		if (ok) conf->browsetimeout_ = v2;
		else
		{
			msg = i18n("%1 wrong argument!").arg(i18n("Browse timeout:"));
			return false;
		}
	}
	if (v2 <= v1)
	{
		msg = i18n("Browse timeout value must be greater than browse interval");
		return false;
	}
	return true;
}

void CupsdBrowsingTimeoutPage::setDefaults()
{
	browseinterval_->setText(QString::number(30));
	browsetimeout_->setText(QString::number(300));
}

void CupsdBrowsingTimeoutPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(browseinterval_, conf->comments_.toolTip(BROWSEINTERVAL_COMM));
        QWhatsThis::add(browsetimeout_, conf->comments_.toolTip(BROWSETIMEOUT_COMM));
}
