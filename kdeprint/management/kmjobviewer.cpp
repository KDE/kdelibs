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

#include "kmjobviewer.h"
#include "kmjobmanager.h"
#include "kmfactory.h"
#include "kmjob.h"
#include "kmprinter.h"
#include "kmmanager.h"
#include "kmuimanager.h"
#include "jobitem.h"
#include "kmtimer.h"
#include "kmconfigjobs.h"
#include "kmconfigpage.h"

#include <klistview.h>
#include <kstatusbar.h>
#include <qpopupmenu.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kmenubar.h>
#include <kdebug.h>
#include <kwin.h>
#include <qtimer.h>
#include <qlayout.h>
#include <stdlib.h>
#include <qlineedit.h>
#include <kdialogbase.h>
#include <qcheckbox.h>

#undef m_manager
#define	m_manager	KMFactory::self()->jobManager()

KMJobViewer::KMJobViewer(QWidget *parent, const char *name)
: KMainWindow(parent,name)
{
	m_view = 0;
	m_pop = 0;
	m_jobs.setAutoDelete(false);
	m_items.setAutoDelete(false);
	m_printers.setAutoDelete(false);
	m_type = KMJobManager::ActiveJobs;
	m_stickybox = 0;

	setToolBarsMovable(false);
	init();

	if (!parent)
	{
		setCaption(i18n("No Printer"));
		resize(550,250);
	}
}

KMJobViewer::~KMJobViewer()
{
	if (!parentWidget())
		emit viewerDestroyed(this);
	removeFromManager();
}

void KMJobViewer::setPrinter(KMPrinter *p)
{
	setPrinter((p ? p->printerName() : QString::null));
}

void KMJobViewer::setPrinter(const QString& prname)
{
	// We need to trigger a refresh even if the printer
	// has not changed, some jobs may have been cancelled
	// outside kdeprint. We can't return simply if
	// prname == m_prname.
	if (m_prname != prname)
	{
		removeFromManager();
		m_prname = prname;
		addToManager();
	}
	triggerRefresh();
}

void KMJobViewer::updateCaption()
{
	if (parentWidget())
		return;

	QString	pixname("fileprint");
	if (!m_prname.isEmpty())
	{
		setCaption(i18n("Print Jobs for %1").arg(m_prname));
		KMPrinter	*prt = KMManager::self()->findPrinter(m_prname);
		if (prt)
			pixname = prt->pixmap();
	}
	else
	{
		setCaption(i18n("No Printer"));
	}
	KWin::setIcons(winId(), DesktopIcon(pixname), SmallIcon(pixname));
}

void KMJobViewer::updateStatusBar()
{
	if (parentWidget())
		return;

	int	limit = m_manager->limit();
	if (limit == 0)
		statusBar()->changeItem(i18n("Max.: %1").arg(i18n("Unlimited")), 0);
	else
		statusBar()->changeItem(i18n("Max.: %1").arg(limit), 0);
}

void KMJobViewer::addToManager()
{
	if (m_prname == i18n("All Printers"))
	{
		loadPrinters();
		QPtrListIterator<KMPrinter>	it(m_printers);
		for (; it.current(); ++it)
			m_manager->addPrinter(it.current()->printerName(), (KMJobManager::JobType)m_type);
	}
	else if (!m_prname.isEmpty())
	{
		m_manager->addPrinter(m_prname, (KMJobManager::JobType)m_type);
	}
}

void KMJobViewer::removeFromManager()
{
	if (m_prname == i18n("All Printers"))
	{
		QPtrListIterator<KMPrinter>	it(m_printers);
		for (; it.current(); ++it)
			m_manager->removePrinter(it.current()->printerName(), (KMJobManager::JobType)m_type);
	}
	else if (!m_prname.isEmpty())
	{
		m_manager->removePrinter(m_prname, (KMJobManager::JobType)m_type);
	}
}

void KMJobViewer::refresh(bool reload)
{
	m_jobs.clear();
	QPtrListIterator<KMJob>	it(m_manager->jobList(reload));
	bool	all = (m_prname == i18n("All Printers")), active = (m_type == KMJobManager::ActiveJobs);
	for (; it.current(); ++it)
		if ((all || it.current()->printer() == m_prname)
		    && ((it.current()->state() >= KMJob::Cancelled && !active)
			    || (it.current()->state() < KMJob::Cancelled && active))
		    && (m_username.isEmpty() || m_username == it.current()->owner()))
			m_jobs.append(it.current());
	updateJobs();


	// update the caption and icon (doesn't do anything if it has a parent widget)
	updateCaption();

	updateStatusBar();

	// do it last as this signal can cause this view to be destroyed. No
	// code can be executed safely after that
	emit jobsShown(this, (m_jobs.count() != 0));
}

