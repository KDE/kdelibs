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
#include "kmtimer.h"
#include "kmprinterview.h"
#include "kmpages.h"
#include "kmmanager.h"
#include "kmuimanager.h"
#include "kmfactory.h"
#include "kmvirtualmanager.h"
#include "kmprinter.h"
#include "driver.h"
#include "kmdriverdialog.h"
#include "kmwizard.h"
#include "kmconfigdialog.h"
#include "kmspecialprinterdlg.h"
#include "plugincombobox.h"
#include "kiconselectaction.h"
#include "messagewindow.h"

#include <qdockarea.h>
#include <kmenubar.h>
#include <qtimer.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <klocale.h>
#include <kconfig.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kdebug.h>
#include <kpopupmenu.h>
#include <klibloader.h>
#include <kdialogbase.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#undef m_manager
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
				if (KMessageBox::warningYesNo(parent,i18n("The printer %1 already exists. Continuing will overwrite existing printer. Do you want to continue?").arg(dlg.printer()->name())) == KMessageBox::No)
					flag = 0;
			// try to add printer only if flag is true.
			if (flag && !KMFactory::self()->manager()->createPrinter(dlg.printer()))
				flag = -1;
		}
		return flag;
	}
	
	bool config_dialog(QWidget *parent)
	{
		KMConfigDialog	dlg(parent);
		return dlg.exec();
	}
};

KMMainView::KMMainView(QWidget *parent, const char *name, KActionCollection *coll)
: QWidget(parent, name)
{
	m_current = 0;
	m_first = true;
	m_msgwindow = 0;

	// create widgets
	m_printerview = new KMPrinterView(this, "PrinterView");
	m_printerpages = new KMPages(this, "PrinterPages");
	m_pop = new QPopupMenu(this);
	m_toolbar = new KToolBar(this, "ToolBar");
	m_toolbar->setMovingEnabled(false);
	m_plugin = new PluginComboBox(this, "Plugin");
	/*
	m_menubar = new KMenuBar( this );
	static_cast<KMenuBar*>( m_menubar )->setTopLevelMenu( false );
	*/
	m_menubar = new KToolBar( this, "MenuBar", false, false );
	m_menubar->setIconText( KToolBar::IconTextRight );
	m_menubar->setMovingEnabled( false );

	// layout
	QVBoxLayout	*m_layout = new QVBoxLayout(this, 0, 0);
	m_layout->addWidget(m_toolbar);
	m_layout->addWidget( m_menubar );
	m_boxlayout = new QBoxLayout(QBoxLayout::TopToBottom, 0, 0);
	m_layout->addLayout(m_boxlayout);
	m_boxlayout->addWidget(m_printerview);
	m_boxlayout->addWidget(m_printerpages);
	m_layout->addSpacing(5);
	m_layout->addWidget(m_plugin, 0);

	// connections
	connect(KMTimer::self(),SIGNAL(timeout()),SLOT(slotTimer()));
	connect(m_printerview,SIGNAL(printerSelected(const QString&)),SLOT(slotPrinterSelected(const QString&)));
	connect(m_printerview,SIGNAL(rightButtonClicked(const QString&,const QPoint&)),SLOT(slotRightButtonClicked(const QString&,const QPoint&)));
	connect(m_pop,SIGNAL(aboutToShow()),KMTimer::self(),SLOT(hold()));
	connect(m_pop,SIGNAL(aboutToHide()),KMTimer::self(),SLOT(release()));
	connect( m_manager, SIGNAL( updatePossible( bool ) ), SLOT( slotUpdatePossible( bool ) ) );

	// actions
    if (coll)
		m_actions = coll;
	else
		m_actions = new KActionCollection(this);
	initActions();

	// first update
	restoreSettings();
	loadParameters();

	//slotRefresh();
	//KMTimer::self()->release(true);
	KMTimer::self()->hold();
	createMessageWindow( i18n( "Initializing manager..." ) );
	m_manager->checkUpdatePossible();
}

