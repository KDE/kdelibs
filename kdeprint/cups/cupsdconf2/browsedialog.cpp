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

#include "browsedialog.h"
#include "cupsdconf.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qregexp.h>

#include <klocale.h>

BrowseDialog::BrowseDialog(QWidget *parent, const char *name)
	: KDialogBase(parent, name, true, QString::null, Ok|Cancel, Ok, true)
{
	QWidget	*dummy = new QWidget(this);
	setMainWidget(dummy);
	type_ = new QComboBox(dummy);
	from_ = new QLineEdit(dummy);
	to_ = new QLineEdit(dummy);
	type_->insertItem(i18n("Send"));
	type_->insertItem(i18n("Allow"));
	type_->insertItem(i18n("Deny"));
	type_->insertItem(i18n("Relay"));
	type_->insertItem(i18n("Poll"));

	QLabel	*l1 = new QLabel(i18n("Type:"), dummy);
	QLabel	*l2 = new QLabel(i18n("From:"), dummy);
	QLabel	*l3 = new QLabel(i18n("To:"), dummy);

	QGridLayout	*m1 = new QGridLayout(dummy, 3, 2, 0, 5);
	m1->addWidget(l1, 0, 0, Qt::AlignRight);
	m1->addWidget(l2, 1, 0, Qt::AlignRight);
	m1->addWidget(l3, 2, 0, Qt::AlignRight);
	m1->addWidget(type_, 0, 1);
	m1->addWidget(from_, 1, 1);
	m1->addWidget(to_, 2, 1);

	connect(type_, SIGNAL(activated(int)), SLOT(slotTypeChanged(int)));
	slotTypeChanged(type_->currentItem());

	setCaption(i18n("Browse Address"));
	resize(250, 100);
}

QString BrowseDialog::addressString()
{
	QString s;
	switch (type_->currentItem())
	{
		case 0:
			s.append("Send");
			break;
		case 1:
			s.append("Allow");
			break;
		case 2:
			s.append("Deny");
			break;
		case 3:
			s.append("Relay");
			break;
		case 4:
			s.append("Poll");
			break;
	}
	if (from_->isEnabled())
		s.append(" ").append(from_->text());
	if (to_->isEnabled())
		s.append(" ").append(to_->text());
	return s;
}

void BrowseDialog::setInfos(CupsdConf *conf)
{
	type_->setWhatsThis(conf->comments_.toolTip("browsetype"));
}

QString BrowseDialog::newAddress(QWidget *parent, CupsdConf *conf)
{
	BrowseDialog	dlg(parent);
	dlg.setInfos(conf);
	if (dlg.exec())
	{
		return dlg.addressString();
	}
	return QString::null;
}

QString BrowseDialog::editAddress(const QString& s, QWidget *parent, CupsdConf *conf)
{
	BrowseDialog	dlg(parent);
	dlg.setInfos(conf);
	QStringList	l = QStringList::split(QRegExp("\\s"), s, false);
	if (l.count() > 1)
	{
		if (l[0] == "Send") dlg.type_->setCurrentItem(0);
		else if (l[0] == "Allow") dlg.type_->setCurrentItem(1);
		else if (l[0] == "Deny") dlg.type_->setCurrentItem(2);
		else if (l[0] == "Relay") dlg.type_->setCurrentItem(3);
		else if (l[0] == "Poll") dlg.type_->setCurrentItem(4);
		dlg.slotTypeChanged(dlg.type_->currentItem());
		int	index(1);
		if (dlg.from_->isEnabled())
			dlg.from_->setText(l[index++]);
		if (dlg.to_->isEnabled())
			dlg.to_->setText(l[index]);
	}
	if (dlg.exec())
	{
		return dlg.addressString();
	}
	return QString::null;
}

void BrowseDialog::slotTypeChanged(int index)
{
	bool	useFrom(true), useTo(true);
	switch (index)
	{
		case 0: useFrom = false; break;
		case 1:
		case 4:
		case 2: useTo = false; break;
	}
	from_->setEnabled(useFrom);
	to_->setEnabled(useTo);
}

#include "browsedialog.moc"
