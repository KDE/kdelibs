#include "kprintdialogpage.h"

KPrintDialogPage::KPrintDialogPage(QWidget *parent, const char *name)
: QWidget(parent,name), m_printer(0), m_driver(0), m_ID(0)
{
}

KPrintDialogPage::KPrintDialogPage(KMPrinter *pr, DrMain *dr, QWidget *parent, const char *name)
: QWidget(parent,name), m_printer(pr), m_driver(dr), m_ID(0)
{
}

KPrintDialogPage::~KPrintDialogPage()
{
}

void KPrintDialogPage::setOptions(const QMap<QString,QString>&)
{
}

void KPrintDialogPage::getOptions(QMap<QString,QString>&, bool)
{
}

bool KPrintDialogPage::isValid(QString&)
{
	return true;
}
#include "kprintdialogpage.moc"
