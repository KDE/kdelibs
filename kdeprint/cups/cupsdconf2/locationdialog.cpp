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

#include "locationdialog.h"
#include "cupsdconf.h"
#include "editlist.h"
#include "addressdialog.h"

#include <qlineedit.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <kiconloader.h>

LocationDialog::LocationDialog(QWidget *parent, const char *name)
	: KDialogBase(parent, name, true, QString(), Ok|Cancel, Ok, true)
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

	authtype_->addItem(i18n("None"));
	authtype_->addItem(i18n("Basic"));
	authtype_->addItem(i18n("Digest"));

	authclass_->addItem(i18n("None"));
	authclass_->addItem(i18n("User"));
	authclass_->addItem(i18n("System"));
	authclass_->addItem(i18n("Group"));

	encryption_->addItem(i18n("Always"));
	encryption_->addItem(i18n("Never"));
	encryption_->addItem(i18n("Required"));
	encryption_->addItem(i18n("If Requested"));

	satisfy_->addItem(i18n("All"));
	satisfy_->addItem(i18n("Any"));

	order_->addItem(i18n("Allow, Deny"));
	order_->addItem(i18n("Deny, Allow"));

	connect(authclass_, SIGNAL(activated(int)), SLOT(slotClassChanged(int)));
	connect(authtype_, SIGNAL(activated(int)), SLOT(slotTypeChanged(int)));

	QLabel	*l1 = new QLabel(i18n("Resource:"), dummy);
	QLabel	*l2 = new QLabel(i18n("Authentication:"), dummy);
	QLabel	*l3 = new QLabel(i18n("Class:"), dummy);
	QLabel	*l4 = new QLabel(i18n("Names:"), dummy);
	QLabel	*l5 = new QLabel(i18n("Encryption:"), dummy);
	QLabel	*l6 = new QLabel(i18n("Satisfy:"), dummy);
	QLabel	*l7 = new QLabel(i18n("ACL order:"), dummy);
	QLabel	*l8 = new QLabel(i18n("ACL addresses:"),dummy);

	QGridLayout	*m1 = new QGridLayout(dummy);
  m1->setMargin(0);
  m1->setSpacing(5);
	m1->setColumnStretch(1, 1);
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
	encryption_->setCurrentIndex(ENCRYPT_IFREQUESTED);

	connect(addresses_, SIGNAL(add()), SLOT(slotAdd()));
	connect(addresses_, SIGNAL(edit(int)), SLOT(slotEdit(int)));
	connect(addresses_, SIGNAL(defaultList()), SLOT(slotDefaultList()));
}

void LocationDialog::setInfos(CupsdConf *conf)
{
	conf_ = conf;

	QListIterator<CupsResource*>	it(conf->resources_);
	while (it.hasNext()) {
    CupsResource *resource(it.next());
		resource_->addItem(SmallIcon(resource->typeToIconName(resource->type_)), resource->text_);
  }

	encryption_->setWhatsThis(conf_->comments_.toolTip("encryption"));
	order_->setWhatsThis(conf_->comments_.toolTip("order"));
	authclass_->setWhatsThis(conf_->comments_.toolTip("authclass"));
	authtype_->setWhatsThis(conf_->comments_.toolTip("authtype"));
	authname_->setWhatsThis(conf_->comments_.toolTip("authname"));
	satisfy_->setWhatsThis(conf_->comments_.toolTip("satisfy"));
	addresses_->setWhatsThis(conf_->comments_.toolTip("allowdeny"));
}

void LocationDialog::fillLocation(CupsLocation *loc)
{
	loc->resource_ = conf_->resources_.at(resource_->currentIndex());
	loc->resourcename_ = loc->resource_->path_;
	loc->authtype_ = authtype_->currentIndex();
	loc->authclass_ = (loc->authtype_ == AUTHTYPE_NONE ? AUTHCLASS_ANONYMOUS : authclass_->currentIndex());
	loc->authname_ = (loc->authclass_ == AUTHCLASS_USER || loc->authclass_ == AUTHCLASS_GROUP ? authname_->text() : QString());
	loc->encryption_ = encryption_->currentIndex();
	loc->satisfy_ = satisfy_->currentIndex();
	loc->order_ = order_->currentIndex();
	loc->addresses_ = addresses_->items();
}

void LocationDialog::setLocation(CupsLocation *loc)
{
	int	index = conf_->resources_.indexOf(loc->resource_);
	resource_->setCurrentIndex(index);
	authtype_->setCurrentIndex(loc->authtype_);
	authclass_->setCurrentIndex(loc->authclass_);
	authname_->setText(loc->authname_);
	encryption_->setCurrentIndex(loc->encryption_);
	satisfy_->setCurrentIndex(loc->satisfy_);
	order_->setCurrentIndex(loc->order_);
	addresses_->insertItems(loc->addresses_);

	slotTypeChanged(loc->authtype_);
	slotClassChanged(loc->authclass_);
}

void LocationDialog::slotTypeChanged(int index)
{
	authclass_->setEnabled(index != AUTHTYPE_NONE);
	if (index != AUTHTYPE_NONE)
		slotClassChanged(authclass_->currentIndex());
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