void KMJobViewer::init()
{
	if (!m_view)
	{
		m_view = new KListView(this);
		m_view->addColumn(i18n("Job ID"));
		m_view->addColumn(i18n("Owner"));
		m_view->addColumn(i18n("Name"), 150);
		m_view->addColumn(i18n("Status", "State"));
		m_view->addColumn(i18n("Size (KB)"));
		m_view->addColumn(i18n("Page(s)"));
		m_view->setColumnAlignment(5,Qt::AlignRight|Qt::AlignVCenter);
		//m_view->addColumn(i18n("Printer"));
		//m_view->setColumnAlignment(6,Qt::AlignRight|Qt::AlignVCenter);
		KMFactory::self()->uiManager()->setupJobViewer(m_view);
		m_view->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
		m_view->setLineWidth(1);
		m_view->setSorting(0);
		m_view->setAllColumnsShowFocus(true);
		m_view->setSelectionMode(QListView::Extended);
		connect(m_view,SIGNAL(selectionChanged()),SLOT(slotSelectionChanged()));
		connect(m_view,SIGNAL(rightButtonClicked(QListViewItem*,const QPoint&,int)),SLOT(slotRightClicked(QListViewItem*,const QPoint&,int)));
		setCentralWidget(m_view);
	}

	initActions();
}

void KMJobViewer::initActions()
{
	// job actions
	KAction	*hact = new KAction(i18n("&Hold"),"stop",0,this,SLOT(slotHold()),actionCollection(),"job_hold");
	KAction	*ract = new KAction(i18n("&Resume"),"run",0,this,SLOT(slotResume()),actionCollection(),"job_resume");
	KAction	*dact = new KAction(i18n("R&emove"),"edittrash",Qt::Key_Delete,this,SLOT(slotRemove()),actionCollection(),"job_remove");
	KAction *sact = new KAction(i18n("Res&tart"),"redo",0,this,SLOT(slotRestart()),actionCollection(),"job_restart");
	KActionMenu *mact = new KActionMenu(i18n("&Move to Printer..."),"fileprint",actionCollection(),"job_move");
	mact->setDelayed(false);
	connect(mact->popupMenu(),SIGNAL(activated(int)),SLOT(slotMove(int)));
	connect(mact->popupMenu(),SIGNAL(aboutToShow()),KMTimer::self(),SLOT(hold()));
	connect(mact->popupMenu(),SIGNAL(aboutToHide()),KMTimer::self(),SLOT(release()));
	connect(mact->popupMenu(),SIGNAL(aboutToShow()),SLOT(slotShowMoveMenu()));
	KToggleAction	*tact = new KToggleAction(i18n("&Toggle Completed Jobs"),"history",0,actionCollection(),"view_completed");
	tact->setEnabled(m_manager->actions() & KMJob::ShowCompleted);
	connect(tact,SIGNAL(toggled(bool)),SLOT(slotShowCompleted(bool)));
	KToggleAction	*uact = new KToggleAction(i18n("Show Only User Jobs"), "personal", 0, actionCollection(), "view_user_jobs");
	connect(uact, SIGNAL(toggled(bool)), SLOT(slotUserOnly(bool)));
	m_userfield = new QLineEdit(0);
	m_userfield->setText(getenv("USER"));
	connect(m_userfield, SIGNAL(returnPressed()), SLOT(slotUserChanged()));
	connect(uact, SIGNAL(toggled(bool)), m_userfield, SLOT(setEnabled(bool)));
	m_userfield->setEnabled(false);
	m_userfield->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	KWidgetAction	*ufact = new KWidgetAction(m_userfield, i18n("User name"), 0, 0, 0, actionCollection(), "view_username");

	if (!m_pop)
	{
		m_pop = new QPopupMenu(this);
		connect(m_pop,SIGNAL(aboutToShow()),KMTimer::self(),SLOT(hold()));
		connect(m_pop,SIGNAL(aboutToHide()),KMTimer::self(),SLOT(release()));
		hact->plug(m_pop);
		ract->plug(m_pop);
		m_pop->insertSeparator();
		dact->plug(m_pop);
		mact->plug(m_pop);
		m_pop->insertSeparator();
		sact->plug(m_pop);
	}

	// Filter actions
	KActionMenu	*fact = new KActionMenu(i18n("&Select Printer"), "kdeprint_printer", actionCollection(), "filter_modify");
	fact->setDelayed(false);
	connect(fact->popupMenu(),SIGNAL(activated(int)),SLOT(slotPrinterSelected(int)));
	connect(fact->popupMenu(),SIGNAL(aboutToShow()),KMTimer::self(),SLOT(hold()));
	connect(fact->popupMenu(),SIGNAL(aboutToHide()),KMTimer::self(),SLOT(release()));
	connect(fact->popupMenu(),SIGNAL(aboutToShow()),SLOT(slotShowPrinterMenu()));

	if (parentWidget())
	{
		KToolBar	*toolbar = toolBar();
		hact->plug(toolbar);
		ract->plug(toolbar);
		toolbar->insertSeparator();
		dact->plug(toolbar);
		mact->plug(toolbar);
		toolbar->insertSeparator();
		sact->plug(toolbar);
		toolbar->insertSeparator();
		tact->plug(toolbar);
		uact->plug(toolbar);
		ufact->plug(toolbar);
	}
	else
	{// stand-alone application
		KStdAction::quit(kapp,SLOT(quit()),actionCollection());
		KStdAction::close(this,SLOT(slotClose()),actionCollection());
		KStdAction::preferences(this, SLOT(slotConfigure()), actionCollection());

		// refresh action
		new KAction(i18n("Refresh"),"reload",0,this,SLOT(slotRefresh()),actionCollection(),"refresh");

		// create status bar
		KStatusBar	*statusbar = statusBar();
		m_stickybox = new QCheckBox( i18n( "Keep window permanent" ), statusbar );
		statusbar->addWidget( m_stickybox, 1, false );
		statusbar->insertItem(" " + i18n("Max.: %1").arg(i18n("Unlimited"))+ " ", 0, 0, true);
		statusbar->setItemFixed(0);
		updateStatusBar();

		createGUI();
	}

	loadPluginActions();
	slotSelectionChanged();
}

