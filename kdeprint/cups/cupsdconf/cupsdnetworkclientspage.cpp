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

#include "cupsdnetworkclientspage.h"

#include <qlineedit.h>
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupsdoption.h"

CupsdNetworkClientsPage::CupsdNetworkClientsPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Network"));
	path_.append(i18n("Clients"));
	header_ = i18n("Network clients configuration");

	for (int i=0;i<3;i++)
		opt_[i] = new CupsdOption(this);

	keepalive_ = new QCheckBox(i18n("Accept \"Keep Alive\" requests"), opt_[0]);
	keepalivetimeout_ = new QLineEdit(opt_[1]);
	maxclients_ = new QLineEdit(opt_[2]);

	QLabel	*l1 = new QLabel(i18n("Keep alive timeout:"), this);
	QLabel	*l2 = new QLabel(i18n("Max number of clients:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 6, 2, 10, 10);
	main_->addWidget(deflabel_, 0, 1, Qt::AlignRight|Qt::AlignVCenter);
	main_->addMultiCellWidget(opt_[0], 1, 1, 0, 1);
	main_->addWidget(opt_[1], 2, 1);
	main_->addWidget(opt_[2], 4, 1);
	main_->addWidget(l1, 2, 0);
	main_->addWidget(l2, 4, 0);
	main_->setRowStretch(5, 1);
	main_->addRowSpacing(3, 20);
}

CupsdNetworkClientsPage::~CupsdNetworkClientsPage()
{
}

bool CupsdNetworkClientsPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	if (conf->keepalive_ != -1)
	{
		opt_[0]->setDefault(false);
		keepalive_->setChecked(conf->keepalive_ == 1);
	}
	if (conf->keepalivetimeout_ != -1)
	{
		opt_[1]->setDefault(false);
		keepalivetimeout_->setText(QString::number(conf->keepalivetimeout_));
	}
	if (conf->maxclients_ != -1)
	{
		opt_[2]->setDefault(false);
		maxclients_->setText(QString::number(conf->maxclients_));
	}
	return true;
}

bool CupsdNetworkClientsPage::saveConfig(CupsdConf *conf, QString& msg)
{
	bool	ok;
	int	v;
	if (!opt_[0]->isDefault()) conf->keepalive_ = (keepalive_->isChecked() ? 1 : 0);
	if (!opt_[1]->isDefault() && !keepalivetimeout_->text().isNull())
	{
		v = keepalivetimeout_->text().toInt(&ok);
		if (ok) conf->keepalivetimeout_ = v;
		else
		{
			msg = i18n("%1 wrong argument").arg(i18n("Keep alive timeout:"));
			return false;
		}
	}
	if (!opt_[2]->isDefault() && !maxclients_->text().isNull())
	{
		v = maxclients_->text().toInt(&ok);
		if (ok) conf->maxclients_ = v;
		else
		{
			msg = i18n("%1 wrong argument").arg(i18n("Max number of clients:"));
			return false;
		}
	}
	return true;
}

void CupsdNetworkClientsPage::setDefaults()
{
	keepalive_->setChecked(true);
	keepalivetimeout_->setText("60");
	maxclients_->setText("100");
}

void CupsdNetworkClientsPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(keepalive_, conf->comments_.toolTip(KEEPALIVE_COMM));
        QWhatsThis::add(keepalivetimeout_, conf->comments_.toolTip(KEEPALIVETIMEOUT_COMM));
        QWhatsThis::add(maxclients_, conf->comments_.toolTip(MAXCLIENTS_COMM));
}
