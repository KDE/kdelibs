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

#include "cupsdservermiscpage.h"

#include "qdirlineedit.h"
#include <qlineedit.h>
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>

#include <kseparator.h>

#include "cupsdconf.h"
#include "cupsdoption.h"

CupsdServerMiscPage::CupsdServerMiscPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Server"));
	path_.append(i18n("Misc"));
	header_ = i18n("Server miscellaneous configuration");

	for (int i=0;i<5;i++)
		opt_[i] = new CupsdOption(this);

	preservejobhistory_ = new QCheckBox(i18n("Preserve job history (after completion)"), opt_[0]);
	preservejobfiles_ = new QCheckBox(i18n("Preserve job file (after completion)"), opt_[1]);

        KSeparator* sep = new KSeparator( KSeparator::HLine, this);

	printcap_ = new QDirLineEdit(opt_[2]);
	printcap_->setFileEdit(true);
	ripcache_ = new QLineEdit(opt_[3]);
	filterlimit_ = new QLineEdit(opt_[4]);

	QLabel	*l1 = new QLabel(i18n("Printcap file:"), this);
	QLabel	*l2 = new QLabel(i18n("RIP cache:"), this);
	QLabel	*l3 = new QLabel(i18n("Filter limit:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 8, 2, 10, 10);
	main_->addWidget(deflabel_, 0, 1, Qt::AlignRight|Qt::AlignVCenter);
	main_->addMultiCellWidget(opt_[0], 1, 1, 0, 1);
	main_->addMultiCellWidget(opt_[1], 2, 2, 0, 1);
	main_->addMultiCellWidget(sep, 3, 3, 0, 1);
        main_->addRowSpacing(3, 10);
	main_->addWidget(opt_[2], 4, 1);
	main_->addWidget(opt_[3], 5, 1);
	main_->addWidget(opt_[4], 6, 1);
	main_->addWidget(l1, 4, 0);
	main_->addWidget(l2, 5, 0);
	main_->addWidget(l3, 6, 0);
	main_->setRowStretch(7, 1);
}

CupsdServerMiscPage::~CupsdServerMiscPage()
{
}

bool CupsdServerMiscPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	if (conf->preservejobhistory_ != -1)
	{
		opt_[0]->setDefault(false);
		preservejobhistory_->setChecked(conf->preservejobhistory_ == 1);
	}
	if (conf->preservejobfiles_ != -1)
	{
		opt_[1]->setDefault(false);
		preservejobfiles_->setChecked(conf->preservejobfiles_ == 1);
	}
	if (!conf->printcap_.isNull())
	{
		opt_[2]->setDefault(false);
		printcap_->setText(conf->printcap_);
	}
	if (!conf->ripcache_.isNull())
	{
		opt_[3]->setDefault(false);
		ripcache_->setText(conf->ripcache_);
	}
	if (conf->filterlimit_ != -1)
	{
		opt_[4]->setDefault(false);
		filterlimit_->setText(QString::number(conf->filterlimit_));
	}
	return true;
}

bool CupsdServerMiscPage::saveConfig(CupsdConf *conf, QString& msg)
{
	if (!opt_[0]->isDefault()) conf->preservejobhistory_ = (preservejobhistory_->isChecked() ? 1 : 0);
	if (!opt_[1]->isDefault()) conf->preservejobfiles_ = (preservejobfiles_->isChecked() ? 1 : 0);
	if (!opt_[2]->isDefault() && !printcap_->text().isNull()) conf->printcap_ = printcap_->text();
	if (!opt_[3]->isDefault() && !ripcache_->text().isNull()) conf->ripcache_ = ripcache_->text();
	if (!opt_[4]->isDefault())
	{
		bool	ok;
		int	value = filterlimit_->text().toInt(&ok);
		if (ok) conf->filterlimit_ = value;
		else
		{
			msg = i18n("%1 wrong argument").arg(i18n("Filter limit:"));
			return false;
		}
	}
	return true;
}

void CupsdServerMiscPage::setDefaults()
{
	preservejobhistory_->setChecked(true);
	preservejobfiles_->setChecked(false);
	printcap_->setText("/etc/printcap");
	ripcache_->setText("8m");
	filterlimit_->setText("0");
}

void CupsdServerMiscPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(preservejobhistory_, conf->comments_.toolTip(PRESERVEJOBHIST_COMM));
        QWhatsThis::add(preservejobfiles_, conf->comments_.toolTip(PRESERVEJOBFILE_COMM));
        QWhatsThis::add(printcap_, conf->comments_.toolTip(PRINTCAP_COMM));
        QWhatsThis::add(ripcache_, conf->comments_.toolTip(RIPCACHE_COMM));
        QWhatsThis::add(filterlimit_, conf->comments_.toolTip(FILTERLIMIT_COMM));
}
