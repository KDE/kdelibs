/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

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
#include "kmconfigdialog.h"
#include "kmspecialprinterdlg.h"
#include "plugincombobox.h"

#include <qtimer.h>
#include <qsplitter.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <klocale.h>
#include <kconfig.h>
#include <ktoolbar.h>

#define	m_manager	KMFactory::self()->manager()

extern "C"
{
	int add_printer_wizard(QWidget *parent)
	{
		KMWizard	dlg(parent);
		int		flag(0);
		if (dlg.exec())
		{
			flag = 1;
			// check if the printer already exists, and ask confirmation if needed.
			if (KMFactory::self()->manager()->findPrinter(dlg.printer()->name()) != 0)
				if (KMessageBox::warningYesNo(parent,i18n("<nobr>The printer <b>%1</b> already exists. Continuing will<br>overwrite existing printer. Do you want to continue ?</nobr>").arg(dlg.printer()->name())) == KMessageBox::No)
					flag = 0;
			// try to add printer only if flag is true.
			if (flag && !KMFactory::self()->manager()->createPrinter(dlg.printer()))
				flag = -1;
		}
		return flag;
	}
};

KMMainView::KMMainView(QWidget *parent, const char *name)
: QWidget(parent, name)
{
	KMTimer::setMainView(this);
	m_current = 0;

	// create widgets
	m_splitter = new QSplitter(Qt::Vertical,this, "Splitter");
	m_printerview = new KMPrinterView(m_splitter,"PrinterView");
	m_printerpages = new KMPages(m_splitter,"PrinterPages");
	m_timer = new QTimer(this);
	m_pop = new QPopupMenu(this);
	m_toolbar = new KToolBar(this, "ToolBar");
	m_plugin = new PluginComboBox(this, "Plugin");
	QLabel	*l1 = new QLabel(i18n("Print system currently used:"), this);
	l1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	// layout
	QVBoxLayout	*m_layout = new QVBoxLayout(this, 0, 0);
	m_layout->addWidget(m_toolbar, 0);
	m_layout->addWidget(m_splitter, 1);
	QHBoxLayout	*lay0 = new QHBoxLayout(0, 0, 10);
	m_layout->addSpacing(5);
	m_layout->addLayout(lay0, 0);
	lay0->addWidget(l1, 1);
	lay0->addWidget(m_plugin, 0);

	// connections
	connect(m_timer,SIGNAL(timeout()),SLOT(slotTimer()));
	connect(m_printerview,SIGNAL(printerSelected(KMPrinter*)),SLOT(slotPrinterSelected(KMPrinter*)));
	connect(m_printerview,SIGNAL(rightButtonClicked(KMPrinter*,const QPoint&)),SLOT(slotRightButtonClicked(KMPrinter*,const QPoint&)));
	connect(m_pop,SIGNAL(aboutToShow()),SLOT(slotShowMenu()));
	connect(m_pop,SIGNAL(aboutToHide()),SLOT(slotHideMenu()));
	connect(m_plugin, SIGNAL(aboutToChange()), SLOT(slotPluginChange()));

	// actions
	m_actions = new KActionCollection(this);
	initActions();

	// first update
	restoreSettings();
	loadParameters();
	slotTimer();
}

KMMainView::~KMMainView()
{
	KMTimer::setMainView(0);
	saveSettings();
	KMFactory::release();
}

void KMMainView::loadParameters()
{
	KConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("General");
	m_timerdelay = conf->readNumEntry("TimerDelay",5);
}

void KMMainView::restoreSettings()
{
	KConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("General");
	setViewType((KMPrinterView::ViewType)conf->readNumEntry("ViewType",KMPrinterView::Icons));
	setOrientation(conf->readNumEntry("Orientation",Qt::Vertical));
	QValueList<int>	sz = conf->readIntListEntry("Sizes");
	while (sz.count() < 2) sz.append(100);
	m_splitter->setSizes(sz);
	bool 	view = conf->readBoolEntry("ViewToolBar",true);
	slotToggleToolBar(view);
	((KToggleAction*)m_actions->action("view_toolbar"))->setChecked(view);
	view = conf->readBoolEntry("ViewPrinterInfos",true);
	slotShowPrinterInfos(view);
	((KToggleAction*)m_actions->action("view_printerinfos"))->setChecked(view);
}

