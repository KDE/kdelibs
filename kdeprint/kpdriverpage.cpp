#include "kpdriverpage.h"
#include "driverview.h"
#include "driver.h"

#include <qlayout.h>
#include <klocale.h>

KPDriverPage::KPDriverPage(KMPrinter *p, DrMain *d, QWidget *parent, const char *name)
: KPrintDialogPage(p,d,parent,name)
{
	setTitle(i18n("Advanced"));

	m_view = new DriverView(this);
	m_view->setAllowFixed(false);
	if (driver()) m_view->setDriver(driver());

	QVBoxLayout	*lay1 = new QVBoxLayout(this, 10, 0);
	lay1->addWidget(m_view);
}

KPDriverPage::~KPDriverPage()
{
}

bool KPDriverPage::isValid(QString& msg)
{
	if (m_view->hasConflict())
	{
		msg = i18n("<nobr>Some options selected are in conflict. You must resolve those conflicts<br>"
 			   "before continuing. See <b>Advanced</b> tab for detailed informations.</nobr>");
		return false;
	}
	return true;
}

void KPDriverPage::setOptions(const QMap<QString,QString>& opts)
{
	m_view->setOptions(opts);
}

void KPDriverPage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	m_view->getOptions(opts,incldef);
}
