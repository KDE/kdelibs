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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
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
#include "kpposterpage.h"

#include <klocale.h>
#include <kdebug.h>

KMUiManager::KMUiManager(QObject *parent)
: QObject(parent)
{
	m_printdialogflags = KMUiManager::PrintDialogAll;
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
	if (stdpages & KPrinter::CopiesPage) {
            KPCopiesPage *cp = new KPCopiesPage(dlg->printer(), 0 );
            cp->setObjectName( "CopiesPage");
            m_printdialogpages.prepend(cp);
        }
	if (stdpages & KPrinter::FilesPage) {
            KPFileSelectPage *fsp = new KPFileSelectPage( 0 );
            fsp->setObjectName( "FileSelectPage" );
            m_printdialogpages.prepend( fsp );
        }

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
		{  // special case
                    KPQtPage *qp = new KPQtPage( dlg );
                    qp->setObjectName( "QtPage" );
                    dlg->addPage( qp );
                    //dlg->enableSaveButton(false);
		}
		else
		{
			// add pages specific to print system
			setupPrinterPropertyDialog(dlg);
		}

		// retrieve the KPrinter object
		KPrinter	*prt(0);
		if (dlg->parent() && dlg->parent()->metaObject()->className() == "KPrintDialog")
			prt = static_cast<KPrintDialog*>(dlg->parent())->printer();

		// add margin page
		if ( ( prt && !prt->fullPage() && prt->applicationType() == KPrinter::Dialog )
                     || prt->applicationType() < 0 )
                {
                    KPMarginPage *mp = new KPMarginPage(prt, driver, dlg );
                    mp->setObjectName( "MarginPage");
                    dlg->addPage(mp);
                }

		// add driver page
		if (driver) {
                    KPDriverPage *dp = new KPDriverPage(dlg->printer(),driver,dlg );
                    dp->setObjectName("DriverPage");
                    dlg->addPage(dp);
                }

		dlg->setCaption(i18n("Configuration of %1").arg(dlg->printer()->name()));
		if ( KXmlCommandManager::self()->checkCommand( "poster", KXmlCommandManager::None, KXmlCommandManager::None ) ) {
                    KPPosterPage *pp = new KPPosterPage( dlg );
                    pp->setObjectName( "PosterPage" );
                    dlg->addPage( pp );
                }
                KPFilterPage *fp = new KPFilterPage( dlg );
                fp->setObjectName( "FilterPage" );
		dlg->addPage( fp );
		dlg->resize(100,100);
	}
}

void KMUiManager::setupPrinterPropertyDialog(KPrinterPropertyDialog *dlg)
{
	if (KMFactory::self()->settings()->application == KPrinter::Dialog
            || KMFactory::self()->settings()->application < 0 )
        {
            KPQtPage *qp = new KPQtPage( dlg );
            qp->setObjectName( "QtPage" );
            dlg->addPage(qp );
        }
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

void KMUiManager::setupPrintDialogPages(QList<KPrintDialogPage*>*)
{
}

void KMUiManager::setupJobViewer(QTreeWidget*)
{
}

#include "kmuimanager.moc"