void KMMainView::saveSettings()
{
	KConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("General");
	conf->writeEntry("ViewType",(int)m_printerview->viewType());
	conf->writeEntry("Orientation",(int)m_splitter->orientation());
	conf->writeEntry("Sizes",m_splitter->sizes());
	conf->writeEntry("ViewToolBar",((KToggleAction*)m_actions->action("view_toolbar"))->isChecked());
	conf->writeEntry("ViewPrinterInfos",m_printerpages->isVisible());
	conf->sync();
}

void KMMainView::initActions()
{
	KSelectAction	*vact = new KSelectAction(i18n("View"),0,m_actions,"view_change");
	vact->setItems(QStringList::split(',',i18n("Icons,List,Tree"),false));
	vact->setCurrentItem(0);
	connect(vact,SIGNAL(activated(int)),SLOT(slotChangeView(int)));

	new KAction(i18n("Enable"),"run",0,this,SLOT(slotEnable()),m_actions,"printer_enable");
	new KAction(i18n("Disable"),"stop",0,this,SLOT(slotDisable()),m_actions,"printer_disable");
	new KAction(i18n("Remove"),"edittrash",0,this,SLOT(slotRemove()),m_actions,"printer_remove");
	new KAction(i18n("Configure"),"configure",0,this,SLOT(slotConfigure()),m_actions,"printer_configure");
	new KAction(i18n("Add printer/class..."),"wizard",0,this,SLOT(slotAdd()),m_actions,"printer_add");
	new KAction(i18n("Add special (pseudo) printer..."),"filequickprint",0,this,SLOT(slotAddSpecial()),m_actions,"printer_add_special");
	new KAction(i18n("Set as local default"),"kdeprint_printer",0,this,SLOT(slotHardDefault()),m_actions,"printer_hard_default");
	new KAction(i18n("Set as user default"),"exec",0,this,SLOT(slotSoftDefault()),m_actions,"printer_soft_default");
	new KAction(i18n("Test printer"),"fileprint",0,this,SLOT(slotTest()),m_actions,"printer_test");
	new KAction(i18n("Configure manager"),"configure",0,this,SLOT(slotManagerConfigure()),m_actions,"manager_configure");
	new KAction(i18n("Refresh view"),"reload",0,this,SLOT(slotTimer()),m_actions,"view_refresh");

	KSelectAction	*dact = new KSelectAction(i18n("Orientation"),0,m_actions,"orientation_change");
	dact->setItems(QStringList::split(',',i18n("Vertical,Horizontal"),false));
	dact->setCurrentItem(0);
	connect(dact,SIGNAL(activated(int)),SLOT(slotChangeDirection(int)));

	new KAction(i18n("Restart server"),"gear",0,this,SLOT(slotServerRestart()),m_actions,"server_restart");
	new KAction(i18n("Configure server"),"configure",0,this,SLOT(slotServerConfigure()),m_actions,"server_configure");

	KToggleAction	*tact = new KToggleAction(i18n("View Toolbar"),0,m_actions,"view_toolbar");
	connect(tact,SIGNAL(toggled(bool)),SLOT(slotToggleToolBar(bool)));
	tact = new KToggleAction(i18n("View Printer Informations"),0,m_actions,"view_printerinfos");
	tact->setChecked(true);
	connect(tact,SIGNAL(toggled(bool)),SLOT(slotShowPrinterInfos(bool)));

	// add actions to the toolbar
	m_actions->action("printer_add")->plug(m_toolbar);
	m_actions->action("printer_add_special")->plug(m_toolbar);
	m_toolbar->insertLineSeparator();
	m_actions->action("printer_disable")->plug(m_toolbar);
	m_actions->action("printer_enable")->plug(m_toolbar);
	m_toolbar->insertSeparator();
	m_actions->action("printer_hard_default")->plug(m_toolbar);
	m_actions->action("printer_soft_default")->plug(m_toolbar);
	m_actions->action("printer_remove")->plug(m_toolbar);
	m_toolbar->insertSeparator();
	m_actions->action("printer_configure")->plug(m_toolbar);
	m_actions->action("printer_test")->plug(m_toolbar);
	m_toolbar->insertLineSeparator();
	m_actions->action("server_restart")->plug(m_toolbar);
	m_actions->action("server_configure")->plug(m_toolbar);
	m_toolbar->insertLineSeparator();
	m_actions->action("manager_configure")->plug(m_toolbar);
	m_actions->action("view_refresh")->plug(m_toolbar);

	slotPrinterSelected(0);
}

