/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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

#include "driverview.h"
#include "droptionview.h"
#include "driveritem.h"
#include "driver.h"

#include <qlistview.h>
#include <qheader.h>
#include <qlayout.h>

DrListView::DrListView(QWidget *parent, const char *name)
: KListView(parent,name)
{
	addColumn("");
	header()->hide();
	setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	setSorting(-1);
}

//****************************************************************************************************

DriverView::DriverView(QWidget *parent, const char *name)
: QWidget(parent,name)
{
	m_driver = 0;

	m_view = new DrListView(this);
	m_optview = new DrOptionView(this);

	QVBoxLayout	*main_ = new QVBoxLayout(this, 0, 10);
	main_->addWidget(m_view,1);
	main_->addWidget(m_optview,0);

	connect(m_view,SIGNAL(selectionChanged(QListViewItem*)),m_optview,SLOT(slotItemSelected(QListViewItem*)));
	connect(m_optview,SIGNAL(changed()),SLOT(slotChanged()));
}

DriverView::~DriverView()
{
}

void DriverView::setDriver(DrMain *driver)
{
	m_driver = driver;
	if (m_driver)
	{
		m_view->clear();
		m_driver->createTreeView(m_view);
		slotChanged();
	}
}

void DriverView::slotChanged()
{
	if (m_driver)
	{
		m_conflict = m_driver->checkConstraints();
		((DriverItem*)m_view->firstChild())->updateConflict();
	}
}

void DriverView::setOptions(const QMap<QString,QString>& opts)
{
	if (m_driver)
	{
		m_driver->setOptions(opts);
		static_cast<DriverItem*>( m_view->firstChild() )->updateTextRecursive();
		slotChanged();
		m_optview->slotItemSelected(m_view->currentItem());
	}
}

void DriverView::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	if (m_driver)
		m_driver->getOptions(opts,incldef);
}

void DriverView::setAllowFixed(bool on)
{
	m_optview->setAllowFixed(on);
}
#include "driverview.moc"