void KMJobViewer::buildPrinterMenu(QPopupMenu *menu, bool use_all)
{
	loadPrinters();
	menu->clear();

	QPtrListIterator<KMPrinter>	it(m_printers);
	int	i(0);
	if (use_all)
	{
		menu->insertItem(SmallIcon("fileprint"), i18n("All Printers"), i++);
		menu->insertSeparator();
	}
	for (; it.current(); ++it, i++)
	{
		if (!it.current()->instanceName().isEmpty())
			continue;
		menu->insertItem(SmallIcon(it.current()->pixmap()), it.current()->printerName(), i);
	}
}

void KMJobViewer::slotShowMoveMenu()
{
	QPopupMenu	*menu = static_cast<KActionMenu*>(actionCollection()->action("job_move"))->popupMenu();
	buildPrinterMenu(menu, false);
}

void KMJobViewer::slotShowPrinterMenu()
{
	QPopupMenu	*menu = static_cast<KActionMenu*>(actionCollection()->action("filter_modify"))->popupMenu();
	buildPrinterMenu(menu, true);
}

void KMJobViewer::updateJobs()
{
	QPtrListIterator<JobItem>	jit(m_items);
	for (;jit.current();++jit)
		jit.current()->setDiscarded(true);

	QPtrListIterator<KMJob>	it(m_jobs);
	for (;it.current();++it)
	{
		KMJob	*j(it.current());
		JobItem	*item = findItem(j->uri());
		if (item)
		{
			item->setDiscarded(false);
			item->init(j);
		}
		else
			m_items.append(new JobItem(m_view,j));
	}

	for (uint i=0; i<m_items.count(); i++)
		if (m_items.at(i)->isDiscarded())
		{
			delete m_items.take(i);
			i--;
		}
}

JobItem* KMJobViewer::findItem(const QString& uri)
{
	QPtrListIterator<JobItem>	it(m_items);
	for (;it.current();++it)
		if (it.current()->jobUri() == uri) return it.current();
	return 0;
}