void KMMainView::startTimer()
{
	if (m_timerdelay > 0)
		m_timer->start(m_timerdelay*1000,true);
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

	// update actions state (only if toolbar enabled, workaround for toolbar
	// problem).
	//if (m_toolbar->isEnabled())
	//{
		int 	mask = (m_manager->hasManagement() ? m_manager->printerOperationMask() : 0);
		bool	sp = !(p && p->isSpecial());
		m_actions->action("printer_remove")->setEnabled(!sp || ((mask & KMManager::PrinterRemoval) && p && p->isLocal() && !p->isImplicit()));
		m_actions->action("printer_configure")->setEnabled(!sp || ((mask & KMManager::PrinterConfigure) && p && !p->isClass(true) && p->isLocal()));
		m_actions->action("printer_hard_default")->setEnabled((sp && (mask & KMManager::PrinterDefault) && p && !p->isClass(true) && !p->isHardDefault() && p->isLocal()));
		m_actions->action("printer_soft_default")->setEnabled((sp && p && !p->isSoftDefault()));
		m_actions->action("printer_test")->setEnabled((sp && (mask & KMManager::PrinterTesting) && p && !p->isClass(true)));
		m_actions->action("printer_enable")->setEnabled((sp && (mask & KMManager::PrinterEnabling) && p && p->state() == KMPrinter::Stopped));
		m_actions->action("printer_disable")->setEnabled((sp && (mask & KMManager::PrinterEnabling) && p && p->state() != KMPrinter::Stopped));

		m_actions->action("printer_add")->setEnabled((mask & KMManager::PrinterCreation));
		mask = m_manager->serverOperationMask();
		m_actions->action("server_restart")->setEnabled((mask & KMManager::ServerRestarting));
		m_actions->action("server_configure")->setEnabled((mask & KMManager::ServerConfigure));
	//}
}

void KMMainView::slotShowMenu()
{
	KMTimer::blockTimer();
}

void KMMainView::slotHideMenu()
{
	KMTimer::releaseTimer(false);
}

void KMMainView::setViewType(int ID)
{
	((KSelectAction*)m_actions->action("view_change"))->setCurrentItem(ID);
	slotChangeView(ID);
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
		if (printer->isLocal() && !printer->isImplicit())
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
		else
		{
			if (!printer->isClass(true))
				m_actions->action("printer_test")->plug(m_pop);
			m_pop->insertSeparator();
		}
	}
	else
	{
		m_actions->action("printer_add")->plug(m_pop);
		m_actions->action("printer_add_special")->plug(m_pop);
		m_pop->insertSeparator();
		m_actions->action("server_restart")->plug(m_pop);
		m_actions->action("server_configure")->plug(m_pop);
		m_pop->insertSeparator();
		m_actions->action("manager_configure")->plug(m_pop);
		m_actions->action("view_refresh")->plug(m_pop);
		m_pop->insertSeparator();
	}
	m_actions->action("view_change")->plug(m_pop);
	m_actions->action("orientation_change")->plug(m_pop);
	m_actions->action("view_toolbar")->plug(m_pop);
	m_actions->action("view_printerinfos")->plug(m_pop);

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
			if (m_current->isSpecial())
			{
				if (!(result=m_manager->removeSpecialPrinter(m_current)))
					showErrorMsg(i18n("Unable to remove special printer <b>%1</b>.").arg(m_current->printerName()));
			}
			else if (!(result=m_manager->removePrinter(m_current)))
				showErrorMsg(i18n("Unable to remove printer <b>%1</b>.").arg(m_current->printerName()));
		KMTimer::releaseTimer(result);
	}
}

