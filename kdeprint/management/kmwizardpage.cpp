#include "kmwizardpage.h"
#include "kmwizard.h"

KMWizardPage::KMWizardPage(QWidget *parent, const char *name)
: QWidget(parent,name)
{
	m_ID = KMWizard::Error;
	m_title = "KMWizardPage";
	m_nextpage = KMWizard::Error;
}

KMWizardPage::~KMWizardPage()
{
}

bool KMWizardPage::isValid(QString&)
{
	return true;
}

void KMWizardPage::initPrinter(KMPrinter*)
{
}

void KMWizardPage::updatePrinter(KMPrinter*)
{
}
#include "kmwizardpage.moc"
