#include "kmprinterview.h"
#include "kmprinter.h"
#include "kmiconview.h"
#include "kmlistview.h"
#include "kmtimer.h"

#include <qlayout.h>
#include <qpopupmenu.h>
#include <kaction.h>
#include <klocale.h>

KMPrinterView::KMPrinterView(QWidget *parent, const char *name)
: QWidget(parent,name), m_type(KMPrinterView::Icons)
{
	m_printers = 0;
	m_iconview = new KMIconView(this);
	m_listview = new KMListView(this);
	m_current = 0;

	connect(m_iconview,SIGNAL(rightButtonClicked(KMPrinter*,const QPoint&)),SIGNAL(rightButtonClicked(KMPrinter*,const QPoint&)));
	connect(m_listview,SIGNAL(rightButtonClicked(KMPrinter*,const QPoint&)),SIGNAL(rightButtonClicked(KMPrinter*,const QPoint&)));
	connect(m_iconview,SIGNAL(printerSelected(KMPrinter*)),SIGNAL(printerSelected(KMPrinter*)));
	connect(m_listview,SIGNAL(printerSelected(KMPrinter*)),SIGNAL(printerSelected(KMPrinter*)));
	connect(m_iconview,SIGNAL(printerSelected(KMPrinter*)),SLOT(slotPrinterSelected(KMPrinter*)));
	connect(m_listview,SIGNAL(printerSelected(KMPrinter*)),SLOT(slotPrinterSelected(KMPrinter*)));

	QVBoxLayout	*main_ = new QVBoxLayout(this, 0, 0);
	main_->addWidget(m_iconview);
	main_->addWidget(m_listview);

	setViewType(m_type);
}

KMPrinterView::~KMPrinterView()
{
}

void KMPrinterView::setPrinterList(QList<KMPrinter> *list)
{
	m_printers = list;
	if (m_type != KMPrinterView::Tree)
		m_iconview->setPrinterList(m_printers);
	else
		m_listview->setPrinterList(m_printers);
}

void KMPrinterView::setViewType(ViewType t)
{
	m_type = t;
	switch (m_type)
	{
		case KMPrinterView::Icons:
			m_iconview->setViewMode(KMIconView::Big);
			break;
		case KMPrinterView::List:
			m_iconview->setViewMode(KMIconView::Small);
			break;
		default:
			break;
	}
	KMPrinter	*oldcurrent = m_current;
	setPrinterList(m_printers);
	if (m_type == KMPrinterView::Tree)
	{
		m_listview->show();
		m_iconview->hide();
		m_listview->setPrinter(oldcurrent);
	}
	else
	{
		m_iconview->show();
		m_listview->hide();
		m_iconview->setPrinter(oldcurrent);
	}
}

void KMPrinterView::slotPrinterSelected(KMPrinter *p)
{
	m_current = p;
}
#include "kmprinterview.moc"