void KMMainView::slotConfigure()
{
	if (m_current)
	{
		KMTimer::blockTimer();
		bool	needRefresh(false);
		if (m_current->isSpecial())
		{
			KMSpecialPrinterDlg	dlg(this);
			dlg.setPrinter(m_current);
			if (dlg.exec())
			{
				KMPrinter	*prt = dlg.printer();
				if (prt->name() != m_current->name())
					m_manager->removeSpecialPrinter(m_current);
				m_manager->createSpecialPrinter(prt);
				needRefresh = true;
			}
		}
		else
		{
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
			else
				showErrorMsg(i18n("Unable to load a valid driver for printer <b>%1</b>.").arg(m_current->printerName()));
		}
		KMTimer::releaseTimer(needRefresh);
	}
}

void KMMainView::slotAdd()
{
	KMTimer::blockTimer();

	int	result(0);
	if ((result=add_printer_wizard(this)) == -1)
		showErrorMsg(i18n("Unable to create printer."));

	KMTimer::releaseTimer((result == 1));
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
	m_splitter->setOrientation((d == 1 ? Qt::Horizontal : Qt::Vertical));
}

void KMMainView::slotTest()
{
	if (m_current)
	{
		KMTimer::blockTimer();
		if (KMFactory::self()->manager()->testPrinter(m_current))
			KMessageBox::information(this,i18n("<nobr>Test page successfully sent to printer <b>%1</b>.</nobr>").arg(m_current->printerName()));
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

void KMMainView::slotServerRestart()
{
	KMTimer::blockTimer();
	bool	result = m_manager->restartServer();
	if (!result)
		showErrorMsg(i18n("Unable to restart print server."));
	KMTimer::releaseTimer(result);
}

void KMMainView::slotServerConfigure()
{
	KMTimer::blockTimer();
	bool	result = m_manager->configureServer(this);
	if (!result)
		showErrorMsg(i18n("Unable to configure print server."));
	KMTimer::releaseTimer(result);
}

void KMMainView::slotToggleToolBar(bool on)
{
	if (on) m_toolbar->show();
	else m_toolbar->hide();
}

void KMMainView::slotManagerConfigure()
{
	KMTimer::blockTimer();
	KMConfigDialog	dlg(this,"ConfigDialog");
	bool 	refresh(false);
	if ((refresh=dlg.exec()))
	{
		loadParameters();
	}
	KMTimer::releaseTimer(refresh);
}

void KMMainView::slotAddSpecial()
{
	KMTimer::blockTimer();
	KMSpecialPrinterDlg	dlg(this);
	if (dlg.exec())
	{
		KMPrinter	*prt = dlg.printer();
		m_manager->createSpecialPrinter(prt);
	}
	KMTimer::releaseTimer(true);
}

void KMMainView::slotShowPrinterInfos(bool on)
{
	if (on)
		m_printerpages->show();
	else
		m_printerpages->hide();
}

void KMMainView::enableToolbar(bool on)
{
	KToggleAction	*act = (KToggleAction*)m_actions->action("view_toolbar");
	m_toolbar->setEnabled(on);
	act->setEnabled(on);
	if (on && act->isChecked())
		m_toolbar->show();
	else
		m_toolbar->hide();
}

KAction* KMMainView::action(const char *name)
{
	return m_actions->action(name);
}

void KMMainView::slotPluginChange()
{
	m_printerview->setPrinterList(0);
}

void KMMainView::reload()
{
	slotTimer();
}

#include "kmmainview.moc"
