#include "kmpropwidget.h"
#include "kmtimer.h"
#include "kmwizard.h"
#include "kmfactory.h"
#include "kmmanager.h"

KMPropWidget::KMPropWidget(QWidget *parent, const char *name)
: QWidget(parent,name)
{
	m_pixmap = "folder";
	m_title = m_header = "Title";
	m_printer = 0;

	connect(this,SIGNAL(enable(bool)),this,SIGNAL(enableChange(bool)));
}

KMPropWidget::~KMPropWidget()
{
}

void KMPropWidget::slotChange()
{
	KMTimer::blockTimer();
	bool	value = requestChange();
	KMTimer::releaseTimer(value);
}

void KMPropWidget::setPrinterBase(KMPrinter *p)
{
	m_printer = p;
	setPrinter(p);
}

void KMPropWidget::setPrinter(KMPrinter*)
{
}

void KMPropWidget::configureWizard(KMWizard*)
{
}

bool KMPropWidget::requestChange()
{
	if (m_printer)
	{
		KMWizard	dlg(this);
		configureWizard(&dlg);
		dlg.setPrinter(m_printer);
		if (dlg.exec())
			return KMFactory::self()->manager()->modifyPrinter(m_printer,dlg.printer());
	}
	return false;
}
#include "kmpropwidget.moc"
