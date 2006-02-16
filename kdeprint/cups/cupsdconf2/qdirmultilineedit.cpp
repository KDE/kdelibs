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
#include <QListWidget>
#include <qpushbutton.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kiconloader.h>

QDirMultiLineEdit::QDirMultiLineEdit(QWidget *parent)
    : QWidget(parent)
{
	m_view = new QListWidget(this);
	connect(m_view, SIGNAL(itemSelectionChanged()), SLOT(slotSelected()));

	m_add = new QPushButton(this);
	m_add->setIcon(SmallIcon("folder_new"));
	connect(m_add, SIGNAL(clicked()), SLOT(slotAddClicked()));
	m_remove = new QPushButton(this);
	m_remove->setIcon(SmallIcon("editdelete"));
	connect(m_remove, SIGNAL(clicked()), SLOT(slotRemoveClicked()));
	m_remove->setEnabled(false);

	m_view->setFixedHeight(qMax(m_view->fontMetrics().lineSpacing()*3+m_view->lineWidth()*2, m_add->sizeHint().height()*2));

	QHBoxLayout	*l0 = new QHBoxLayout(this);
  l0->setMargin(0);
  l0->setSpacing(3);
	QVBoxLayout	*l1 = new QVBoxLayout(0);
	l0->addWidget(m_view);
	l0->addLayout(l1);
  l1->setMargin(0);
  l1->setSpacing(0);
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
	QStringList	l;

  for ( int i = 0; i < m_view->count(); ++i )
    l << m_view->item( i )->text();

	return l;
}

void QDirMultiLineEdit::addURL(const QString& url)
{
	new QListWidgetItem(url, m_view);
}

void QDirMultiLineEdit::slotAddClicked()
{
	QString	dirname = KFileDialog::getExistingDirectory(QString(), this);
	if (!dirname.isEmpty())
		addURL(dirname);
}

void QDirMultiLineEdit::slotRemoveClicked()
{
	QListWidgetItem	*item = m_view->currentItem();
	if (item)
	{
		delete item;
		slotSelected();
	}
}

void QDirMultiLineEdit::slotSelected()
{
	m_remove->setEnabled(!(m_view->selectedItems().isEmpty()));
}

#include "qdirmultilineedit.moc"