KMMainView::~KMMainView()
{
	saveSettings();
	//KMFactory::release();
}

void KMMainView::loadParameters()
{
}

void KMMainView::restoreSettings()
{
	KConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("General");
	setViewType((KMPrinterView::ViewType)conf->readNumEntry("ViewType",KMPrinterView::Icons));
	setOrientation(conf->readNumEntry("Orientation", Qt::Vertical));
	bool 	view = conf->readBoolEntry("ViewToolBar",false);
	slotToggleToolBar(view);
	((KToggleAction*)m_actions->action("view_toolbar"))->setChecked(view);
	view = conf->readBoolEntry( "ViewMenuBar", true );
	slotToggleMenuBar( view );
	static_cast<KToggleAction*>( m_actions->action( "view_menubar" ) )->setChecked( view );
	view = conf->readBoolEntry("ViewPrinterInfos",true);
	slotShowPrinterInfos(view);
	((KToggleAction*)m_actions->action("view_printerinfos"))->setChecked(view);
}

void KMMainView::saveSettings()
{
	KConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("General");
	conf->writeEntry("ViewType",(int)m_printerview->viewType());
	conf->writeEntry("Orientation",(int)orientation());
	conf->writeEntry("ViewToolBar",((KToggleAction*)m_actions->action("view_toolbar"))->isChecked());
	conf->writeEntry("ViewMenuBar",static_cast<KToggleAction*>( m_actions->action("view_menubar") )->isChecked());
	conf->writeEntry("ViewPrinterInfos",((KToggleAction*)m_actions->action("view_printerinfos"))->isChecked());
	conf->sync();
}

