#include "kmwinfopage.h"
#include "kmwizard.h"

#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>

KMWInfoPage::KMWInfoPage(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::Start;
	m_title = i18n("Introduction");
	m_nextpage = KMWizard::Backend;

	QLabel	*m_label = new QLabel(this);
	m_label->setText(i18n("<p>Welcome,</p><br>"
		"<p>This wizard will help to install a new printer on your computer. "
		"It will guides you through the various steps of the process of installing "
		"and configuring a printer for your printing system. At each step, you "
		"can always go back using the <b>Back</b> button.</p><br>"
		"<p>We hope you'll enjoy this tool !</p><br>"
		"<p align=right><i>The KDE printing team.</i></p>"));

	QVBoxLayout	*main_ = new QVBoxLayout(this, 0, 0);
	main_->addWidget(m_label);
}
