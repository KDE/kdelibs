#include "kmpropbanners.h"
#include "kmprinter.h"
#include "kmwizard.h"

#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>

KMPropBanners::KMPropBanners(QWidget *parent, const char *name)
: KMPropWidget(parent,name)
{
	m_startbanner = new QLabel(this);
	m_stopbanner = new QLabel(this);

	QLabel	*l1 = new QLabel(i18n("Starting banner:"), this);
	QLabel	*l2 = new QLabel(i18n("Ending banner:"), this);

	QGridLayout	*main_ = new QGridLayout(this, 3, 2, 10, 10);
	main_->setColStretch(1,1);
	main_->setRowStretch(2,1);
	main_->addWidget(l1,0,0);
	main_->addWidget(l2,1,0);
	main_->addWidget(m_startbanner,0,1);
	main_->addWidget(m_stopbanner,1,1);

	m_title = i18n("Banners");
	m_header = i18n("Banners settings");
	m_pixmap = "editcopy";
}

KMPropBanners::~KMPropBanners()
{
}

void KMPropBanners::setPrinter(KMPrinter *p)
{
	if (p && p->isPrinter() && p->isLocal())
	{
		QStringList	l = QStringList::split(',',p->option("kde-banners"),false);
		m_startbanner->setText((l.count() > 0 ? l[0] : QString::fromLatin1("none")));
		m_stopbanner->setText((l.count() > 1 ? l[1] : QString::fromLatin1("none")));
		emit enable(true);
	}
	else
	{
		emit enable(false);
		m_startbanner->setText("");
		m_stopbanner->setText("");
	}
}

void KMPropBanners::configureWizard(KMWizard *w)
{
	w->configure(KMWizard::Banners,KMWizard::Banners,true);
}
