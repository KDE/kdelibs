#include "kmmainview.h"
#include "kmprinterview.h"
#include "kmpages.h"
#include "kmmanager.h"
#include "kmfactory.h"
#include "kmtimer.h"
#include "kmvirtualmanager.h"
#include "kmprinter.h"
#include "driver.h"
#include "kmdriverdialog.h"
#include "kmwizard.h"
#include "kmvirtualmanager.h"

#include <qtimer.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <klocale.h>
#include <ktoolbar.h>

KMMainView::KMMainView(QWidget *parent, const char *name)
: QWidget(parent, name)
{
	KMTimer::setMainView(this);
	m_current = 0;

	// create widgets
	m_printerview = new KMPrinterView(this,"PrinterView");
	m_printerpages = new KMPages(this,"PrinterPages");
	m_timer = new QTimer(this);
	m_manager = KMFactory::self()->manager();
	m_pop = new QPopupMenu(this);

	// layout
	m_layout = new QBoxLayout(this, QBoxLayout::TopToBottom, 0, 10);
	m_layout->addWidget(m_printerview);
	m_layout->addWidget(m_printerpages);

	// connections
	connect(m_timer,SIGNAL(timeout()),SLOT(slotTimer()));
	connect(m_printerview,SIGNAL(printerSelected(KMPrinter*)),SLOT(slotPrinterSelected(KMPrinter*)));
	connect(m_printerview,SIGNAL(rightButtonClicked(KMPrinter*,const QPoint&)),SLOT(slotRightButtonClicked(KMPrinter*,const QPoint&)));
	connect(m_pop,SIGNAL(aboutToShow()),SLOT(slotShowMenu()));
	connect(m_pop,SIGNAL(aboutToHide()),SLOT(slotHideMenu()));

	// actions
	m_actions = new KActionCollection(this);
	initActions();

	// first update
	slotTimer();
}

KMMainView::~KMMainView()
{
	KMTimer::setMainView(0);
	KMFactory::release();
}

void KMMainView::initActions()
{
	KSelectAction	*vact = new KSelectAction(i18n("View..."),0,m_actions,"view_change");
	vact->setItems(QStringList::split(',',i18n("Icons,List,Tree"),false));
	vact->setCurrentItem(0);
	connect(vact,SIGNAL(activated(int)),SLOT(slotChangeView(int)));

	new KAction(i18n("Enable"),"run",0,this,SLOT(slotEnable()),m_actions,"printer_enable");
	new KAction(i18n("Disable"),"stop",0,this,SLOT(slotDisable()),m_actions,"printer_disable");
	new KAction(i18n("Remove"),"edittrash",0,this,SLOT(slotRemove()),m_actions,"printer_remove");
	new KAction(i18n("Configure"),"configure",0,this,SLOT(slotConfigure()),m_actions,"printer_configure");
	new KAction(i18n("Add printer/class..."),"wizard",0,this,SLOT(slotAdd()),m_actions,"printer_add");
	new KAction(i18n("Set as local default"),"kdeprint_printer",0,this,SLOT(slotHardDefault()),m_actions,"printer_hard_default");
	new KAction(i18n("Set as user default"),"exec",0,this,SLOT(slotSoftDefault()),m_actions,"printer_soft_default");
	new KAction(i18n("Test printer"),"fileprint",0,this,SLOT(slotTest()),m_actions,"printer_test");

	KSelectAction	*dact = new KSelectAction(i18n("Orientation..."),0,m_actions,"orientation_change");
	dact->setItems(QStringList::split(',',i18n("Vertical,Horizontal"),false));
	dact->setCurrentItem(0);
	connect(dact,SIGNAL(activated(int)),SLOT(slotChangeDirection(int)));
}

void KMMainView::startTimer()
{
	m_timer->start(5000,true);
}

void KMMainView::stopTimer()
{
	m_timer->stop();
}

void KMMainView::slotTimer()
{
	QList<KMPrinter>	*printerlist = m_manager->printerList();
	m_printerview->setPrinterList(printerlist);
	startTimer();
}

void KMMainView::slotPrinterSelected(KMPrinter *p)
{
	m_current = p;
	if (p) KMFactory::self()->manager()->completePrinter(p);
	m_printerpages->setPrinter(p);
}

void KMMainView::slotShowMenu()
{
	KMTimer::blockTimer();
}

void KMMainView::slotHideMenu()
{
	KMTimer::releaseTimer(false);
}

void KMMainView::slotChangeView(int ID)
{
	if (ID >= KMPrinterView::Icons && ID <= KMPrinterView::Tree)
		m_printerview->setViewType((KMPrinterView::ViewType)ID);
}