void KMMainView::initActions()
{
	KIconSelectAction	*vact = new KIconSelectAction(i18n("&View"),0,m_actions,"view_change");
	QStringList	iconlst;
	iconlst << "view_icon" << "view_detailed" << "view_tree";
	vact->setItems(QStringList::split(',',i18n("&Icons,&List,&Tree"),false), iconlst);
	vact->setCurrentItem(0);
	connect(vact,SIGNAL(activated(int)),SLOT(slotChangeView(int)));

	KActionMenu	*stateAct = new KActionMenu(i18n("Start/Stop Printer"), "kdeprint_printstate", m_actions, "printer_state_change");
	stateAct->setDelayed(false);
	stateAct->insert(new KAction(i18n("&Start Printer"),"kdeprint_enableprinter",0,this,SLOT(slotChangePrinterState()),m_actions,"printer_start"));
	stateAct->insert(new KAction(i18n("Sto&p Printer"),"kdeprint_stopprinter",0,this,SLOT(slotChangePrinterState()),m_actions,"printer_stop"));

	stateAct = new KActionMenu(i18n("Enable/Disable Job Spooling"), "kdeprint_queuestate", m_actions, "printer_spool_change");
	stateAct->setDelayed(false);
	stateAct->insert(new KAction(i18n("&Enable Job Spooling"),"kdeprint_enableprinter",0,this,SLOT(slotChangePrinterState()),m_actions,"printer_enable"));
	stateAct->insert(new KAction(i18n("&Disable Job Spooling"),"kdeprint_stopprinter",0,this,SLOT(slotChangePrinterState()),m_actions,"printer_disable"));

	new KAction(i18n("&Remove"),"edittrash",0,this,SLOT(slotRemove()),m_actions,"printer_remove");
	new KAction(i18n("&Configure..."),"configure",0,this,SLOT(slotConfigure()),m_actions,"printer_configure");
	new KAction(i18n("Add &Printer/Class..."),"kdeprint_addprinter",0,this,SLOT(slotAdd()),m_actions,"printer_add");
	new KAction(i18n("Add &Special (pseudo) Printer..."),"kdeprint_addpseudo",0,this,SLOT(slotAddSpecial()),m_actions,"printer_add_special");
	new KAction(i18n("Set as &Local Default"),"kdeprint_defaulthard",0,this,SLOT(slotHardDefault()),m_actions,"printer_hard_default");
	new KAction(i18n("Set as &User Default"),"kdeprint_defaultsoft",0,this,SLOT(slotSoftDefault()),m_actions,"printer_soft_default");
	new KAction(i18n("&Test Printer..."),"kdeprint_testprinter",0,this,SLOT(slotTest()),m_actions,"printer_test");
	new KAction(i18n("Configure &Manager..."),"kdeprint_configmgr",0,this,SLOT(slotManagerConfigure()),m_actions,"manager_configure");
	new KAction(i18n("Initialize Manager/&View"),"reload",0,this,SLOT(slotInit()),m_actions,"view_refresh");

	KIconSelectAction	*dact = new KIconSelectAction(i18n("&Orientation"),0,m_actions,"orientation_change");
	iconlst.clear();
	iconlst << "view_top_bottom" << "view_left_right";
	dact->setItems(QStringList::split(',',i18n("&Vertical,&Horizontal"),false), iconlst);
	dact->setCurrentItem(0);
	connect(dact,SIGNAL(activated(int)),SLOT(slotChangeDirection(int)));

	new KAction(i18n("R&estart Server"),"kdeprint_restartsrv",0,this,SLOT(slotServerRestart()),m_actions,"server_restart");
	new KAction(i18n("Configure &Server..."),"kdeprint_configsrv",0,this,SLOT(slotServerConfigure()),m_actions,"server_configure");

	KToggleAction	*tact = new KToggleAction(i18n("View &Toolbar"),0,m_actions,"view_toolbar");
	connect(tact,SIGNAL(toggled(bool)),SLOT(slotToggleToolBar(bool)));
	tact = new KToggleAction( i18n( "View Me&nu Toolbar" ), 0, m_actions, "view_menubar" );
	connect( tact, SIGNAL( toggled( bool ) ), SLOT( slotToggleMenuBar( bool ) ) );
	tact = new KToggleAction(i18n("Show/Hide Pr&inter Details"),"kdeprint_printer_infos", 0,m_actions,"view_printerinfos");
	tact->setChecked(true);
	connect(tact,SIGNAL(toggled(bool)),SLOT(slotShowPrinterInfos(bool)));

	tact = new KToggleAction(i18n("Toggle Printer &Filtering"), "filter", 0, m_actions, "view_pfilter");
	tact->setChecked(KMManager::self()->isFilterEnabled());
	connect(tact, SIGNAL(toggled(bool)), SLOT(slotToggleFilter(bool)));

	KActionMenu	*mact = new KActionMenu(i18n("Pri&nter Tools"), "package_utilities", m_actions, "printer_tool");
	mact->setDelayed(false);
	connect(mact->popupMenu(), SIGNAL(activated(int)), SLOT(slotToolSelected(int)));
	QStringList	files = KGlobal::dirs()->findAllResources("data", "kdeprint/tools/*.desktop");
	for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
	{
		KSimpleConfig	conf(*it);
		conf.setGroup("Desktop Entry");
		mact->popupMenu()->insertItem(conf.readEntry("Name", "Unnamed"), mact->popupMenu()->count());
		m_toollist << conf.readEntry("X-KDE-Library");
	}

	// add actions to the toolbar
	m_actions->action("printer_add")->plug(m_toolbar);
	m_actions->action("printer_add_special")->plug(m_toolbar);
	m_toolbar->insertLineSeparator();
	m_actions->action("printer_state_change")->plug(m_toolbar);
	m_actions->action("printer_spool_change")->plug(m_toolbar);
	m_toolbar->insertSeparator();
	m_actions->action("printer_hard_default")->plug(m_toolbar);
	m_actions->action("printer_soft_default")->plug(m_toolbar);
	m_actions->action("printer_remove")->plug(m_toolbar);
	m_toolbar->insertSeparator();
	m_actions->action("printer_configure")->plug(m_toolbar);
	m_actions->action("printer_test")->plug(m_toolbar);
	m_actions->action("printer_tool")->plug(m_toolbar);
	m_pactionsindex = m_toolbar->insertSeparator();
	m_toolbar->insertLineSeparator();
	m_actions->action("server_restart")->plug(m_toolbar);
	m_actions->action("server_configure")->plug(m_toolbar);
	m_toolbar->insertLineSeparator();
	m_actions->action("manager_configure")->plug(m_toolbar);
	m_actions->action("view_refresh")->plug(m_toolbar);
	m_toolbar->insertLineSeparator();
	m_actions->action("view_printerinfos")->plug(m_toolbar);
	m_actions->action("view_change")->plug(m_toolbar);
	m_actions->action("orientation_change")->plug(m_toolbar);
	m_actions->action("view_pfilter")->plug(m_toolbar);

	// add actions to the menu bar
	QPopupMenu *menu = new QPopupMenu( this );
	m_actions->action( "printer_add" )->plug( menu );
	m_actions->action( "printer_add_special" )->plug( menu );
	//m_menubar->insertItem( i18n( "Add" ), menu );
	m_menubar->insertButton( "wizard", 0, true, i18n( "Add" ) );
	m_menubar->getButton( 0 )->setPopup( menu, true );
	menu = new QPopupMenu( this );
	m_actions->action("printer_state_change")->plug( menu );
	m_actions->action("printer_spool_change")->plug( menu );
	menu->insertSeparator();
	m_actions->action("printer_hard_default")->plug( menu );
	m_actions->action("printer_soft_default")->plug( menu );
	m_actions->action("printer_remove")->plug( menu );
	menu->insertSeparator();
	m_actions->action("printer_configure")->plug( menu );
	m_actions->action("printer_test")->plug( menu );
	m_actions->action("printer_tool")->plug( menu );
	menu->insertSeparator();
	//m_menubar->insertItem( i18n( "Printer" ), menu );
	m_menubar->insertButton( "printer2", 1, true, i18n( "Printer" ) );
	m_menubar->getButton( 1 )->setPopup( menu, true );
	menu = new QPopupMenu( this );
	m_actions->action("server_restart")->plug( menu );
	m_actions->action("server_configure")->plug( menu );
	//m_menubar->insertItem( i18n( "Server" ), menu );
	m_menubar->insertButton( "misc", 2, true, i18n( "Print Server" ) );
	m_menubar->getButton( 2 )->setPopup( menu, true );
	menu = new QPopupMenu( this );
	m_actions->action("manager_configure")->plug( menu );
	m_actions->action("view_refresh")->plug( menu );
	//m_menubar->insertItem( i18n( "Manager" ), menu );
	m_menubar->insertButton( "konsole3", 3, true, i18n( "Print Manager" ) );
	m_menubar->getButton( 3 )->setPopup( menu, true );
	menu = new QPopupMenu( this );
	m_actions->action("view_printerinfos")->plug( menu );
	m_actions->action("view_change")->plug( menu );
	m_actions->action("orientation_change")->plug( menu );
	m_actions->action( "view_toolbar" )->plug ( menu );
	m_actions->action( "view_menubar" )->plug ( menu );
	menu->insertSeparator();
	m_actions->action("view_pfilter")->plug( menu );
	//m_menubar->insertItem( i18n( "View" ), menu );
	m_menubar->insertButton( "view_remove", 4, true, i18n( "View" ) );
	m_menubar->getButton( 4 )->setPopup( menu, true );
	//m_menubar->setMinimumHeight( m_menubar->heightForWidth( 1000 ) );

	loadPluginActions();
	slotPrinterSelected(QString::null);
}

