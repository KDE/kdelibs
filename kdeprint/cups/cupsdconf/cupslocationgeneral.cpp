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

#include "cupslocationgeneral.h"

#include <qlineedit.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <klocale.h>
#include <kiconloader.h>
#include <qwhatsthis.h>

#include "cupsdconf.h"

CupsLocationGeneral::CupsLocationGeneral(CupsdConf *conf, QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	conf_ = conf;

	resource_ = new QComboBox(this);
	//int	h = resource_->sizeHint().height();
	for (conf_->resources_.first();conf_->resources_.current();conf_->resources_.next())
		resource_->insertItem(SmallIcon(CupsResource::typeToIconName(conf_->resources_.current()->type_)), conf_->resources_.current()->text_);

	authtype_ = new QComboBox(this);
	authtype_->insertItem(i18n("None"));
	authtype_->insertItem(i18n("Basic"));
	authtype_->insertItem(i18n("Digest"));
	authtype_->setCurrentItem(0);
	connect(authtype_, SIGNAL(highlighted(int)), SLOT(authTypeChanged(int)));
	authclass_ = new QComboBox(this);
	authclass_->insertItem(i18n("Anonymous"));
	authclass_->insertItem(i18n("User"));
	authclass_->insertItem(i18n("System"));
	authclass_->insertItem(i18n("Group"));
	authclass_->setCurrentItem(1);
	authclass_->setEnabled(false);
	connect(authclass_, SIGNAL(highlighted(int)), SLOT(authClassChanged(int)));
	authgroupname_ = new QLineEdit(this);
	authgroupname_->setEnabled(false);
        encryption_ = new QComboBox(this);
        encryption_->insertItem(i18n("Always"));
        encryption_->insertItem(i18n("Never"));
        encryption_->insertItem(i18n("Required"));
        encryption_->insertItem(i18n("If requested"));
        encryption_->setCurrentItem(3);

	QLabel	*l1 = new QLabel(i18n("Resource:"), this);
	QLabel	*l2 = new QLabel(i18n("Authorization type:"), this);
	QLabel	*l3 = new QLabel(i18n("Authorization class:"), this);
	QLabel	*l4 = new QLabel(i18n("Authorization group:"), this);
	QLabel	*l5 = new QLabel(i18n("Encryption type:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 7, 2, 10, 10);
	main_->addWidget(l1, 0, 0);
	main_->addWidget(l2, 2, 0);
	main_->addWidget(l3, 3, 0);
	main_->addWidget(l4, 4, 0);
	main_->addWidget(l5, 5, 0);
	main_->addWidget(resource_, 0, 1);
	main_->addWidget(authtype_, 2, 1);
	main_->addWidget(authclass_, 3, 1);
	main_->addWidget(authgroupname_, 4, 1);
	main_->addWidget(encryption_, 5, 1);
	main_->addRowSpacing(1, 20);
	main_->setRowStretch(6, 1);
}

CupsLocationGeneral::~CupsLocationGeneral()
{
}

void CupsLocationGeneral::loadLocation(CupsLocation *loc)
{
//	resource_->setText(loc->resource_);
	// can't change resource name
	resource_->setEnabled(false);
	if (!loc->resource_) resource_->setCurrentItem(-1);
	else
	{
		int	index = conf_->resources_.findRef(loc->resource_);
		resource_->setCurrentItem(index);
	}
	if (loc->authtype_ != -1)
	{
		authtype_->setCurrentItem(loc->authtype_);
	}
	if (loc->authclass_ != -1)
	{
		authclass_->setCurrentItem(loc->authclass_);
	}
        if (loc->encryption_ != -1)
        {
        	encryption_->setCurrentItem(loc->encryption_);
        }
	authgroupname_->setText(loc->authgroupname_);
}

void CupsLocationGeneral::saveLocation(CupsLocation *loc)
{
	if (resource_->currentItem() != -1)
	{
		loc->resource_ = conf_->resources_.at(resource_->currentItem());
		loc->resourcename_ = loc->resource_->path_;
	}
	loc->authtype_ = authtype_->currentItem();
	loc->authclass_ = authclass_->currentItem();
	loc->authgroupname_ = authgroupname_->text();
        loc->encryption_ = encryption_->currentItem();
}

void CupsLocationGeneral::authTypeChanged(int index)
{
	if (index != -1)
	{
		authclass_->setEnabled(index != AUTHTYPE_NONE);
		authgroupname_->setEnabled((index != AUTHTYPE_NONE) && (authclass_->currentItem() == AUTHCLASS_GROUP));
	}
}

void CupsLocationGeneral::authClassChanged(int index)
{
	if (index != -1)
	{
		authgroupname_->setEnabled(index == AUTHCLASS_GROUP);
	}
}

bool CupsLocationGeneral::isValid()
{
//	return !resource_->text().isEmpty();
	return true;
}

void CupsLocationGeneral::setInfos(CupsdConf *conf)
{
	QWhatsThis::add(authtype_, conf->comments_.toolTip(LOCAUTHTYPE_COMM));
	QWhatsThis::add(authclass_, conf->comments_.toolTip(LOCAUTHCLASS_COMM));
	QWhatsThis::add(authgroupname_, conf->comments_.toolTip(LOCAUTHGROUPNAME_COMM));
	QWhatsThis::add(encryption_, conf->comments_.toolTip(LOCENCRYPTION_COMM));
}
#include "cupslocationgeneral.moc"
