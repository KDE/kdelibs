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

#include "cupsdserverjobpage.h"

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

CupsdServerJobPage::CupsdServerJobPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Server"));
	path_.append(i18n("Jobs"));
	header_ = i18n("Server jobs configuration");

	for (int i=0;i<7;i++)
		opt_[i] = new CupsdOption(this);

	preservejobhistory_ = new QCheckBox(i18n("Preserve job history (after completion)"), opt_[0]);
	preservejobfiles_ = new QCheckBox(i18n("Preserve job file (after completion)"), opt_[1]);
	autopurgejobs_ = new QCheckBox(i18n("Remove jobs if no quotas"), opt_[2]);
	maxjobs_ = new QLineEdit(opt_[3]);
	filterlimit_ = new QLineEdit(opt_[4]);

        KSeparator* sep = new KSeparator( KSeparator::HLine, this);

	classification_ = new QLineEdit(opt_[5]);
	classifyoverride_ = new QCheckBox(i18n("Allow classification override"), opt_[6]);

	QLabel	*l1 = new QLabel(i18n("Jobs to keep in memory:"), this);
	QLabel	*l2 = new QLabel(i18n("Filter limit:"), this);
	QLabel	*l3 = new QLabel(i18n("Default classification:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 10, 2, 10, 10);
	main_->addWidget(deflabel_, 0, 1, Qt::AlignRight|Qt::AlignVCenter);
	main_->addMultiCellWidget(opt_[0], 1, 1, 0, 1);
	main_->addMultiCellWidget(opt_[1], 2, 2, 0, 1);
	main_->addMultiCellWidget(opt_[2], 3, 3, 0, 1);
	main_->addWidget(opt_[3], 4, 1);
	main_->addWidget(opt_[4], 5, 1);
	main_->addWidget(l1, 4, 0);
	main_->addWidget(l2, 5, 0);
	main_->addMultiCellWidget(sep, 6, 6, 0, 1);
	main_->addWidget(opt_[5], 7, 1);
	main_->addWidget(l3, 7, 0);
	main_->addMultiCellWidget(opt_[6], 8, 8, 0, 1);
	main_->setRowStretch(9, 1);
}

CupsdServerJobPage::~CupsdServerJobPage()
{
}

bool CupsdServerJobPage::loadConfig(CupsdConf *conf, QString&)
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
	if (conf->autopurgejobs_ != -1)
	{
		opt_[2]->setDefault(false);
		autopurgejobs_->setChecked(conf->autopurgejobs_ == 1);
	}
	if (conf->maxjobs_ != -1)
	{
		opt_[3]->setDefault(false);
		maxjobs_->setText(QString::number(conf->maxjobs_));
	}
	if (conf->filterlimit_ != -1)
	{
		opt_[4]->setDefault(false);
		filterlimit_->setText(QString::number(conf->filterlimit_));
	}
	if (!conf->classification_.isNull())
	{
		opt_[5]->setDefault(false);
		classification_->setText(conf->classification_);
	}
	if (conf->classifyoverride_ != -1)
	{
		opt_[6]->setDefault(false);
		classifyoverride_->setChecked(conf->classifyoverride_ == 1);
	}
	return true;
}

bool CupsdServerJobPage::saveConfig(CupsdConf *conf, QString& msg)
{
	if (!opt_[0]->isDefault()) conf->preservejobhistory_ = (preservejobhistory_->isChecked() ? 1 : 0);
	if (!opt_[1]->isDefault()) conf->preservejobfiles_ = (preservejobfiles_->isChecked() ? 1 : 0);
	if (!opt_[2]->isDefault()) conf->autopurgejobs_ = (autopurgejobs_->isChecked() ? 1 : 0);
	if (!opt_[3]->isDefault())
	{
		bool	ok;
		int	value = maxjobs_->text().toInt(&ok);
		if (ok) conf->maxjobs_ = value;
		else
		{
			msg = i18n("%1 wrong argument").arg(i18n("Job to keep in memory:"));
			return false;
		}
	}
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
	if (!opt_[5]->isDefault() && !classification_->text().isNull()) conf->classification_ = classification_->text();
	if (!opt_[6]->isDefault()) conf->classifyoverride_ = (classifyoverride_->isChecked() ? 1 : 0);
	return true;
}

void CupsdServerJobPage::setDefaults()
{
	preservejobhistory_->setChecked(true);
	preservejobfiles_->setChecked(false);
	autopurgejobs_->setChecked(false);
	maxjobs_->setText("0");
	filterlimit_->setText("0");
	classification_->setText(QString::null);
	classifyoverride_->setChecked(false);
}

void CupsdServerJobPage::setInfos(CupsdConf *conf)
{
	QWhatsThis::add(preservejobhistory_, conf->comments_.toolTip(PRESERVEJOBHIST_COMM));
        QWhatsThis::add(preservejobfiles_, conf->comments_.toolTip(PRESERVEJOBFILE_COMM));
        QWhatsThis::add(autopurgejobs_, conf->comments_.toolTip(AUTOPURGEJOBS_COMM));
        QWhatsThis::add(maxjobs_, conf->comments_.toolTip(MAXJOBS_COMM));
        QWhatsThis::add(filterlimit_, conf->comments_.toolTip(FILTERLIMIT_COMM));
        QWhatsThis::add(classification_, conf->comments_.toolTip(CLASSIFICATION_COMM));
        QWhatsThis::add(classifyoverride_, conf->comments_.toolTip(CLASSIFYOVERRIDE_COMM));
}
