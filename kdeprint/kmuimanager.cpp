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

#include <config.h>

#include "kmuimanager.h"
#include "kprintdialog.h"
#include "kprintdialogpage.h"
#include "kpcopiespage.h"
#include "kprinter.h"
#include "kprinterpropertydialog.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kmprinter.h"
#include "kpdriverpage.h"
#include "kpmarginpage.h"
#include "kpqtpage.h"
#include "kpfilterpage.h"
#include "kpfileselectpage.h"
#include "kxmlcommand.h"

#include <klocale.h>
#include <kdebug.h>

KMUiManager::KMUiManager(QObject *parent, const char *name)
: QObject(parent,name)
{
	m_printdialogflags = KMUiManager::PrintDialogAll;
	m_printdialogpages.setAutoDelete(false);
}

KMUiManager::~KMUiManager()
{
}

void KMUiManager::setupPropertyPages(KMPropertyPage*)
{
}

void KMUiManager::setupWizard(KMWizard*)
{
}

void KMUiManager::setupConfigDialog(KMConfigDialog*)
{
}

int KMUiManager::copyFlags(KPrinter *pr, bool usePlugin)
{
	int	fl(0), pcap(pluginPageCap());
	if (KMFactory::self()->settings()->pageSelection == KPrinter::ApplicationSide)
	{
		if (pr)
		{
			if (pr->currentPage() > 0) fl |= Current;
			if (pr->minPage() > 0 && pr->maxPage() > 0)
				fl |= (Range|PageSet|Order);
		}
		//else fl = CopyAll;
		if (usePlugin)
			fl |= (pcap & (Collate|NoAutoCollate));
		else
			fl |= NoAutoCollate;
	}
	else if (usePlugin)
		// in this case, we want page capabilities with plugin, it means
		// for a regular real printer.
		fl = pageCap();
	else
		// int this case, we want page capabilities for non standard
		// printer, set auto-collate to false as copies will be handled
		// by Qt
		fl = systemPageCap() | NoAutoCollate;
	return fl;
}

int KMUiManager::dialogFlags()
{
	int	f = m_printdialogflags;
	int	appf = KMFactory::self()->settings()->application;
	if (appf != KPrinter::Dialog)
	{
		f &= ~(KMUiManager::Preview);
		if ( appf == KPrinter::StandAlonePersistent)
			f |= KMUiManager::Persistent;
	}
	return f;
}

void KMUiManager::setupPrintDialog(KPrintDialog *dlg)
{
	// dialog flags
	int	f = dialogFlags();
	dlg->setFlags(f);

	// add standard dialog pages
	int	stdpages = KMFactory::self()->settings()->standardDialogPages;
	if (stdpages & KPrinter::CopiesPage)
		m_printdialogpages.prepend(new KPCopiesPage(dlg->printer(), 0, "CopiesPage"));
	if (stdpages & KPrinter::FilesPage)
		m_printdialogpages.prepend(new KPFileSelectPage(0, "FileSelectPage"));

	// add plugins pages
	setupPrintDialogPages(&m_printdialogpages);

	dlg->setDialogPages(&m_printdialogpages);
}

void KMUiManager::setupPropertyDialog(KPrinterPropertyDialog *dlg)
{
	if (dlg->printer())
	{
		DrMain	*driver = KMManager::self()->loadDriver(dlg->printer(), false);
		dlg->setDriver(driver);

		if (dlg->printer()->isSpecial())
		{ // special case
			dlg->addPage(new KPQtPage(dlg,"QtPage"));
			//dlg->enableSaveButton(false);
		}
		else
		{
			// add pages specific to print system
			setupPrinterPropertyDialog(dlg);
		}

		// retrieve the KPrinter object
		KPrinter	*prt(0);
		if (dlg->parent() && dlg->parent()->isA("KPrintDialog"))
			prt = static_cast<KPrintDialog*>(dlg->parent())->printer();

#ifdef KDEPRINT_USE_MARGINS
		// add margin page
		if (prt && !prt->fullPage() && prt->applicationType() == KPrinter::Dialog)
			dlg->addPage(new KPMarginPage(prt, driver, dlg, "MarginPage"));
#endif

		// add driver page
		if (driver)
			dlg->addPage(new KPDriverPage(dlg->printer(),driver,dlg,"DriverPage"));

		dlg->setCaption(i18n("Configuration of %1").arg(dlg->printer()->name()));
		dlg->addPage(new KPFilterPage(dlg,"FilterPage"));
		dlg->resize(100,100);
	}
}

void KMUiManager::setupPrinterPropertyDialog(KPrinterPropertyDialog *dlg)
{
	if (KMFactory::self()->settings()->application == KPrinter::Dialog)
		dlg->addPage(new KPQtPage(dlg,"QtPage"));
}

int KMUiManager::pageCap()
{
	int	val = systemPageCap();
	val |= pluginPageCap();
	return val;
}

int KMUiManager::systemPageCap()
{
	int	val(0);
	if (KXmlCommandManager::self()->checkCommand("psselect"))
		val |= KMUiManager::PSSelect;
	return val;
}

int KMUiManager::pluginPageCap()
{
	return 0;
}

void KMUiManager::setupPrintDialogPages(QPtrList<KPrintDialogPage>*)
{
}

void KMUiManager::setupJobViewer(QListView*)
{
}
