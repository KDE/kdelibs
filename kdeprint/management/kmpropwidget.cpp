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

#include "kmpropwidget.h"
#include "kmwizard.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kmtimer.h"

KMPropWidget::KMPropWidget(QWidget *parent, const char *name)
: QWidget(parent,name)
{
	m_pixmap = "folder";
	m_title = m_header = "Title";
	m_printer = 0;
	m_canchange = ((KMFactory::self()->manager()->printerOperationMask() & KMManager::PrinterCreation) && KMFactory::self()->manager()->hasManagement());

	connect(this,SIGNAL(enable(bool)),this,SIGNAL(enableChange(bool)));
}

KMPropWidget::~KMPropWidget()
{
}

void KMPropWidget::slotChange()
{
	KMTimer::self()->hold();
	bool	value = requestChange();
	KMTimer::self()->release(value);
}

void KMPropWidget::setPrinterBase(KMPrinter *p)
{
	m_printer = p;
	setPrinter(p);
}

void KMPropWidget::setPrinter(KMPrinter*)
{
}

void KMPropWidget::configureWizard(KMWizard*)
{
}

bool KMPropWidget::requestChange()
{
	if (m_printer)
	{
		KMWizard	dlg(this);
		configureWizard(&dlg);
		dlg.setPrinter(m_printer);
		if (dlg.exec())
			return KMFactory::self()->manager()->modifyPrinter(m_printer,dlg.printer());
	}
	return false;
}
#include "kmpropwidget.moc"
