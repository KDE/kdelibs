#include "kmpages.h"
#include "kminfopage.h"
#include "kmjobviewer.h"
#include "kmpropertypage.h"
#include "kminstancepage.h"

#include <klocale.h>
#include <kiconloader.h>

KMPages::KMPages(QWidget *parent, const char *name)
: QTabWidget(parent,name)
{
	m_pages.setAutoDelete(false);
	initialize();
}

KMPages::~KMPages()
{
}

void KMPages::setPrinter(KMPrinter *p)
{
	QListIterator<KMPrinterPage>	it(m_pages);
	for (;it.current();++it)
		it.current()->setPrinter(p);
}

void KMPages::initialize()
{
	setMargin(10);

	// Info page
	KMInfoPage	*infopage = new KMInfoPage(this, "InfoPage");
	addTab(infopage, SmallIcon("help"), i18n("Informations"));
	m_pages.append(infopage);

	// Job page
	KMJobViewer	*jobviewer = new KMJobViewer(this, "JobViewer");
	addTab(jobviewer, SmallIcon("folder"), i18n("Jobs"));
	m_pages.append(jobviewer);

	// Property page
	KMPropertyPage	*proppage = new KMPropertyPage(this, "Property");
	addTab(proppage, SmallIcon("configure"), i18n("Properties"));
	m_pages.append(proppage);

	// Instance page
	KMInstancePage	*instpage = new KMInstancePage(this, "Instance");
	addTab(instpage, SmallIcon("fileprint"), i18n("Instances"));
	m_pages.append(instpage);

	// initialize pages
	setPrinter(0);
}
