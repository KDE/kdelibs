#include "kmpropertypage.h"
#include "kmpropwidget.h"
#include "kmpropcontainer.h"
#include "kmprinter.h"
#include "kmfactory.h"
#include "kmuimanager.h"

#include "kmpropgeneral.h"
#include "kmpropmembers.h"
#include "kmpropbackend.h"
#include "kmpropdriver.h"

#include <qvbox.h>
#include <kiconloader.h>

KMPropertyPage::KMPropertyPage(QWidget *parent, const char *name)
: CJanusWidget(parent,name)
{
	m_widgets.setAutoDelete(false);

	addPropPage(new KMPropGeneral(this, "General"));
	addPropPage(new KMPropMembers(this, "Members"));
	addPropPage(new KMPropBackend(this, "Backend"));
	addPropPage(new KMPropDriver(this, "Driver"));

	// add specific pages
	KMFactory::self()->uiManager()->addPropertyPages(this);
}

KMPropertyPage::~KMPropertyPage()
{
}

void KMPropertyPage::setPrinter(KMPrinter *p)
{
	QListIterator<KMPropWidget>	it(m_widgets);
	for (;it.current();++it)
		it.current()->setPrinterBase(p);
}

void KMPropertyPage::addPropPage(KMPropWidget *w)
{
	if (w)
	{
		m_widgets.append(w);
		KMPropContainer	*ctn = new KMPropContainer(this,"Container");
		ctn->setWidget(w);
		connect(ctn,SIGNAL(enable(bool)),SLOT(slotEnable(bool)));
		addPage(ctn,w->title(),w->header(),DesktopIcon(w->pixmap()));
	}
}

void KMPropertyPage::slotEnable(bool on)
{
	QWidget	*w = (QWidget*)(sender());
	if (on)
		enablePage(w);
	else
		disablePage(w);
}
#include "kmpropertypage.moc"