void KMMainView::slotRefresh()
{
	KMTimer::self()->delay(10);
}

void KMMainView::slotTimer()
{
	kdDebug() << "KMMainView::slotTimer" << endl;
	QPtrList<KMPrinter>	*printerlist = m_manager->printerList();
	bool ok = m_manager->errorMsg().isEmpty();
	m_printerview->setPrinterList(printerlist);
	if (!ok && m_first)
	{
		showErrorMsg(i18n("An error occurred while retrieving the printer list."));
		m_first = false;
	}
}

void KMMainView::slotPrinterSelected(const QString& prname)
{
	KMPrinter	*p = KMManager::self()->findPrinter(prname);
	m_current = p;
	if (p && !p->isSpecial())
		KMFactory::self()->manager()->completePrinter(p);
	m_printerpages->setPrinter(p);

	// update actions state (only if toolbar enabled, workaround for toolbar
	// problem).
	//if (m_toolbar->isEnabled())
	//{
		int 	mask = (m_manager->hasManagement() ? m_manager->printerOperationMask() : 0);
		bool	sp = !(p && p->isSpecial());
		m_actions->action("printer_remove")->setEnabled(!sp || ((mask & KMManager::PrinterRemoval) && p && p->isLocal() && !p->isImplicit()));
		m_actions->action("printer_configure")->setEnabled(!sp || ((mask & KMManager::PrinterConfigure) && p && !p->isClass(true) /*&& p->isLocal()*/));
		m_actions->action("printer_hard_default")->setEnabled((sp && (mask & KMManager::PrinterDefault) && p && !p->isClass(true) && !p->isHardDefault() && p->isLocal()));
		m_actions->action("printer_soft_default")->setEnabled((sp && p && !p->isSoftDefault()));
		m_actions->action("printer_test")->setEnabled((sp && (mask & KMManager::PrinterTesting) && p && !p->isClass(true)));
		bool	stmask = (sp && (mask & KMManager::PrinterEnabling) && p);
		m_actions->action("printer_state_change")->setEnabled(stmask && p->isLocal());
		m_actions->action("printer_spool_change")->setEnabled(stmask);
		m_actions->action("printer_start")->setEnabled((stmask && p->state() == KMPrinter::Stopped));
		m_actions->action("printer_stop")->setEnabled((stmask && p->state() != KMPrinter::Stopped));
		m_actions->action("printer_enable")->setEnabled((stmask && !p->acceptJobs()));
		m_actions->action("printer_disable")->setEnabled((stmask && p->acceptJobs()));

		m_actions->action("printer_add")->setEnabled((mask & KMManager::PrinterCreation));
		mask = m_manager->serverOperationMask();
		m_actions->action("server_restart")->setEnabled((mask & KMManager::ServerRestarting));
		m_actions->action("server_configure")->setEnabled((mask & KMManager::ServerConfigure));

		KMFactory::self()->manager()->validatePluginActions(m_actions, p);
	//}
	m_actions->action("printer_tool")->setEnabled(p && !p->isClass(true) && !p->isRemote() && !p->isSpecial());
}

