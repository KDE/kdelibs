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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kmpropwidget.h"
#include "kmwizard.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kmtimer.h"

#include <kmessagebox.h>
#include <klocale.h>

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
	int	value = requestChange();
	if (value == -1)
	{
		KMessageBox::error(this, i18n("<qt>Unable to change printer properties. Error received from manager:<p>%1</p></qt>").arg(KMManager::self()->errorMsg()));
		KMManager::self()->setErrorMsg(QString::null);
	}
	KMTimer::self()->release((value == 1));
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

// return status:
//   -1 : error
//    0 : nothing to be done (canceled)
//    1 : success
int KMPropWidget::requestChange()
{
	if (m_printer)
	{
		KMWizard	dlg(this);
		configureWizard(&dlg);
		dlg.setPrinter(m_printer);
		if (dlg.exec())
			return (KMFactory::self()->manager()->modifyPrinter(m_printer,dlg.printer()) ? 1 : -1);
	}
	return 0;
}
#include "kmpropwidget.moc"
