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

#include <q3listview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>
#include <kmessagebox.h>

KMCupsUiManager::KMCupsUiManager(QObject *parent, const char *name, const QStringList & /*args*/)
: KMUiManager(parent)
{
}

KMCupsUiManager::~KMCupsUiManager()
{
}

void KMCupsUiManager::setupPropertyPages(KMPropertyPage *p)
{
    KMPropWidget *prop;
    prop = new KMPropMembers(p);
    prop->setObjectName( "Members" );
    p->addPropPage(prop);
    prop = new KMPropBackend( p );
    prop->setObjectName( "Backend");
    p->addPropPage(prop);
    prop = new KMPropDriver( p );
    prop->setObjectName("Driver");
    p->addPropPage(prop);
    prop = new KMPropBanners( p );
    prop->setObjectName( "Banners" );
    p->addPropPage(prop);
    prop = new KMPropQuota( p );
    prop->setObjectName("Quotas");
    p->addPropPage(prop);
    prop = new KMPropUsers(p);
    prop->setObjectName("Users");
    p->addPropPage(prop);
}

void KMCupsUiManager::setupWizard(KMWizard *wizard)
{
	QString whatsThisRemoteCUPSIPPBackend =
		i18n( "<qt><p>Print queue on remote CUPS server</p>"
			  "<p>Use this for a print queue installed on a remote "
			  "machine running a CUPS server. This allows to use "
			  "remote printers when CUPS browsing is turned off.</p></qt>"
			);

	QString whatsThisRemotePrinterIPPBackend =
		i18n( "<qt><p>Network IPP printer</p>"
			  "<p>Use this for a network-enabled printer using the "
			  "IPP protocol. Modern high-end printers can use this mode. "
			  "Use this mode instead of TCP if your printer can do both.</p></qt>"
			);

	QString whatsThisSerialFaxModemBackend =
		i18n( "<qt><p>Fax/Modem printer</p>"
			  "<p>Use this for a fax/modem printer. This requires the installation "
			  "of the <a href=\"http://vigna.dsi.unimi.it/fax4CUPS/\">fax4CUPS</a> backend. Documents sent on this printer will be faxed "
			  "to the given target fax number.</p></qt>"
			);

	QString whatsThisOtherPrintertypeBackend =
		i18n( "<qt><p>Other printer</p>"
			  "<p>Use this for any printer type. To use this option, you must know "
			  "the URI of the printer you want to install. Refer to the CUPS documentation "
			  "for more information about the printer URI. This option is mainly useful for "
			  "printer types using 3rd party backends not covered by the other possibilities.</p></qt>"
			);

	QString whatsThisClassOfPrinters =
		i18n( "<qt><p>Class of printers</p>"
			  "<p>Use this to create a class of printers. When sending a document to a class, "
			  "the document is actually sent to the first available (idle) printer in the class. "
			  "Refer to the CUPS documentation for more information about class of printers.</p></qt>"
			);

	KMWBackend	*backend = wizard->backendPage();
	if (!backend)
		return;
	backend->addBackend(KMWizard::Local,false);
	backend->addBackend(KMWizard::LPD,false);
	backend->addBackend(KMWizard::SMB,false,KMWizard::Password);
	backend->addBackend(KMWizard::TCP,false);
	backend->addBackend(KMWizard::IPP,i18n("Re&mote CUPS server (IPP/HTTP)"),false,whatsThisRemoteCUPSIPPBackend,KMWizard::Password);
	backend->addBackend(KMWizard::Custom+1,i18n("Network printer w/&IPP (IPP/HTTP)"),false,whatsThisRemotePrinterIPPBackend);
	backend->addBackend(KMWizard::Custom+2,i18n("S&erial Fax/Modem printer"),false,whatsThisSerialFaxModemBackend);
	backend->addBackend(KMWizard::Custom+5,i18n("Other &printer type"),false,whatsThisOtherPrintertypeBackend);
	backend->addBackend();
	backend->addBackend(KMWizard::Class,i18n("Cl&ass of printers"),false,whatsThisClassOfPrinters);

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
	else
		KMessageBox::error(wizard,
			"<qt><nobr>" +
			i18n("An error occurred while retrieving the list of available backends:") +
			"</nobr><br><br>" + req.statusMessage() + "</qt>");

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
        KPGeneralPage *gp = new KPGeneralPage(dlg->printer(),dlg->driver(),dlg );
        gp->setObjectName( "GeneralPage");
        dlg->addPage(gp );
	if (KMFactory::self()->settings()->application != KPrinter::Dialog)
	{
            KPrintDialogPage *page;
            page = new KPImagePage(dlg->driver(), dlg );
            page->setObjectName("ImagePage");
            dlg->addPage(page);
            page = new KPTextPage(dlg->driver(), dlg );
            page->setObjectName("TextPage");
            dlg->addPage(page);
            page = new KPHpgl2Page(dlg );
            page->setObjectName("Hpgl2Page");
            dlg->addPage(page);
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

void KMCupsUiManager::setupPrintDialogPages(Q3PtrList<KPrintDialogPage>* pages)
{
	pages->append(new KPSchedulePage());
	pages->append(new KPTagsPage());
}

void KMCupsUiManager::setupJobViewer(Q3ListView *lv)
{
	lv->addColumn(i18n("Priority"));
	lv->setColumnAlignment(lv->columns()-1, Qt::AlignRight|Qt::AlignVCenter);
	lv->addColumn(i18n("Billing Information"));
	lv->setColumnAlignment(lv->columns()-1, Qt::AlignRight|Qt::AlignVCenter);
}
