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

#include "cupslist.h"

#include <qlistview.h>
#include <qpushbutton.h>
#include <klocale.h>
#include <qlayout.h>
#include <qheader.h>

#include "qinputbox.h"

CupsListBox::CupsListBox(QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	init(1);
}

CupsListBox::CupsListBox(int columns, QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	init(columns);
}

CupsListBox::~CupsListBox()
{
}

void CupsListBox::init(int columns)
{
	list_ = 0;
	add_ = remove_ = 0;
	addmsg_.append(i18n("Enter new value:"));

	createView(columns);
	createButtons();
	createLayout();
}

void CupsListBox::createView(int columns)
{
	if (list_ == 0)
	{
		list_ = new QListView(this);
		list_->setFrameStyle(QFrame::Sunken|QFrame::WinPanel);
		list_->setLineWidth(1);
		list_->setAllColumnsShowFocus(true);
		columns_ = columns;
		for (int i=0;i<columns_;i++)
			list_->addColumn("column");
		if (columns_ <= 1)
		{
			list_->header()->hide();
			list_->setMaximumHeight(70);
		}
	}
}

void CupsListBox::createLayout()
{
/*	QGridLayout	*main_ = new QGridLayout(this, 2, 2, 0, 10);
	main_->addMultiCellWidget(list_, 0, 1, 0, 0);
	main_->addWidget(add_, 0, 1);
	main_->addWidget(remove_, 1, 1);*/
	QHBoxLayout	*main_ = new QHBoxLayout(this, 0, 10);
	QVBoxLayout	*btn_ = new QVBoxLayout(0, 0, 10);
	main_->addWidget(list_);
	main_->addLayout(btn_);
	btn_->addWidget(add_);
	btn_->addWidget(remove_);
	btn_->addStretch(1);
}

void CupsListBox::createButtons()
{
	if (add_ == 0 && remove_ == 0)
	{
		add_ = new QPushButton(i18n("Add..."), this);
		connect(add_, SIGNAL(clicked()), SLOT(addClicked()));
		remove_ = new QPushButton(i18n("Remove"), this);
		connect(remove_, SIGNAL(clicked()), SLOT(removeClicked()));
	}
}

QListViewItem* CupsListBox::findItemAtIndex(int i) const
{
	if (!list_ || i < 0 || i >= count()) return 0;

	QListViewItem	*item(list_->firstChild());
	int	j(0);
	while (item)
	{
		if (j == i) break;
		else
		{
			item = item->nextSibling();
			j++;
		}
	}
	return item;
}

void CupsListBox::setColumnText(int column, const QString& txt)
{
	if (column >= 0 && column < columns_ && list_)
	{
		list_->setColumnText(column, txt);
		if (!list_->header()->isVisible())
			list_->header()->show();
	}
}

QString CupsListBox::text(int index, int column) const
{
	QListViewItem	*item = findItemAtIndex(index);
	if (item)
	{
		return item->text(column);
	}
	return QString::null;
}

void CupsListBox::insertItem(const QString& str, int index)
{
	if (index == -1) index = count()-1;
	QListViewItem	*after = findItemAtIndex(index);
	if (after)
		new QListViewItem(list_, after, str);
	else
		new QListViewItem(list_, str);
}

void CupsListBox::insertItem(const QStringList& strs, int index)
{
	if (index == -1) index = count()-1;
	QListViewItem	*after = findItemAtIndex(index), *item;
	if (after)
		item = new QListViewItem(list_, after);
	else
		item = new QListViewItem(list_);
	for (uint i=0;i<strs.count();i++)
		item->setText(i, *(strs.at(i)));
}

void CupsListBox::addClicked()
{
	bool	ok;
	QStringList	strs = QInputBox::inputBox(this, i18n("Add"), addmsg_, columns_, &ok);
	if (ok)
	{
		insertItem(strs);
	}

}

void CupsListBox::removeClicked()
{
	if (list_->currentItem() != 0)
		delete list_->currentItem();
}

void CupsListBox::setAddMessage(const QString& msg, int index)
{
	if (index >= 0 && index < (int)addmsg_.count())
		*(addmsg_.at(index)) = msg;
	else
		addmsg_.append(msg);
}

int CupsListBox::count() const
{
	return (int)(list_->childCount());
}
#include "cupslist.moc"
