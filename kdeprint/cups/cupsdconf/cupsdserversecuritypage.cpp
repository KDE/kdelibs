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

#include "cupsdserversecuritypage.h"

#include <qlineedit.h>
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kseparator.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"
#include "cupsdoption.h"
#include "cupslocationdialog.h"

CupsdServerSecurityPage::CupsdServerSecurityPage(QWidget *parent, const char *name)
	: CupsdPage(parent, name)
{
	path_.append(i18n("Security"));
	header_ = i18n("Security configuration");

	for (int i=0;i<1;i++)
		opt_[i] = new CupsdOption(this);

	QVBoxLayout	*main_ = new QVBoxLayout(this, 10, 10);
	main_->addWidget(deflabel_, 0, Qt::AlignRight|Qt::AlignVCenter);

	QHBoxLayout	*sub1_ = new QHBoxLayout(0, 0, 10);
	QLabel	*l1 = new QLabel(i18n("System group:"), this);
	systemgroup_ = new QLineEdit(opt_[0]);
	main_->addLayout(sub1_);
	sub1_->addWidget(l1);
	sub1_->addWidget(opt_[0]);

        KSeparator* sep = new KSeparator( KSeparator::HLine, this);
	sep->setFixedHeight(20);
	main_->addWidget(sep);

	locations_ = new QListView(this);
	locations_->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	locations_->setLineWidth(1);
	locations_->addColumn(i18n("Resource"));
	locations_->addColumn(i18n("Path"));
	locations_->setSorting(1);

	QLabel	*l2 = new QLabel(i18n("Resources:"), this);

	QPushButton	*add_ = new QPushButton(i18n("Add..."), this);
	connect(add_, SIGNAL(clicked()), SLOT(addClicked()));
	QPushButton	*modify_ = new QPushButton(i18n("Modify"), this);
	connect(modify_, SIGNAL(clicked()), SLOT(modifyClicked()));
	QPushButton	*remove_ = new QPushButton(i18n("Remove"), this);
	connect(remove_, SIGNAL(clicked()), SLOT(removeClicked()));

	QGridLayout	*sub2_ = new QGridLayout(0, 3, 3, 0, 10);
	main_->addLayout(sub2_);
	sub2_->addWidget(l2, 0, 0, Qt::AlignLeft|Qt::AlignTop);
	sub2_->addMultiCellWidget(locations_, 0, 2, 1, 1);
	sub2_->addWidget(add_, 0, 2);
	sub2_->addWidget(modify_, 1, 2);
	sub2_->addWidget(remove_, 2, 2);

	main_->addStretch(1);

	loclist_.setAutoDelete(true);
}

CupsdServerSecurityPage::~CupsdServerSecurityPage()
{
}

void CupsdServerSecurityPage::updateLocations()
{
	locations_->clear();
	for (loclist_.first();loclist_.current();loclist_.next())
	{
		QListViewItem	*item = new QListViewItem(locations_, (loclist_.current()->resource_ ? loclist_.current()->resource_->text_ : i18n("<Unmatched resource>")), loclist_.current()->resourcename_);
		if (loclist_.current()->resource_) item->setPixmap(0, SmallIcon(CupsResource::typeToIconName(loclist_.current()->resource_->type_)));
		else item->setPixmap(0, SmallIcon(""));
	}
}

bool CupsdServerSecurityPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	if (!conf->systemgroup_.isNull())
	{
		opt_[0]->setDefault(0);
		systemgroup_->setText(conf->systemgroup_);
	}
	loclist_.clear();
	for (conf->locations_.first();conf->locations_.current();conf->locations_.next())
		loclist_.append(new CupsLocation(*(conf->locations_.current())));
	updateLocations();
	return true;
}

bool CupsdServerSecurityPage::saveConfig(CupsdConf *conf, QString&)
{
	if (!opt_[0]->isDefault() && !systemgroup_->text().isNull()) conf->systemgroup_ = systemgroup_->text();
	conf->locations_.clear();
	for (loclist_.first();loclist_.current();loclist_.next())
		conf->locations_.append(new CupsLocation(*(loclist_.current())));
	return true;
}

void CupsdServerSecurityPage::setDefaults()
{
	systemgroup_->setText("sys");
	loclist_.clear();
	updateLocations();
}

void CupsdServerSecurityPage::addClicked()
{
	CupsLocationDialog	dlg(conf_, this);
	if (dlg.exec())
	{
		CupsLocation	*loc = new CupsLocation;
		loclist_.append(loc);
		dlg.saveLocation(loc);
		updateLocations();
	}
}

void CupsdServerSecurityPage::modifyClicked()
{
	if (locations_->currentItem() != NULL)
	{
		for (loclist_.first();loclist_.current();loclist_.next())
			if (loclist_.current()->resourcename_ == locations_->currentItem()->text(1)) break;
		if (!loclist_.current()) return;
		CupsLocation	*loc = loclist_.current();
		CupsLocationDialog	dlg(conf_, this);
		dlg.loadLocation(loc);
		if (dlg.exec())
		{
			dlg.saveLocation(loc);
//			updateLocations();
//			locations_->setSelected(index, true);
		}
	}
}

void CupsdServerSecurityPage::removeClicked()
{
	if (!locations_->currentItem()) return;
	for (loclist_.first();loclist_.current();loclist_.next())
		if (loclist_.current()->resourcename_ == locations_->currentItem()->text(1)) break;
	if (!loclist_.current()) return;
	if (KMessageBox::warningYesNo(this, i18n("Really remove resource \"%1\"?").arg(loclist_.current()->resourcename_)) == KMessageBox::Yes)
	{
		loclist_.remove();
		delete locations_->currentItem();
//		updateLocations();
		// locations_->removeItem(index);
	}
}

void CupsdServerSecurityPage::setInfos(CupsdConf *conf)
{
        QWhatsThis::add(systemgroup_, conf->comments_.toolTip(SYSTEMGROUP_COMM));
        QWhatsThis::add(locations_, conf->comments_.toolTip(LOCATIONS_COMM));
}
#include "cupsdserversecuritypage.moc"