void KMJobViewer::slotSelectionChanged()
{
	int	acts = m_manager->actions();
	int	state(-1);
	int	thread(0);
	bool	completed(true), remote(false);

	QPtrListIterator<JobItem>	it(m_items);
	QPtrList<KMJob>	joblist;

	joblist.setAutoDelete(false);
	for (;it.current();++it)
	{
		if (it.current()->isSelected())
		{
			// check if threaded job. "thread" value will be:
			//	0 -> no jobs
			//	1 -> only thread jobs
			//	2 -> only system jobs
			//	3 -> thread and system jobs
			if (it.current()->job()->type() == KMJob::Threaded) thread |= 0x1;
			else thread |= 0x2;

			if (state == -1) state = it.current()->job()->state();
			else if (state != 0 && state != it.current()->job()->state()) state = 0;

			completed = (completed && it.current()->job()->isCompleted());
			joblist.append(it.current()->job());
			if (it.current()->job()->isRemote())
				remote = true;
		}
	}
	if (thread != 2)
		joblist.clear();

	actionCollection()->action("job_remove")->setEnabled((thread == 1) || (/*!remote &&*/ !completed && (state >= 0) && (acts & KMJob::Remove)));
	actionCollection()->action("job_hold")->setEnabled(/*!remote &&*/ !completed && (thread == 2) && (state > 0) && (state != KMJob::Held) && (acts & KMJob::Hold));
	actionCollection()->action("job_resume")->setEnabled(/*!remote &&*/ !completed && (thread == 2) && (state > 0) && (state == KMJob::Held) && (acts & KMJob::Resume));
	actionCollection()->action("job_move")->setEnabled(!remote && !completed && (thread == 2) && (state >= 0) && (acts & KMJob::Move));
	actionCollection()->action("job_restart")->setEnabled(!remote && (thread == 2) && (state >= 0) && (completed) && (acts & KMJob::Restart));

	m_manager->validatePluginActions(actionCollection(), joblist);
}

void KMJobViewer::jobSelection(QPtrList<KMJob>& l)
{
	l.setAutoDelete(false);
	QPtrListIterator<JobItem>	it(m_items);
	for (;it.current();++it)
		if (it.current()->isSelected())
			l.append(it.current()->job());
}

void KMJobViewer::send(int cmd, const QString& name, const QString& arg)
{
	KMTimer::self()->hold();

	QPtrList<KMJob>	l;
	jobSelection(l);
	if (!m_manager->sendCommand(l,cmd,arg))
	{
		KMessageBox::error(this,"<qt>"+i18n("Unable to perform action \"%1\" on selected jobs. Error received from manager:").arg(name)+"<p>"+KMManager::self()->errorMsg()+"</p></qt>");
		// error reported, clean it
		KMManager::self()->setErrorMsg(QString::null);
	}

	triggerRefresh();

	KMTimer::self()->release();
}

void KMJobViewer::slotHold()
{
	send(KMJob::Hold,i18n("Hold"));
}

void KMJobViewer::slotResume()
{
	send(KMJob::Resume,i18n("Resume"));
}

void KMJobViewer::slotRemove()
{
	send(KMJob::Remove,i18n("Remove"));
}

void KMJobViewer::slotRestart()
{
	send(KMJob::Restart,i18n("Restart"));
}

void KMJobViewer::slotMove(int prID)
{
	if (prID >= 0 && prID < (int)(m_printers.count()))
	{
		KMPrinter	*p = m_printers.at(prID);
		send(KMJob::Move,i18n("Move to %1").arg(p->printerName()),p->printerName());
	}
}

void KMJobViewer::slotRightClicked(QListViewItem*,const QPoint& p,int)
{
	if (m_pop) m_pop->popup(p);
}

void KMJobViewer::loadPrinters()
{
	m_printers.clear();

	// retrieve printer list without reloading it (faster)
	QPtrListIterator<KMPrinter>	it(*(KMFactory::self()->manager()->printerList(false)));
	for (;it.current();++it)
	{
		// keep only real printers (no instance, no implicit)
		if ((it.current()->isPrinter() || it.current()->isClass(false)) && (it.current()->name() == it.current()->printerName()))
			m_printers.append(it.current());
	}
}

void KMJobViewer::slotPrinterSelected(int prID)
{
	if (prID >= 0 && prID < (int)(m_printers.count()+1))
	{
		QString	prname = (prID == 0 ? i18n("All Printers") : m_printers.at(prID-1)->printerName());
		emit printerChanged(this, prname);
	}
}

void KMJobViewer::slotRefresh()
{
	triggerRefresh();
}

void KMJobViewer::triggerRefresh()
{
	// parent widget -> embedded in KControl and needs
	// to update itself. Otherwise, it's standalone
	// kjobviewer and we need to synchronize all possible
	// opened windows -> do the job on higher level.
	if (parentWidget())
		refresh(true);
	else
		emit refreshClicked();
}