void KMMainView::slotRightButtonClicked(KMPrinter *printer, const QPoint& p)
{
	// construct popup menu
	m_pop->clear();
	if (printer)
	{
		m_current = printer;
		if (printer->state() == KMPrinter::Stopped)
			m_actions->action("printer_enable")->plug(m_pop);
		else
			m_actions->action("printer_disable")->plug(m_pop);
		m_pop->insertSeparator();
		if (!printer->isSoftDefault()) m_actions->action("printer_soft_default")->plug(m_pop);
		if (printer->isLocal())
		{
			if (!printer->isHardDefault()) m_actions->action("printer_hard_default")->plug(m_pop);
			m_actions->action("printer_remove")->plug(m_pop);
			m_pop->insertSeparator();
			if (!printer->isClass(true))
			{
				m_actions->action("printer_configure")->plug(m_pop);
				m_actions->action("printer_test")->plug(m_pop);
				m_pop->insertSeparator();
			}
		}
		else if (!printer->isClass(true))
		{
			m_actions->action("printer_test")->plug(m_pop);
			m_pop->insertSeparator();
		}
	}
	else
	{
		m_actions->action("printer_add")->plug(m_pop);
		m_pop->insertSeparator();
	}
	m_actions->action("view_change")->plug(m_pop);
	m_actions->action("orientation_change")->plug(m_pop);

	// pop the menu
	m_pop->popup(p);
}

void KMMainView::slotEnable()
{
	if (m_current)
	{
		KMTimer::blockTimer();
		bool	result = m_manager->enablePrinter(m_current);
		if (!result)
			showErrorMsg(i18n("Unable to enable printer <b>%1</b>.").arg(m_current->printerName()));
		KMTimer::releaseTimer(result);
	}
}

void KMMainView::slotDisable()
{
	if (m_current)
	{
		KMTimer::blockTimer();
		bool	result = m_manager->disablePrinter(m_current);
		if (!result)
			showErrorMsg(i18n("Unable to disable printer <b>%1</b>.").arg(m_current->printerName()));
		KMTimer::releaseTimer(result);
	}
}

void KMMainView::slotRemove()
{
	if (m_current)
	{
		KMTimer::blockTimer();
		bool	result(false);
		if (KMessageBox::warningYesNo(this,i18n("<nobr>Do you really want to remove <b>%1</b> ?</nobr>").arg(m_current->printerName())) == KMessageBox::Yes)
			if (!(result=m_manager->removePrinter(m_current)))
				showErrorMsg(i18n("Unable to remove printer <b>%1</b>.").arg(m_current->printerName()));
		KMTimer::releaseTimer(result);
	}
}

void KMMainView::slotConfigure()
{
	if (m_current)
	{
		KMTimer::blockTimer();
		DrMain	*driver = m_manager->loadPrinterDriver(m_current, true);
		if (driver)
		{
			KMDriverDialog	dlg(this);
			dlg.setCaption(i18n("Configure %1").arg(m_current->printerName()));
			dlg.setDriver(driver);
			if (dlg.exec())
				if (!m_manager->savePrinterDriver(m_current,driver))
					showErrorMsg(i18n("Unable to modify settings of printer <b>%1</b>.").arg(m_current->printerName()));
			delete driver;
		}
		KMTimer::releaseTimer(false);
	}
}

void KMMainView::slotAdd()
{
	KMTimer::blockTimer();
	KMWizard	dlg(this);
	bool		flag(false);
	if (dlg.exec())
	{
		flag = true;
		if (!m_manager->createPrinter(dlg.printer()))
			showErrorMsg(i18n("Unable to create printer."));
	}
	KMTimer::releaseTimer(flag);
}

void KMMainView::slotHardDefault()
{
	if (m_current)
	{
		KMTimer::blockTimer();
		bool	result = m_manager->setDefaultPrinter(m_current);
		if (!result)
			showErrorMsg(i18n("Unable to define printer <b>%1</b> as default.").arg(m_current->printerName()));
		KMTimer::releaseTimer(result);
	}
}

void KMMainView::slotSoftDefault()
{
	if (m_current)
	{
		KMTimer::blockTimer();
		KMFactory::self()->virtualManager()->setAsDefault(m_current,QString::null);
		KMTimer::releaseTimer(true);
	}
}

void KMMainView::setOrientation(int o)
{
	int 	ID = (o == Qt::Horizontal ? 1 : 0);
	((KSelectAction*)m_actions->action("orientation_change"))->setCurrentItem(ID);
	slotChangeDirection(ID);
}

void KMMainView::slotChangeDirection(int d)
{
	m_layout->setDirection((d == 1 ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom));
}

void KMMainView::slotTest()
{
	if (m_current)
	{
		KMTimer::blockTimer();
		if (KMFactory::self()->manager()->testPrinter(m_current))
			KMessageBox::information(this,i18n("<nobr>Test page successfully send to printer <b>%1</b>.</nobr>").arg(m_current->printerName()));
		else
			showErrorMsg(i18n("Unable to test printer <b>%1</b>.").arg(m_current->printerName()));
		KMTimer::releaseTimer(true);
	}
}

void KMMainView::showErrorMsg(const QString& msg, bool usemgr)
{
	QString	s("<nobr>"+msg+"</nobr>");
	if (usemgr)
	{
		s.append("<br>");
		s += i18n("Error message received from manager:<br><br>%1").arg(m_manager->errorMsg());
	}
	KMessageBox::error(this,s);
}
#include "kmmainview.moc"
