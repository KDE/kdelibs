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

#include "cupsdbrowsingrelaypage.h"

#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupsdoption.h"
#include "cupsrelay.h"

CupsdBrowsingRelayPage::CupsdBrowsingRelayPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Browsing"));
	path_.append(i18n("Relay"));
	header_ = i18n("Browsing relay configuration");

	QVBoxLayout	*main_ = new QVBoxLayout(this, 10, 10);

	for (int i=0;i<1;i++)
		opt_[i] = new CupsdOption(this);

	browserelay_ = new CupsRelay(opt_[0]);

	QLabel	*l1 = new QLabel(i18n("Browser packets relay:"), this);

	main_->addWidget(deflabel_, 0, Qt::AlignRight|Qt::AlignVCenter);
	main_->addWidget(l1);
	main_->addWidget(opt_[0]);
	main_->addStretch(1);
}

CupsdBrowsingRelayPage::~CupsdBrowsingRelayPage()
{
}

bool CupsdBrowsingRelayPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	QStringList::Iterator	it;
	if (conf->browserelay_.count() > 0)
	{
		opt_[0]->setDefault(false);
		for (it=conf->browserelay_.begin();it!=conf->browserelay_.end();++it)
		{
			QString	from, to;
			from = (*it);
			++it;
			to = (*it);
			browserelay_->addRelay(from,to);
		}
	}
	return true;
}

bool CupsdBrowsingRelayPage::saveConfig(CupsdConf *conf, QString&)
{
	if (!opt_[0]->isDefault() && browserelay_->count() > 0)
	{
		conf->browserelay_.clear();
		for (int i=0;i<browserelay_->count();i++)
		{
			conf->browserelay_.append(browserelay_->relayFrom(i));
			conf->browserelay_.append(browserelay_->relayTo(i));
		}
	}
	return true;
}

void CupsdBrowsingRelayPage::setDefaults()
{
}

void CupsdBrowsingRelayPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(browserelay_, conf->comments_.toolTip(BROWSERELAY_COMM));
}
