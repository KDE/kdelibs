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

#include "locationdialog.h"
#include "cupsdconf.h"
#include "editlist.h"
#include "addressdialog.h"

#include <qlineedit.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

#include <klocale.h>
#include <kiconloader.h>

LocationDialog::LocationDialog(QWidget *parent, const char *name)
	: KDialogBase(parent, name, true, QString::null, Ok|Cancel, Ok, true)
{
	QWidget	*dummy = new QWidget(this);
	setMainWidget(dummy);
	resource_ = new QComboBox(dummy);
	authtype_ = new QComboBox(dummy);
	authclass_ = new QComboBox(dummy);
	authname_ = new QLineEdit(dummy);
	encryption_ = new QComboBox(dummy);
	satisfy_ = new QComboBox(dummy);
	order_ = new QComboBox(dummy);
	addresses_ = new EditList(dummy);

	authtype_->insertItem(i18n("None"));
	authtype_->insertItem(i18n("Basic"));
	authtype_->insertItem(i18n("Digest"));

	authclass_->insertItem(i18n("None"));
	authclass_->insertItem(i18n("User"));
	authclass_->insertItem(i18n("System"));
	authclass_->insertItem(i18n("Group"));

	encryption_->insertItem(i18n("Always"));
	encryption_->insertItem(i18n("Never"));
	encryption_->insertItem(i18n("Required"));
	encryption_->insertItem(i18n("If Requested"));

	satisfy_->insertItem(i18n("All"));
	satisfy_->insertItem(i18n("Any"));

	order_->insertItem(i18n("Allow, Deny"));
	order_->insertItem(i18n("Deny, Allow"));

	connect(authclass_, SIGNAL(activated(int)), SLOT(slotClassChanged(int)));
	connect(authtype_, SIGNAL(activated(int)), SLOT(slotTypeChanged(int)));

	QLabel	*l1 = new QLabel(i18n("Resource:"), dummy);
	QLabel	*l2 = new QLabel(i18n("Authentification:"), dummy);
	QLabel	*l3 = new QLabel(i18n("Class:"), dummy);
	QLabel	*l4 = new QLabel(i18n("Names:"), dummy);
	QLabel	*l5 = new QLabel(i18n("Encryption:"), dummy);
	QLabel	*l6 = new QLabel(i18n("Satisfy:"), dummy);
	QLabel	*l7 = new QLabel(i18n("ACL order:"), dummy);
	QLabel	*l8 = new QLabel(i18n("ACL addresses:"),dummy);

	QGridLayout	*m1 = new QGridLayout(dummy, 8, 2, 0, 5);
	m1->setColStretch(1, 1);
	m1->addWidget(l1, 0, 0, Qt::AlignRight);
	m1->addWidget(l2, 1, 0, Qt::AlignRight);
	m1->addWidget(l3, 2, 0, Qt::AlignRight);
	m1->addWidget(l4, 3, 0, Qt::AlignRight);
	m1->addWidget(l5, 4, 0, Qt::AlignRight);
	m1->addWidget(l6, 5, 0, Qt::AlignRight);
	m1->addWidget(l7, 6, 0, Qt::AlignRight);
	m1->addWidget(l8, 7, 0, Qt::AlignRight|Qt::AlignTop);
	m1->addWidget(resource_, 0, 1);
	m1->addWidget(authtype_, 1, 1);
	m1->addWidget(authclass_, 2, 1);
	m1->addWidget(authname_, 3, 1);
	m1->addWidget(encryption_, 4, 1);
	m1->addWidget(satisfy_, 5, 1);
	m1->addWidget(order_, 6, 1);
	m1->addWidget(addresses_, 7, 1);

	setCaption(i18n("Location"));
	resize(400, 100);

	slotTypeChanged(AUTHTYPE_NONE);
	slotClassChanged(AUTHCLASS_ANONYMOUS);
	encryption_->setCurrentItem(ENCRYPT_IFREQUESTED);

	connect(addresses_, SIGNAL(add()), SLOT(slotAdd()));
	connect(addresses_, SIGNAL(edit(int)), SLOT(slotEdit(int)));
	connect(addresses_, SIGNAL(defaultList()), SLOT(slotDefaultList()));
}