void KMMainView::setViewType(int ID)
{
	((KSelectAction*)m_actions->action("view_change"))->setCurrentItem(ID);
	slotChangeView(ID);
}

int KMMainView::viewType() const
{ return m_printerview->viewType(); }

void KMMainView::slotChangeView(int ID)
{
	kdDebug() << "KMMainView::slotChangeView" << endl;
	if (ID >= KMPrinterView::Icons && ID <= KMPrinterView::Tree)
		m_printerview->setViewType((KMPrinterView::ViewType)ID);
}

void KMMainView::slotRightButtonClicked(const QString& prname, const QPoint& p)
{
	KMPrinter	*printer = KMManager::self()->findPrinter(prname);
	// construct popup menu
	m_pop->clear();
	if (printer)
	{
		m_current = printer;
		if (!printer->isSpecial())
		{
			if (printer->isLocal())
				m_actions->action((printer->state() == KMPrinter::Stopped ? "printer_start" : "printer_stop"))->plug(m_pop);
			m_actions->action((printer->acceptJobs() ? "printer_disable" : "printer_enable"))->plug(m_pop);
			m_pop->insertSeparator();
		}
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
				m_actions->action("printer_tool")->plug(m_pop);
				m_pop->insertSeparator();
			}
		}
		else
		{
			if (!printer->isClass(true))
			{
				m_actions->action("printer_configure")->plug(m_pop);
				m_actions->action("printer_test")->plug(m_pop);
			}
			m_pop->insertSeparator();
		}
		if (!printer->isSpecial())
		{
			QValueList<KAction*>	pactions = m_actions->actions("plugin");
			for (QValueList<KAction*>::Iterator it=pactions.begin(); it!=pactions.end(); ++it)
				(*it)->plug(m_pop);
			if (pactions.count() > 0)
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
	m_actions->action("view_printerinfos")->plug(m_pop);
	m_actions->action("view_change")->plug(m_pop);
	m_actions->action("orientation_change")->plug(m_pop);
	m_actions->action("view_toolbar")->plug(m_pop);
	m_actions->action("view_menubar")->plug(m_pop);
	m_pop->insertSeparator();
	m_actions->action("view_pfilter")->plug(m_pop);

	// pop the menu
	m_pop->popup(p);
}

