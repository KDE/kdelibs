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

#include "kmcupsuimanager.h"
#include "kmpropertypage.h"
#include "kmwizard.h"
#include "kmconfigdialog.h"
#include "kmwbackend.h"
#include "kmfactory.h"
#include "kprinter.h"

#include "kmpropbanners.h"
#include "kmpropmembers.h"
#include "kmpropbackend.h"
#include "kmpropdriver.h"
#include "kmwbanners.h"
#include "kmwipp.h"
#include "kmwippselect.h"
#include "kmwippprinter.h"
#include "kmconfigcups.h"
#include "kmconfigcupsdir.h"
#include "kmwfax.h"
#include "kmwother.h"
#include "kmwquota.h"
#include "kmpropquota.h"
#include "kmwusers.h"
#include "kmpropusers.h"
#include "kpschedulepage.h"
#include "kptagspage.h"

#include "kprinterpropertydialog.h"
#include "kpgeneralpage.h"
#include "kpimagepage.h"
#include "kptextpage.h"
#include "kphpgl2page.h"

#include "ipprequest.h"
#include "cupsinfos.h"

#include <qlistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>

KMCupsUiManager::KMCupsUiManager(QObject *parent, const char *name)
: KMUiManager(parent,name)
{
}

KMCupsUiManager::~KMCupsUiManager()
{
}

void KMCupsUiManager::setupPropertyPages(KMPropertyPage *p)
{
	p->addPropPage(new KMPropMembers(p, "Members"));
	p->addPropPage(new KMPropBackend(p, "Backend"));
	p->addPropPage(new KMPropDriver(p, "Driver"));
	p->addPropPage(new KMPropBanners(p, "Banners"));
	p->addPropPage(new KMPropQuota(p, "Quotas"));
	p->addPropPage(new KMPropUsers(p, "Users"));
}

void KMCupsUiManager::setupWizard(KMWizard *wizard)
{
	KMWBackend	*backend = wizard->backendPage();
	if (!backend)
		return;
	backend->addBackend(KMWizard::Local,i18n("&Local printer (parallel, serial, USB)"),false);
	backend->addBackend(KMWizard::LPD,i18n("&Remote LPD queue"),false);
	backend->addBackend(KMWizard::SMB,i18n("&SMB shared printer (Windows)"),false,KMWizard::Password);
	backend->addBackend(KMWizard::TCP,i18n("Ne&twork printer (TCP)"),false);
	backend->addBackend(KMWizard::IPP,i18n("Re&mote CUPS server (IPP/HTTP)"),false,KMWizard::Password);
	backend->addBackend(KMWizard::Custom+1,i18n("Network printer w/&IPP (IPP/HTTP)"),false);
	backend->addBackend(KMWizard::Custom+2,i18n("S&erial Fax/Modem printer"),false);
	backend->addBackend(KMWizard::Custom+5,i18n("Other &printer type"),false);
	backend->addBackend();
	backend->addBackend(KMWizard::Class,i18n("Cl&ass of printers"),false);

	IppRequest	req;
	QString		uri;

	req.setOperation(CUPS_GET_DEVICES);
	uri = QString::fromLocal8Bit("ipp://%1:%2/printers/").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port());
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);

	if (req.doRequest("/"))
	{
		ipp_attribute_t	*attr = req.first();
		while (attr)
		{
			if (attr->name && strcmp(attr->name,"device-uri") == 0)
			{
				if (strncmp(attr->values[0].string.text,"socket",6) == 0) backend->enableBackend(KMWizard::TCP,true);
				else if (strncmp(attr->values[0].string.text,"parallel",8) == 0) backend->enableBackend(KMWizard::Local,true);
				else if (strncmp(attr->values[0].string.text,"serial",6) == 0) backend->enableBackend(KMWizard::Local,true);
				else if (strncmp(attr->values[0].string.text,"smb",3) == 0) backend->enableBackend(KMWizard::SMB,true);
				else if (strncmp(attr->values[0].string.text,"lpd",3) == 0) backend->enableBackend(KMWizard::LPD,true);
				else if (strncmp(attr->values[0].string.text,"usb",3) == 0) backend->enableBackend(KMWizard::Local,true);
				else if (strncmp(attr->values[0].string.text,"http",4) == 0 || strncmp(attr->values[0].string.text,"ipp",3) == 0)
				{
					backend->enableBackend(KMWizard::IPP,true);
					backend->enableBackend(KMWizard::Custom+1,true);
				}
				else if (strncmp(attr->values[0].string.text,"fax",3) == 0) backend->enableBackend(KMWizard::Custom+2,true);
			}
			attr = attr->next;
		}
		backend->enableBackend(KMWizard::Class, true);
		backend->enableBackend(KMWizard::Custom+5, true);
	}

	// banners page
	wizard->addPage(new KMWBanners(wizard));
	wizard->setNextPage(KMWizard::DriverTest,KMWizard::Banners);
	wizard->addPage(new KMWIpp(wizard));
	wizard->addPage(new KMWIppSelect(wizard));
	wizard->addPage(new KMWIppPrinter(wizard));
	wizard->addPage(new KMWFax(wizard));
	wizard->addPage(new KMWQuota(wizard));
	wizard->addPage(new KMWUsers(wizard));
	wizard->addPage(new KMWOther(wizard));
}

void KMCupsUiManager::setupPrinterPropertyDialog(KPrinterPropertyDialog *dlg)
{
	// add general page
	dlg->addPage(new KPGeneralPage(dlg->printer(),dlg->driver(),dlg,"GeneralPage"));
	if (KMFactory::self()->settings()->application != KPrinter::Dialog)
	{
		dlg->addPage(new KPImagePage(dlg->driver(), dlg, "ImagePage"));
		dlg->addPage(new KPTextPage(dlg->driver(), dlg, "TextPage"));
		dlg->addPage(new KPHpgl2Page(dlg, "Hpgl2Page"));
	}
}

void KMCupsUiManager::setupConfigDialog(KMConfigDialog *dlg)
{
	dlg->addConfigPage(new KMConfigCups(dlg));
	dlg->addConfigPage(new KMConfigCupsDir(dlg));
}

int KMCupsUiManager::pluginPageCap()
{
	return (KMUiManager::CopyAll & ~KMUiManager::Current);
}

void KMCupsUiManager::setupPrintDialogPages(QPtrList<KPrintDialogPage>* pages)
{
	pages->append(new KPSchedulePage());
	pages->append(new KPTagsPage());
}

void KMCupsUiManager::setupJobViewer(QListView *lv)
{
	lv->addColumn(i18n("Priority"));
	lv->setColumnAlignment(lv->columns()-1, Qt::AlignRight|Qt::AlignVCenter);
	lv->addColumn(i18n("Billing Information"));
	lv->setColumnAlignment(lv->columns()-1, Qt::AlignRight|Qt::AlignVCenter);
}
