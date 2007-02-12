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

#include "editlist.h"

#include <klistwidget.h>
#include <kpushbutton.h>
#include <qlayout.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kguiitem.h>

EditList::EditList(QWidget *parent)
	: QWidget(parent)
{
	list_ = new KListWidget(this);
	addbtn_ = new KPushButton(KGuiItem(i18n("Add..."), "filenew"), this);
	editbtn_ = new KPushButton(KGuiItem(i18n("Edit..."), "edit"), this);
	delbtn_ = new KPushButton(KGuiItem(i18n("Delete"), "editdelete"), this);
	defbtn_ = new KPushButton(KGuiItem(i18n("Default List"), "history"), this);

	QGridLayout *m1 = new QGridLayout(this);
  m1->setMargin(0);
  m1->setSpacing(0);
	m1->setColumnStretch(0, 1);
	m1->addWidget(list_, 0, 3, 0, 1);
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
	int index = list_->currentRow();
	if (index >= 0)
		emit edit(index);
}

void EditList::slotDelete()
{
	int	index = list_->currentRow();
	delete list_->takeItem(index);
	slotSelected((list_->count() > 0 ? list_->currentRow() : -1));
	emit deleted(index);
}

void EditList::slotSelected(int index)
{
	editbtn_->setEnabled(index >= 0);
	delbtn_->setEnabled(index >= 0);
}

QString EditList::text(int index)
{
	return list_->item(index)->text();
}

void EditList::setText(int index, const QString& s)
{
	if (list_->item(index)->text() != s)
	{
		QListWidgetItem	*it = list_->findItems(s, Qt::MatchExactly).value(0);
		if (!it) {
			list_->item(index)->setText(s);
        }
		else {
			delete list_->takeItem(index);
        }
	}
}

void EditList::clear()
{
	list_->clear();
	slotSelected(-1);
}

void EditList::insertItem(const QString& s)
{
	if (list_->findItems(s, Qt::MatchExactly).isEmpty())
		list_->addItem(s);
}

void EditList::insertItem(const QPixmap& icon, const QString& s)
{
	if (!list_->findItems(s, Qt::MatchExactly).isEmpty()) { 
	    QListWidgetItem* item = new QListWidgetItem();
        item->setIcon(icon);
        item->setText(s);
        list_->addItem(item);
    }
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
		l << list_->item(i)->text();
	return l;
}

#include "editlist.moc"
