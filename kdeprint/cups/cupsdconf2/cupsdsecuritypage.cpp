/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "cupsdsecuritypage.h"
#include "cupsdconf.h"
#include "qdirlineedit.h"
#include "editlist.h"
#include "locationdialog.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>

CupsdSecurityPage::CupsdSecurityPage(QWidget *parent)
	: CupsdPage(parent)
{
	setPageLabel(i18n("Security"));
	setHeader(i18n("Security Settings"));
	setPixmap("password");
	locs_.setAutoDelete(true);

	remoteroot_ = new QLineEdit(this);
	systemgroup_ = new QLineEdit(this);
	encryptcert_ = new QDirLineEdit(true, this);
	encryptkey_ = new QDirLineEdit(true, this);
	locations_ = new EditList(this);

	QLabel *l1 = new QLabel(i18n("Remote root user:"), this);
	QLabel *l2 = new QLabel(i18n("System group:"), this);
	QLabel *l3 = new QLabel(i18n("Encryption certificate:"), this);
	QLabel *l4 = new QLabel(i18n("Encryption key:"), this);
	QLabel *l5 = new QLabel(i18n("Locations:"), this);

	QGridLayout	*m1 = new QGridLayout(this, 6, 2, 10, 7);
	m1->setRowStretch(5, 1);
	m1->setColStretch(1, 1);
	m1->addWidget(l1, 0, 0, Qt::AlignRight);
	m1->addWidget(l2, 1, 0, Qt::AlignRight);
	m1->addWidget(l3, 2, 0, Qt::AlignRight);
	m1->addWidget(l4, 3, 0, Qt::AlignRight);
	m1->addWidget(l5, 4, 0, Qt::AlignRight|Qt::AlignTop);
	m1->addWidget(remoteroot_, 0, 1);
	m1->addWidget(systemgroup_, 1, 1);
	m1->addWidget(encryptcert_, 2, 1);
	m1->addWidget(encryptkey_, 3, 1);
	m1->addWidget(locations_, 4, 1);

	connect(locations_, SIGNAL(add()), SLOT(slotAdd()));
	connect(locations_, SIGNAL(edit(int)), SLOT(slotEdit(int)));
	connect(locations_, SIGNAL(defaultList()), SLOT(slotDefaultList()));
	connect(locations_, SIGNAL(deleted(int)), SLOT(slotDeleted(int)));
}

bool CupsdSecurityPage::loadConfig(CupsdConf *conf, QString&)
{
	conf_ = conf;
	remoteroot_->setText(conf_->remoteroot_);
	systemgroup_->setText(conf_->systemgroup_);
	encryptcert_->setURL(conf_->encryptcert_);
	encryptkey_->setURL(conf_->encryptkey_);
	locs_.clear();
	Q3PtrListIterator<CupsLocation>	it(conf_->locations_);
	for (;it.current();++it)
	{
		locs_.append(new CupsLocation(*(it.current())));
		if (it.current()->resource_)
			locations_->insertItem(SmallIcon(CupsResource::typeToIconName(it.current()->resource_->type_)), it.current()->resource_->text_);
		else
			locations_->insertItem(it.current()->resourcename_);
	}

	return true;
}

bool CupsdSecurityPage::saveConfig(CupsdConf *conf, QString&)
{
	conf->remoteroot_ = remoteroot_->text();
	conf->systemgroup_ = systemgroup_->text();
	conf->encryptcert_ = encryptcert_->url();
	conf->encryptkey_ = encryptkey_->url();
	conf->locations_.clear();
	Q3PtrListIterator<CupsLocation>	it(locs_);
	for (;it.current();++it)
		conf->locations_.append(new CupsLocation(*(it.current())));

	return true;
}

void CupsdSecurityPage::setInfos(CupsdConf *conf)
{
	remoteroot_->setWhatsThis(conf->comments_.toolTip("remoteroot"));
	systemgroup_->setWhatsThis(conf->comments_.toolTip("systemgroup"));
	encryptcert_->setWhatsThis(conf->comments_.toolTip("servercertificate"));
	encryptkey_->setWhatsThis(conf->comments_.toolTip("serverkey"));
	locations_->setWhatsThis(conf->comments_.toolTip("locationsshort"));
}

void CupsdSecurityPage::slotAdd()
{
	CupsLocation	*loc = new CupsLocation;
	if (LocationDialog::newLocation(loc, this, conf_))
	{
		int index(-1);
		for (locs_.first(); locs_.current(); locs_.next())
			if (locs_.current()->resource_ == loc->resource_)
			{
				if (KMessageBox::warningContinueCancel(this, i18n("This location is already defined. Do you want to replace the existing one?"),QString(),i18n("Replace")) == KMessageBox::Continue)
				{
					index = locs_.at();
					locs_.remove();
					break;
				}
				else
				{
					delete loc;
					return;
				}
			}

		if (index == -1)
			index = locs_.count();
		locs_.insert(index, loc);
		locations_->insertItem(SmallIcon(loc->resource_->typeToIconName(loc->resource_->type_)), loc->resource_->text_);
	}
	else
		delete loc;
}

void CupsdSecurityPage::slotEdit(int index)
{
	CupsLocation *loc = locs_.at(index);
	LocationDialog::editLocation(loc, this, conf_);
}

void CupsdSecurityPage::slotDefaultList()
{
	locs_.clear();
	locations_->clear();
}

void CupsdSecurityPage::slotDeleted(int index)
{
	if (index >= 0 && index < (int)(locs_.count()))
		locs_.remove(index);
}

#include "cupsdsecuritypage.moc"