void KMJobViewer::slotShowCompleted(bool on)
{
	removeFromManager();
	m_type = (on ? KMJobManager::CompletedJobs : KMJobManager::ActiveJobs);
	addToManager();
	triggerRefresh();
}

void KMJobViewer::slotClose()
{
	delete this;
}

void KMJobViewer::loadPluginActions()
{
	int	mpopindex(7), toolbarindex(parentWidget()?7:8), menuindex(7);
	QMenuData	*menu(0);

	if (!parentWidget())
	{
		// standalone window, insert actions into main menubar
		KAction	*act = actionCollection()->action("job_restart");
		for (int i=0;i<act->containerCount();i++)
		{
			if (menuBar()->findItem(act->itemId(i), &menu))
			{
				menuindex = mpopindex = menu->indexOf(act->itemId(i))+1;
				break;
			}
		}
	}

	QValueList<KAction*>	acts = m_manager->createPluginActions(actionCollection());
	for (QValueListIterator<KAction*> it=acts.begin(); it!=acts.end(); ++it)
	{
		// connect the action to this
		connect((*it), SIGNAL(activated(int)), SLOT(pluginActionActivated(int)));

		// should add it to the toolbar and menubar
		(*it)->plug(toolBar(), toolbarindex++);
		if (m_pop)
			(*it)->plug(m_pop, mpopindex++);
		if (menu)
			(*it)->plug(static_cast<QPopupMenu*>(menu), menuindex++);
	}
}

void KMJobViewer::removePluginActions()
{
	QValueList<KAction*>	acts = actionCollection()->actions("plugin");
	for (QValueListIterator<KAction*> it=acts.begin(); it!=acts.end(); ++it)
	{
		(*it)->unplugAll();
		delete (*it);
	}
}

/*
void KMJobViewer::aboutToReload()
{
	if (m_view)
	{
		m_view->clear();
		m_items.clear();
	}
	m_jobs.clear();
}
*/

void KMJobViewer::reload()
{
	removePluginActions();
	loadPluginActions();
	// re-add the current printer to the job manager: the job
	// manager has been destroyed, so the new one doesn't know
	// which printer it has to list
	addToManager();
	// no refresh needed: view has been cleared before reloading
	// and the actual refresh will be triggered either by the KControl
	// module, or by KJobViewerApp using timer.
	
	// reload the columns needed: remove the old one
	for (int c=m_view->columns()-1; c>5; c--)
		m_view->removeColumn(c);
	KMFactory::self()->uiManager()->setupJobViewer(m_view);

	// update the "History" action state
	actionCollection()->action("view_completed")->setEnabled(m_manager->actions() & KMJob::ShowCompleted);
	static_cast<KToggleAction*>(actionCollection()->action("view_completed"))->setChecked(false);
}

void KMJobViewer::closeEvent(QCloseEvent *e)
{
	if (!parentWidget())
	{
		hide();
		e->ignore();
	}
	else
		e->accept();
}

void KMJobViewer::pluginActionActivated(int ID)
{
	KMTimer::self()->hold();

	QPtrList<KMJob>	joblist;
	jobSelection(joblist);
	if (!m_manager->doPluginAction(ID, joblist))
		KMessageBox::error(this, "<qt>"+i18n("Operation failed.")+"<p>"+KMManager::self()->errorMsg()+"</p></qt>");

	triggerRefresh();
	KMTimer::self()->release();
}

void KMJobViewer::slotUserOnly(bool on)
{
	m_username = (on ? m_userfield->text() : QString::null);
	refresh(false);
}

void KMJobViewer::slotUserChanged()
{
	if (m_userfield->isEnabled())
	{
		m_username = m_userfield->text();
		refresh(false);
	}
}

void KMJobViewer::slotConfigure()
{
	KMTimer::self()->hold();

	KDialogBase	dlg(this, 0, true, i18n("Print jobs settings"), KDialogBase::Ok|KDialogBase::Cancel);
	KMConfigJobs	*w = new KMConfigJobs(&dlg);
	dlg.setMainWidget(w);
	dlg.resize(300, 10);
	KConfig	*conf = KMFactory::self()->printConfig();
	w->loadConfig(conf);
	if (dlg.exec())
	{
		w->saveConfig(conf);
		updateStatusBar();
		refresh(true);
	}

	KMTimer::self()->release();
}

bool KMJobViewer::isSticky() const
{
	return ( m_stickybox ? m_stickybox->isChecked() : false );
}

#include "kmjobviewer.moc"