void KMMainView::slotChangePrinterState()
{
	QString	opname = sender()->name();
	if (m_current && opname.startsWith("printer_"))
	{
		opname = opname.mid(8);
		KMTimer::self()->hold();
		bool	result(false);
		if (opname == "enable")
			result = m_manager->enablePrinter(m_current, true);
		else if (opname == "disable")
			result = m_manager->enablePrinter(m_current, false);
		else if (opname == "start")
			result = m_manager->startPrinter(m_current, true);
		else if (opname == "stop")
			result = m_manager->startPrinter(m_current, false);
		if (!result)
			showErrorMsg(i18n("Unable to modify the state of printer %1.").arg(m_current->printerName()));
		KMTimer::self()->release(result);
	}
}

void KMMainView::slotRemove()
{
	if (m_current)
	{
		KMTimer::self()->hold();
		bool	result(false);
		if (KMessageBox::warningYesNo(this,i18n("Do you really want to remove %1?").arg(m_current->printerName())) == KMessageBox::Yes)
			if (m_current->isSpecial())
			{
				if (!(result=m_manager->removeSpecialPrinter(m_current)))
					showErrorMsg(i18n("Unable to remove special printer %1.").arg(m_current->printerName()));
			}
			else if (!(result=m_manager->removePrinter(m_current)))
				showErrorMsg(i18n("Unable to remove printer %1.").arg(m_current->printerName()));
		KMTimer::self()->release(result);
	}
}

void KMMainView::slotConfigure()
{
	if (m_current)
	{
		KMTimer::self()->hold();
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
				// disable OK button for remote printer (read-only dialog)
				if (m_current->isRemote())
					dlg.enableButtonOK(false);
				if (dlg.exec())
					if (!m_manager->savePrinterDriver(m_current,driver))
						showErrorMsg(i18n("Unable to modify settings of printer %1.").arg(m_current->printerName()));
				delete driver;
			}
			else
				showErrorMsg(i18n("Unable to load a valid driver for printer %1.").arg(m_current->printerName()));
		}
		KMTimer::self()->release(needRefresh);
	}
}

void KMMainView::slotAdd()
{
	KMTimer::self()->hold();

	int	result(0);
	if ((result=add_printer_wizard(this)) == -1)
		showErrorMsg(i18n("Unable to create printer."));

	KMTimer::self()->release((result == 1));
}

void KMMainView::slotHardDefault()
{
	if (m_current)
	{
		KMTimer::self()->hold();
		bool	result = m_manager->setDefaultPrinter(m_current);
		if (!result)
			showErrorMsg(i18n("Unable to define printer %1 as default.").arg(m_current->printerName()));
		KMTimer::self()->release(result);
	}
}

void KMMainView::slotSoftDefault()
{
	if (m_current)
	{
		KMTimer::self()->hold();
		KMFactory::self()->virtualManager()->setAsDefault(m_current,QString::null);
		KMTimer::self()->release(true);
	}
}

