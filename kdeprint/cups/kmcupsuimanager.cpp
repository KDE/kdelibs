#include "kmcupsuimanager.h"
#include "kmpropertypage.h"
#include "kmwizard.h"
#include "kmwbackend.h"

#include "kmpropbanners.h"
#include "kmwbanners.h"
#include "kmwipp.h"
#include "kmwippselect.h"

#include "kprinterpropertydialog.h"
#include "kpgeneralpage.h"

#include "ipprequest.h"
#include "cupsinfos.h"

#include <klocale.h>

KMCupsUiManager::KMCupsUiManager(QObject *parent, const char *name)
: KMUiManager(parent,name)
{
	m_printdialogflags |= KMUiManager::Options;
	m_copyflags = (KMUiManager::CopyAll & ~KMUiManager::Current);
}

KMCupsUiManager::~KMCupsUiManager()
{
}

void KMCupsUiManager::addPropertyPages(KMPropertyPage *p)
{
	p->addPropPage(new KMPropBanners(p, "Banners"));
}

void KMCupsUiManager::setupWizard(KMWizard *wizard)
{
	KMWBackend	*backend = wizard->backendPage();
	if (!backend)
		return;
	backend->addBackend(KMWizard::Local,i18n("Local printer (parallel, serial, USB)"),false);
	backend->addBackend(KMWizard::LPD,i18n("Remote LPD queue"),false);
	backend->addBackend(KMWizard::SMB,i18n("SMB shared printer (Windows)"),false,KMWizard::Password);
	backend->addBackend(KMWizard::TCP,i18n("Network printer (TCP)"),false);
	backend->addBackend(KMWizard::IPP,i18n("Network printer (IPP/HTTP)"),false,KMWizard::Password);
	backend->addBackend(KMWizard::File,i18n("File printer (print to file)"),false);
	backend->addBackend();
	backend->addBackend(KMWizard::Class,i18n("Class of printers"));

	IppRequest	req;
	QString		uri;

	req.setOperation(CUPS_GET_DEVICES);
	uri = QString::fromLatin1("ipp://%1:%2/printers/").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port());
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);

	if (req.doRequest("/"))
	{
		ipp_attribute_t	*attr = req.first();
		while (attr)
		{
			if (strcmp(attr->name,"device-uri") == 0)
			{
				if (strncmp(attr->values[0].string.text,"socket",6) == 0) backend->enableBackend(KMWizard::TCP,true);
				else if (strncmp(attr->values[0].string.text,"parallel",8) == 0) backend->enableBackend(KMWizard::Local,true);
				else if (strncmp(attr->values[0].string.text,"serial",6) == 0) backend->enableBackend(KMWizard::Local,true);
				else if (strncmp(attr->values[0].string.text,"smb",3) == 0) backend->enableBackend(KMWizard::SMB,true);
				else if (strncmp(attr->values[0].string.text,"lpd",3) == 0) backend->enableBackend(KMWizard::LPD,true);
				else if (strncmp(attr->values[0].string.text,"usb",3) == 0) backend->enableBackend(KMWizard::Local,true);
				else if (strncmp(attr->values[0].string.text,"file",4) == 0) backend->enableBackend(KMWizard::File,true);
				else if (strncmp(attr->values[0].string.text,"http",4) == 0) backend->enableBackend(KMWizard::IPP,true);
				else if (strncmp(attr->values[0].string.text,"ipp",3) == 0) backend->enableBackend(KMWizard::IPP,true);
			}
			attr = attr->next;
		}
	}

	// banners page
	wizard->addPage(new KMWBanners(wizard));
	wizard->setNextPage(KMWizard::DriverTest,KMWizard::Banners);
	wizard->addPage(new KMWIpp(wizard));
	wizard->addPage(new KMWIppSelect(wizard));
}

void KMCupsUiManager::setupPrinterPropertyDialog(KPrinterPropertyDialog *dlg)
{
	// add general page
	dlg->addPage(new KPGeneralPage(dlg->printer(),dlg->driver(),dlg,"GeneralPage"));
}
