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

#include "kmprinterview.h"
#include "kmprinter.h"
#include "kmiconview.h"
#include "kmlistview.h"
#include "kmtimer.h"
#include "kmmanager.h"

#include <qlayout.h>
#include <qpopupmenu.h>
#include <kaction.h>
#include <klocale.h>

KMPrinterView::KMPrinterView(QWidget *parent, const char *name)
: QWidgetStack(parent,name), m_type(KMPrinterView::Icons)
{
	m_iconview = new KMIconView(this);
	addWidget(m_iconview,0);
	m_listview = new KMListView(this);
	addWidget(m_listview,1);
	m_current = QString::null;
	m_listset = false;

	connect(m_iconview,SIGNAL(rightButtonClicked(const QString&,const QPoint&)),SIGNAL(rightButtonClicked(const QString&,const QPoint&)));
	connect(m_listview,SIGNAL(rightButtonClicked(const QString&,const QPoint&)),SIGNAL(rightButtonClicked(const QString&,const QPoint&)));
	connect(m_iconview,SIGNAL(printerSelected(const QString&)),SIGNAL(printerSelected(const QString&)));
	connect(m_listview,SIGNAL(printerSelected(const QString&)),SIGNAL(printerSelected(const QString&)));
	connect(m_iconview,SIGNAL(printerSelected(const QString&)),SLOT(slotPrinterSelected(const QString&)));
	connect(m_listview,SIGNAL(printerSelected(const QString&)),SLOT(slotPrinterSelected(const QString&)));

	setViewType(m_type);
	setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
}

KMPrinterView::~KMPrinterView()
{
}

void KMPrinterView::setPrinterList(QPtrList<KMPrinter> *list)
{
	if (m_type != KMPrinterView::Tree || list == 0)
		m_iconview->setPrinterList(list);
	if (m_type == KMPrinterView::Tree || list == 0)
		m_listview->setPrinterList(list);
	m_listset = ( list != 0 );
}

void KMPrinterView::setViewType(ViewType t)
{
	m_type = t;
	switch (m_type)
	{
		case KMPrinterView::Icons:
			m_iconview->setViewMode(KMIconView::Big);
			break;
		case KMPrinterView::List:
			m_iconview->setViewMode(KMIconView::Small);
			break;
		default:
			break;
	}
	QString	oldcurrent = m_current;
	if ( m_listset )
		setPrinterList(KMManager::self()->printerList(false));
	if (m_type == KMPrinterView::Tree)
	{
		raiseWidget(m_listview);
		m_listview->setPrinter(oldcurrent);
	}
	else
	{
		raiseWidget(m_iconview);
		m_iconview->setPrinter(oldcurrent);
	}
}

void KMPrinterView::slotPrinterSelected(const QString& p)
{
	m_current = p;
}

QSize KMPrinterView::minimumSizeHint() const
{
	return QWidgetStack::minimumSizeHint();
}

#include "kmprinterview.moc"
