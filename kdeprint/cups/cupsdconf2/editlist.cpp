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

#include "editlist.h"

#include <klistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <klocale.h>
#include <kiconloader.h>

EditList::EditList(QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	list_ = new KListBox(this);
	addbtn_ = new QPushButton(i18n("Add..."), this);
	editbtn_ = new QPushButton(i18n("Edit..."), this);
	delbtn_ = new QPushButton(i18n("Delete"), this);
	defbtn_ = new QPushButton(i18n("Default List"), this);

	QGridLayout *m1 = new QGridLayout(this, 4, 2, 0, 0);
	m1->setColStretch(0, 1);
	m1->addMultiCellWidget(list_, 0, 3, 0, 1);
	m1->addWidget(addbtn_, 0, 1);
	m1->addWidget(editbtn_, 1, 1);
	m1->addWidget(delbtn_, 2, 1);
	m1->addWidget(defbtn_, 3, 1);

	connect(addbtn_, SIGNAL(clicked()), SIGNAL(add()));
	connect(editbtn_, SIGNAL(clicked()), SLOT(slotEdit()));
	connect(delbtn_, SIGNAL(clicked()), SLOT(slotDelete()));
	connect(defbtn_, SIGNAL(clicked()), SIGNAL(defaultList()));
	connect(list_, SIGNAL(highlighted(int)), SLOT(slotSelected(int)));
	slotSelected(-1);
}

void EditList::slotEdit()
{
	int index = list_->currentItem();
	if (index >= 0)
		emit edit(index);
}

void EditList::slotDelete()
{
	int	index = list_->currentItem();
	list_->removeItem(index);
	slotSelected((list_->count() > 0 ? list_->currentItem() : -1));
	emit deleted(index);
}

void EditList::slotSelected(int index)
{
	editbtn_->setEnabled(index >= 0);
	delbtn_->setEnabled(index >= 0);
}

QString EditList::text(int index)
{
	return list_->text(index);
}

void EditList::setText(int index, const QString& s)
{
	if (list_->text(index) != s)
	{
		QListBoxItem	*it = list_->findItem(s, Qt::ExactMatch);
		if (!it)
			list_->changeItem(s, index);
		else
			list_->removeItem(index);
	}
}

void EditList::clear()
{
	list_->clear();
	slotSelected(-1);
}

void EditList::insertItem(const QString& s)
{
	if (!list_->findItem(s, Qt::ExactMatch))
		list_->insertItem(s);
}

void EditList::insertItem(const QPixmap& icon, const QString& s)
{
	if (!list_->findItem(s, Qt::ExactMatch))
		list_->insertItem(icon, s);
}

void EditList::insertItems(const QStringList& l)
{
	for (QStringList::ConstIterator it=l.begin(); it!=l.end(); ++it)
		insertItem(*it);
}

QStringList EditList::items()
{
	QStringList l;
	for (int i=0; i<list_->count(); i++)
		l << list_->text(i);
	return l;
}

#include "editlist.moc"