void KMMainView::setOrientation(int o)
{
	int 	ID = (o == Qt::Horizontal ? 1 : 0);
	((KSelectAction*)m_actions->action("orientation_change"))->setCurrentItem(ID);
	slotChangeDirection(ID);
}

int KMMainView::orientation() const
{ return (m_boxlayout->direction() == QBoxLayout::LeftToRight ? Qt::Horizontal : Qt::Vertical);  }

void KMMainView::slotChangeDirection(int d)
{
	m_boxlayout->setDirection(d == 1 ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
}

void KMMainView::slotTest()
{
	if (m_current)
	{
		KMTimer::self()->hold();
		if (KMessageBox::warningContinueCancel(this, i18n("You are about to print a test page on %1. Do you want to continue?").arg(m_current->printerName()), QString::null, i18n("Print Test Page"), "printTestPage") == KMessageBox::Continue)
		{
			if (KMFactory::self()->manager()->testPrinter(m_current))
				KMessageBox::information(this,i18n("Test page successfully sent to printer %1.").arg(m_current->printerName()));
			else
				showErrorMsg(i18n("Unable to test printer %1.").arg(m_current->printerName()));
		}
		KMTimer::self()->release(true);
	}
}

void KMMainView::showErrorMsg(const QString& msg, bool usemgr)
{
	QString	s(msg);
	if (usemgr)
	{
		s.prepend("<p>");
		s.append(" ");
		s += i18n("Error message received from manager:</p><p>%1</p>");
		if (m_manager->errorMsg().isEmpty())
			s = s.arg(i18n("Internal error (no error message)."));
		else
			s = s.arg(m_manager->errorMsg());
		// clean up error message
		m_manager->setErrorMsg(QString::null);
	}
	s.prepend("<qt>").append("</qt>");
	KMTimer::self()->hold();
	KMessageBox::error(this,s);
	KMTimer::self()->release();
}

void KMMainView::slotServerRestart()
{
	KMTimer::self()->hold();
	bool	result = m_manager->restartServer();
	if (!result)
	{
		showErrorMsg(i18n("Unable to restart print server."));
		KMTimer::self()->release( false );
	}
	else
	{
		m_printerview->setPrinterList( 0 );
		createMessageWindow( i18n( "Restarting server..." ), 0 );
		m_manager->checkUpdatePossible();
	}
}

void KMMainView::slotServerConfigure()
{
	KMTimer::self()->hold();
	bool	result = m_manager->configureServer(this);
	if (!result)
	{
		showErrorMsg(i18n("Unable to configure print server."));
		KMTimer::self()->release( false );
	}
	else
	{
		m_printerview->setPrinterList( 0 );
		createMessageWindow( i18n( "Configuring server..." ), 0 );
		m_manager->checkUpdatePossible();
	}
}

void KMMainView::slotToggleToolBar(bool on)
{
	if (on) m_toolbar->show();
	else m_toolbar->hide();
}

void KMMainView::slotToggleMenuBar( bool on )
{
	if ( on )
		m_menubar->show();
	else
		m_menubar->hide();
}

void KMMainView::slotManagerConfigure()
{
	KMTimer::self()->hold();
	KMConfigDialog	dlg(this,"ConfigDialog");
	bool 	refresh(false);
	if ((refresh=dlg.exec()))
	{
		loadParameters();
	}
	KMTimer::self()->release(refresh);
}

void KMMainView::slotAddSpecial()
{
	KMTimer::self()->hold();
	KMSpecialPrinterDlg	dlg(this);
	if (dlg.exec())
	{
		KMPrinter	*prt = dlg.printer();
		m_manager->createSpecialPrinter(prt);
	}
	KMTimer::self()->release(true);
}

void KMMainView::slotShowPrinterInfos(bool on)
{
	if (on)
		m_printerpages->show();
	else
		m_printerpages->hide();
	m_actions->action("orientation_change")->setEnabled(on);
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

/*
void KMMainView::aboutToReload()
{
	m_printerview->setPrinterList(0);
}
*/

void KMMainView::reload()
{
	removePluginActions();
	loadPluginActions();

	// redo the connection as the old manager object has been removed
	connect( m_manager, SIGNAL( updatePossible( bool ) ), SLOT( slotUpdatePossible( bool ) ) );

	// We must delay the refresh such that all objects has been
	// correctly reloaded (otherwise, crash in KMJobViewer).
	KMTimer::self()->hold();
	m_printerview->setPrinterList( 0 );
	createMessageWindow( i18n( "Initializing manager..." ) );
	m_manager->checkUpdatePossible();
}

void KMMainView::showPrinterInfos(bool on)
{
	static_cast<KToggleAction*>(m_actions->action("view_printerinfos"))->setChecked(on);
	slotShowPrinterInfos(on);
}

bool KMMainView::printerInfosShown() const
{
	return (static_cast<KToggleAction*>(m_actions->action("view_printerinfos"))->isChecked());
}

void KMMainView::loadPluginActions()
{
	KMFactory::self()->manager()->createPluginActions(m_actions);
	QValueList<KAction*>	pactions = m_actions->actions("plugin");
	int	index = m_pactionsindex;
	//QPopupMenu *menu = m_menubar->findItem( m_menubar->idAt( 1 ) )->popup();
	QPopupMenu *menu = m_menubar->getButton( 1 )->popup();
	for (QValueList<KAction*>::Iterator it=pactions.begin(); it!=pactions.end(); ++it)
	{
		(*it)->plug(m_toolbar, index++);
		( *it )->plug( menu );
	}
}

void KMMainView::removePluginActions()
{
	QValueList<KAction*>	pactions = m_actions->actions("plugin");
	for (QValueList<KAction*>::Iterator it=pactions.begin(); it!=pactions.end(); ++it)
	{
		(*it)->unplugAll();
		delete (*it);
	}
}

void KMMainView::slotToolSelected(int ID)
{
	KMTimer::self()->hold();

	QString	libname = m_toollist[ID];
	libname.prepend("kdeprint_tool_");
	if (m_current && !m_current->device().isEmpty() && !libname.isEmpty())
	{
		KLibFactory	*factory = KLibLoader::self()->factory(libname.local8Bit());
		if (factory)
		{
			QStringList	args;
			args << m_current->device().url() << m_current->printerName();
			KDialogBase	*dlg = static_cast<KDialogBase*>(factory->create(this, "Tool", 0, args));
			if (dlg)
				dlg->exec();
			delete dlg;
		}
	}
	else
		KMessageBox::error(this,
			i18n("Unable to start printer tool. Possible reasons are: "
			     "no printer selected, the selected printer doesn't have "
			     "any local device defined (printer port), or the tool library "
			     "could not be found."));
	
	KMTimer::self()->release();
}

void KMMainView::slotToggleFilter(bool on)
{
	KMTimer::self()->hold();
	KMManager::self()->enableFilter(on);
	KMTimer::self()->release(true);
}

void KMMainView::configChanged()
{
	slotRefresh();
}

void KMMainView::slotUpdatePossible( bool flag )
{
	destroyMessageWindow();
	if ( !flag )
		showErrorMsg( i18n( "Unable to retrieve the printer list." ) );
	KMTimer::self()->release( true );
}

void KMMainView::createMessageWindow( const QString& txt, int delay )
{
	destroyMessageWindow();
	m_msgwindow = new MessageWindow( txt, delay, m_printerview, "MessageWindow" );
}

void KMMainView::destroyMessageWindow()
{
	delete m_msgwindow;
	m_msgwindow = 0;
}

void KMMainView::slotInit()
{
	KMTimer::self()->hold();
	m_printerview->setPrinterList( 0 );
	createMessageWindow( i18n( "Initializing manager..." ) );
	m_manager->checkUpdatePossible();
}

#include "kmmainview.moc"