void LocationDialog::setInfos(CupsdConf *conf)
{
	conf_ = conf;

	QPtrListIterator<CupsResource>	it(conf->resources_);
	for (; it.current(); ++it)
		resource_->insertItem(SmallIcon(it.current()->typeToIconName(it.current()->type_)), it.current()->text_);

	QWhatsThis::add(encryption_, conf_->comments_.toolTip("encryption"));
	QWhatsThis::add(order_, conf_->comments_.toolTip("order"));
	QWhatsThis::add(authclass_, conf_->comments_.toolTip("authclass"));
	QWhatsThis::add(authtype_, conf_->comments_.toolTip("authtype"));
	QWhatsThis::add(authname_, conf_->comments_.toolTip("authname"));
	QWhatsThis::add(satisfy_, conf_->comments_.toolTip("satisfy"));
	QWhatsThis::add(addresses_, conf_->comments_.toolTip("allowdeny"));
}

void LocationDialog::fillLocation(CupsLocation *loc)
{
	loc->resource_ = conf_->resources_.at(resource_->currentItem());
	loc->resourcename_ = loc->resource_->path_;
	loc->authtype_ = authtype_->currentItem();
	loc->authclass_ = (loc->authtype_ == AUTHTYPE_NONE ? AUTHCLASS_ANONYMOUS : authclass_->currentItem());
	loc->authname_ = (loc->authclass_ == AUTHCLASS_USER || loc->authclass_ == AUTHCLASS_GROUP ? authname_->text() : QString::null);
	loc->encryption_ = encryption_->currentItem();
	loc->satisfy_ = satisfy_->currentItem();
	loc->order_ = order_->currentItem();
	loc->addresses_ = addresses_->items();
}

void LocationDialog::setLocation(CupsLocation *loc)
{
	int	index = conf_->resources_.findRef(loc->resource_);
	resource_->setCurrentItem(index);
	authtype_->setCurrentItem(loc->authtype_);
	authclass_->setCurrentItem(loc->authclass_);
	authname_->setText(loc->authname_);
	encryption_->setCurrentItem(loc->encryption_);
	satisfy_->setCurrentItem(loc->satisfy_);
	order_->setCurrentItem(loc->order_);
	addresses_->insertItems(loc->addresses_);

	slotTypeChanged(loc->authtype_);
	slotClassChanged(loc->authclass_);
}

void LocationDialog::slotTypeChanged(int index)
{
	authclass_->setEnabled(index != AUTHTYPE_NONE);
	if (index != AUTHTYPE_NONE)
		slotClassChanged(authclass_->currentItem());
	else
		authname_->setEnabled(false);
}

void LocationDialog::slotClassChanged(int index)
{
	authname_->setEnabled((index == AUTHCLASS_USER || index == AUTHCLASS_GROUP));
}

bool LocationDialog::newLocation(CupsLocation *loc, QWidget *parent, CupsdConf *conf)
{
	LocationDialog	dlg(parent);
	if (conf)
		dlg.setInfos(conf);
	if (dlg.exec())
	{
		dlg.fillLocation(loc);
		return true;
	}
	else
		return false;
}

bool LocationDialog::editLocation(CupsLocation *loc, QWidget *parent, CupsdConf *conf)
{
	LocationDialog	dlg(parent);
	if (conf)
		dlg.setInfos(conf);
	dlg.setLocation(loc);
	dlg.resource_->setEnabled(false);
	if (dlg.exec())
	{
		dlg.fillLocation(loc);
		return true;
	}
	else
		return false;
}

void LocationDialog::slotAdd()
{
	QString addr = AddressDialog::newAddress(this);
	if (!addr.isEmpty())
		addresses_->insertItem(addr);
}

void LocationDialog::slotEdit(int index)
{
	QString addr = addresses_->text(index);
	addr = AddressDialog::editAddress(addr, this);
	if (!addr.isEmpty())
		addresses_->insertItem(addr);
}

void LocationDialog::slotDefaultList()
{
	addresses_->clear();
}

#include "locationdialog.moc"
