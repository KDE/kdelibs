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

#include "addressdialog.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>

AddressDialog::AddressDialog(QWidget *parent, const char *name)
	: KDialogBase(Swallow, i18n("ACL Address"), Ok|Cancel, Ok, parent, name, true, true)
{
	QWidget *w = new QWidget(this);
	type_ = new QComboBox(w);
	address_ = new QLineEdit(w);

	type_->insertItem(i18n("Allow"));
	type_->insertItem(i18n("Deny"));

	QLabel	*l1 = new QLabel(i18n("Type:"), w);
	QLabel	*l2 = new QLabel(i18n("Address:"), w);

	QGridLayout	*m1 = new QGridLayout(w, 2, 2, 0, 5);
	m1->setColStretch(1, 1);
	m1->addWidget(l1, 0, 0, Qt::AlignRight);
	m1->addWidget(l2, 1, 0, Qt::AlignRight);
	m1->addWidget(type_, 0, 1);
	m1->addWidget(address_, 1, 1);

	setMainWidget(w);
	resize(300, 100);
}

QString AddressDialog::addressString()
{
	QString s;
	if (type_->currentItem() == 0)
		s.append("Allow ");
	else
		s.append("Deny ");
	if (address_->text().isEmpty())
		s.append("All");
	else
		s.append(address_->text());
	return s;
}

QString AddressDialog::newAddress(QWidget *parent)
{
	AddressDialog	dlg(parent);
	if (dlg.exec())
		return dlg.addressString();
	else
		return QString::null;
}

QString AddressDialog::editAddress(const QString& addr, QWidget *parent)
{
	AddressDialog	dlg(parent);
	int p = addr.find(' ');
	if (p != -1)
	{
		dlg.type_->setCurrentItem(addr.left(p).lower() == "deny" ? 1 : 0);
		dlg.address_->setText(addr.mid(p+1));
	}
	if (dlg.exec())
		return dlg.addressString();
	else
		return QString::null;
}
