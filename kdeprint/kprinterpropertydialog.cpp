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
#include <qtabwidget.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kguiitem.h>

KPrinterPropertyDialog::KPrinterPropertyDialog(KMPrinter *p, QWidget *parent, const char *name)
: KDialogBase(parent, name, true, QString::null, KDialogBase::Ok|KDialogBase::Cancel|KDialogBase::User1, KDialogBase::Ok, false, KGuiItem(i18n("&Save"), "filesave")),
  m_printer(p), m_driver(0), m_current(0)
{
	m_pages.setAutoDelete(false);

	// set a margin
	m_tw = new QTabWidget(this);
	m_tw->setMargin(10);
	connect(m_tw,SIGNAL(currentChanged(QWidget*)),SLOT(slotCurrentChanged(QWidget*)));
	setMainWidget(m_tw);

	if (m_printer)
		m_options = (m_printer->isEdited() ? m_printer->editedOptions() : m_printer->defaultOptions());
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
	m_tw->addTab(page,page->title());
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
			KMessageBox::error(this, msg.prepend("<qt>").append("</qt>"), i18n("Printer Configuration"));
			return false;
		}
	}
	return true;
}

void KPrinterPropertyDialog::setOptions(const QMap<QString,QString>& opts)
{
	// merge the 2 options sets
	for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
		m_options[it.key()] = it.data();
	// update all existing pages
	QPtrListIterator<KPrintDialogPage>	it(m_pages);
	for (; it.current(); ++it)
		it.current()->setOptions(m_options);
}

void KPrinterPropertyDialog::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	collectOptions(opts, incldef);
}

void KPrinterPropertyDialog::collectOptions(QMap<QString,QString>& opts, bool incldef)
{
	QPtrListIterator<KPrintDialogPage>	it(m_pages);
	for (;it.current();++it)
		it.current()->getOptions(opts,incldef);
}

void KPrinterPropertyDialog::slotOk()
{
	if (!synchronize())
		return;
	KDialogBase::slotOk();
}

void KPrinterPropertyDialog::slotUser1()
{
	if (m_printer && synchronize())
	{
		QMap<QString,QString>	opts;
		collectOptions(opts, false);
		m_printer->setDefaultOptions(opts);
		m_printer->setEditedOptions(QMap<QString,QString>());
		m_printer->setEdited(false);
		KMFactory::self()->virtualManager()->triggerSave();
	}
}

void KPrinterPropertyDialog::enableSaveButton(bool state)
{
	showButton(KDialogBase::User1, state);
}

void KPrinterPropertyDialog::setupPrinter(KMPrinter *pr, QWidget *parent)
{
	KPrinterPropertyDialog	dlg(pr,parent,"PropertyDialog");
	KMFactory::self()->uiManager()->setupPropertyDialog(&dlg);
	if (dlg.m_pages.count() == 0)
		KMessageBox::information(parent,i18n("No configurable options for that printer!"),i18n("Printer Configuration"));
	else if (dlg.exec())
	{
		QMap<QString,QString>	opts;
		dlg.collectOptions(opts, false);
		pr->setEditedOptions(opts);
		pr->setEdited(true);
	}
}
#include "kprinterpropertydialog.moc"
