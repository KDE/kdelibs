/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <kdeprint@swing.be>
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

#include "qdirmultilineedit.h"

#include <qlayout.h>
#include <q3header.h>
#include <qpushbutton.h>
#include <klistview.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kiconloader.h>

QDirMultiLineEdit::QDirMultiLineEdit(QWidget *parent)
    : QWidget(parent)
{
	m_view = new KListView(this);
	m_view->header()->hide();
	m_view->addColumn("");
	m_view->setFullWidth(true);
	connect(m_view, SIGNAL(selectionChanged(Q3ListViewItem*)), SLOT(slotSelected(Q3ListViewItem*)));

	m_add = new QPushButton(this);
	m_add->setPixmap(SmallIcon("folder_new"));
	connect(m_add, SIGNAL(clicked()), SLOT(slotAddClicked()));
	m_remove = new QPushButton(this);
	m_remove->setPixmap(SmallIcon("editdelete"));
	connect(m_remove, SIGNAL(clicked()), SLOT(slotRemoveClicked()));
	m_remove->setEnabled(false);

	m_view->setFixedHeight(QMAX(m_view->fontMetrics().lineSpacing()*3+m_view->lineWidth()*2, m_add->sizeHint().height()*2));

	QHBoxLayout	*l0 = new QHBoxLayout(this, 0, 3);
	QVBoxLayout	*l1 = new QVBoxLayout(0, 0, 0);
	l0->addWidget(m_view);
	l0->addLayout(l1);
	l1->addWidget(m_add);
	l1->addWidget(m_remove);
	l1->addStretch(1);
}

QDirMultiLineEdit::~QDirMultiLineEdit()
{
}

void QDirMultiLineEdit::setURLs(const QStringList& urls)
{
	m_view->clear();
	for (QStringList::ConstIterator it=urls.begin(); it!=urls.end(); ++it)
		addURL(*it);
}

QStringList QDirMultiLineEdit::urls()
{
	Q3ListViewItem	*item = m_view->firstChild();
	QStringList	l;
	while (item)
	{
		l << item->text(0);
		item = item->nextSibling();
	}
	return l;
}

void QDirMultiLineEdit::addURL(const QString& url)
{
	Q3ListViewItem	*item = new Q3ListViewItem(m_view, url);
	item->setRenameEnabled(0, true);
}

void QDirMultiLineEdit::slotAddClicked()
{
	QString	dirname = KFileDialog::getExistingDirectory(QString(), this);
	if (!dirname.isEmpty())
		addURL(dirname);
}

void QDirMultiLineEdit::slotRemoveClicked()
{
	Q3ListViewItem	*item = m_view->currentItem();
	if (item)
	{
		delete item;
		slotSelected(m_view->currentItem());
	}
}

void QDirMultiLineEdit::slotSelected(Q3ListViewItem *item)
{
	m_remove->setEnabled((item != NULL));
}

#include "qdirmultilineedit.moc"
