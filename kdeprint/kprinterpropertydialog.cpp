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

#include "kprinterpropertydialog.h"
#include "kprintdialogpage.h"
#include "kmfactory.h"
#include "kmuimanager.h"
#include "kmvirtualmanager.h"
#include "kmprinter.h"
#include "driver.h"

#include <kmessagebox.h>
#include <klocale.h>

KPrinterPropertyDialog::KPrinterPropertyDialog(KMPrinter *p, QWidget *parent, const char *name)
: QTabDialog(parent,name,true), m_printer(p), m_driver(0), m_current(0)
{
	m_pages.setAutoDelete(false);
	setOkButton(i18n("OK"));
	setCancelButton(i18n("Cancel"));
	setDefaultButton(i18n("Save"));

	if (m_printer)
		m_options = (m_printer->isEdited() ? m_printer->editedOptions() : m_printer->defaultOptions());

	connect(this,SIGNAL(currentChanged(QWidget*)),SLOT(slotCurrentChanged(QWidget*)));
	connect(this,SIGNAL(defaultButtonPressed()),SLOT(slotSaveClicked()));
}

KPrinterPropertyDialog::~KPrinterPropertyDialog()
{
	delete m_driver;
}

void KPrinterPropertyDialog::slotCurrentChanged(QWidget *w)
{
	if (m_current) m_current->getOptions(m_options,true);
	m_current = (KPrintDialogPage*)w;
	if (m_current) m_current->setOptions(m_options);
}

void KPrinterPropertyDialog::addPage(KPrintDialogPage *page)
{
	addTab(page,page->title());
	m_pages.append(page);
}

bool KPrinterPropertyDialog::synchronize()
{
	if (m_current) m_current->getOptions(m_options,true);
	QString	msg;
	QPtrListIterator<KPrintDialogPage>	it(m_pages);
	for (;it.current();++it)
	{
		it.current()->setOptions(m_options);
		if (!it.current()->isValid(msg))
		{
			KMessageBox::error(this,msg,i18n("Printer configuration"));
			return false;
		}
	}
	return true;
}

void KPrinterPropertyDialog::collectOptions(QMap<QString,QString>& opts)
{
	QPtrListIterator<KPrintDialogPage>	it(m_pages);
	for (;it.current();++it)
		it.current()->getOptions(opts,false);
}

void KPrinterPropertyDialog::done(int result)
{
	if (result == Accepted && !synchronize())
		return;
	QTabDialog::done(result);
}

void KPrinterPropertyDialog::slotSaveClicked()
{
	if (m_printer && synchronize())
	{
		QMap<QString,QString>	opts;
		collectOptions(opts);
		m_printer->setDefaultOptions(opts);
		m_printer->setEditedOptions(QMap<QString,QString>());
		m_printer->setEdited(false);
		KMFactory::self()->virtualManager()->triggerSave();
	}
}

void KPrinterPropertyDialog::setupPrinter(KMPrinter *pr, QWidget *parent)
{
	KPrinterPropertyDialog	dlg(pr,parent,"PropertyDialog");
	KMFactory::self()->uiManager()->setupPropertyDialog(&dlg);
	if (dlg.m_pages.count() == 0)
		KMessageBox::information(parent,i18n("No configurable options for that printer !"),i18n("Printer configuration"));
	else if (dlg.exec())
	{
		QMap<QString,QString>	opts;
		dlg.collectOptions(opts);
		pr->setEditedOptions(opts);
		pr->setEdited(true);
	}
}
#include "kprinterpropertydialog.moc"
