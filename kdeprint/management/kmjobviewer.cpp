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
#include "jobitem.h"
#include "kmtimer.h"

#include <qlistview.h>
#include <qpopupmenu.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kiconloader.h>
#include <kapp.h>
#include <kcursor.h>

#define	m_manager	KMFactory::self()->jobManager()

KMJobViewer::KMJobViewer(QWidget *parent, const char *name)
: KMainWindow(parent,name)
{
	m_view = 0;
	m_pop = 0;
	m_jobs.setAutoDelete(false);
	m_items.setAutoDelete(false);
	m_printers.setAutoDelete(false);

	setToolBarsMovable(false);
	init();

	if (!parent)
	{
		setCaption(i18n("Print queue"));
		resize(500,200);
	}
}

KMJobViewer::~KMJobViewer()
{
}

void KMJobViewer::setPrinter(KMPrinter *p)
{
	setPrinter((p ? p->printerName() : QString::null));
}

void KMJobViewer::setPrinter(const QString& prname)
{
	m_manager->clearFilter();
	QValueList<KAction*>	acts = actionCollection()->actions("printer_group");
	for (QValueList<KAction*>::ConstIterator it=acts.begin(); it!=acts.end(); ++it)
		((KToggleAction*)(*it))->setChecked(false);
	addPrinter(prname);
}

void KMJobViewer::addPrinter(const QString& prname)
{
	if (!prname.isEmpty())
	{
		m_manager->addPrinter(prname);
		KToggleAction	*act_ = (KToggleAction*)(actionCollection()->action(("printer_"+prname).utf8()));
		if (act_) act_->setChecked(true);
	}
	slotRefresh();
}

void KMJobViewer::refresh()
{
	m_jobs = m_manager->jobList();
	if (m_jobs.count() == 0 && !isVisible() && !parentWidget())
		kapp->quit();
	else
	{
		updateJobs();
		slotSelectionChanged();
		emit jobsShown();
	}
}

void KMJobViewer::init()
{
	if (!m_view)
	{
		m_view = new QListView(this);
		m_view->addColumn(i18n("Job ID"));
		m_view->addColumn(i18n("Printer"));
		m_view->addColumn(i18n("Name"));
		m_view->addColumn(i18n("Owner"));
		m_view->addColumn(i18n("Status", "State"));
		m_view->addColumn(i18n("Size (KB)"));
		m_view->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
		m_view->setLineWidth(1);
		m_view->setSorting(0);
		m_view->setAllColumnsShowFocus(true);
		m_view->setSelectionMode(QListView::Extended);
		connect(m_view,SIGNAL(selectionChanged()),SLOT(slotSelectionChanged()));
		connect(m_view,SIGNAL(rightButtonClicked(QListViewItem*,const QPoint&,int)),SLOT(slotRightClicked(QListViewItem*,const QPoint&,int)));
		connect(m_view,SIGNAL(onItem(QListViewItem*)),SLOT(slotOnItem(QListViewItem*)));
		connect(m_view,SIGNAL(onViewport()),SLOT(slotOnViewport()));
		setCentralWidget(m_view);
	}

	initActions();
}

void KMJobViewer::initActions()
{
	// job actions
	KAction	*hact = new KAction(i18n("Hold"),"stop",0,this,SLOT(slotHold()),actionCollection(),"job_hold");
	KAction	*ract = new KAction(i18n("Resume"),"run",0,this,SLOT(slotResume()),actionCollection(),"job_resume");
	KAction	*dact = new KAction(i18n("Remove"),"edittrash",Qt::Key_Delete,this,SLOT(slotRemove()),actionCollection(),"job_remove");
	KActionMenu *mact = new KActionMenu(i18n("Move to printer..."),"fileprint",actionCollection(),"job_move");
	mact->setDelayed(false);
	connect(mact->popupMenu(),SIGNAL(activated(int)),SLOT(slotMove(int)));
	connect(mact->popupMenu(),SIGNAL(aboutToShow()),SLOT(slotShowMenu()));
	connect(mact->popupMenu(),SIGNAL(aboutToHide()),SLOT(slotHideMenu()));

	if (!m_pop)
	{
		m_pop = new QPopupMenu(this);
		connect(m_pop,SIGNAL(aboutToShow()),SLOT(slotShowMenu()));
		connect(m_pop,SIGNAL(aboutToHide()),SLOT(slotHideMenu()));
		hact->plug(m_pop);
		ract->plug(m_pop);
		m_pop->insertSeparator();
		dact->plug(m_pop);
		m_pop->insertSeparator();
		mact->plug(m_pop);
	}

	// Filter actions
	KActionMenu	*fact = new KActionMenu(i18n("Modify filter..."),"filter",actionCollection(),"filter_modify");
	fact->setDelayed(false);
	new KAction(i18n("All printers"),0,this,SLOT(slotAllPrinters()),actionCollection(),"filter_all");
	new KAction(i18n("No printer"),0,this,SLOT(slotAllPrinters()),actionCollection(),"filter_none");
	new KActionSeparator(actionCollection(),"filter_sep");

	initPrinterActions();

	if (parentWidget())
	{
		KToolBar	*toolbar = toolBar();
		hact->plug(toolbar);
		ract->plug(toolbar);
		toolbar->insertSeparator();
		dact->plug(toolbar);
		toolbar->insertSeparator();
		mact->plug(toolbar);
	}
	else
	{// stand-alone application
		KStdAction::quit(kapp,SLOT(quit()),actionCollection());

		// refresh action
		new KAction(i18n("Refresh"),"reload",0,this,SLOT(slotRefresh()),actionCollection(),"refresh");

		createGUI();
	}

	slotSelectionChanged();
}


void KMJobViewer::initPrinterActions()
{
	loadPrinters();

	// get menus
	KActionMenu *mact = (KActionMenu*)(actionCollection()->action("job_move"));
	KActionMenu *fact = (KActionMenu*)(actionCollection()->action("filter_modify"));

	// some clean-up
	mact->popupMenu()->clear();
	fact->popupMenu()->clear();
	QValueList<KAction*>	acts = actionCollection()->actions("printer_group");
	for (QValueList<KAction*>::ConstIterator it=acts.begin(); it!=acts.end(); ++it)
		delete (*it);

	// initialize "filter" menu
	fact->insert(actionCollection()->action("filter_all"));
	fact->insert(actionCollection()->action("filter_none"));
	fact->insert(actionCollection()->action("filter_sep"));

	// parse printers
	QListIterator<KMPrinter>	it(m_printers);
	for (int i=0;it.current();++it,i++)
	{
		if (!it.current()->instanceName().isEmpty())
			continue;
		mact->popupMenu()->insertItem(SmallIcon(it.current()->pixmap()),it.current()->printerName(),i);
		KToggleAction	*nact = new KToggleAction(it.current()->printerName(),0,actionCollection(),("printer_"+it.current()->printerName()).utf8());
		nact->setGroup("printer_group");
		connect(nact,SIGNAL(toggled(bool)),this,SLOT(slotPrinterToggled(bool)));
		if (m_manager->filter().contains(it.current()->printerName()) > 0)
			nact->setChecked(true);
		fact->insert(nact);
	}
}

void KMJobViewer::updateJobs()
{
	QListIterator<JobItem>	jit(m_items);
	for (;jit.current();++jit)
		jit.current()->setDiscarded(true);

	QListIterator<KMJob>	it(m_jobs);
	for (;it.current();++it)
	{
		KMJob	*j(it.current());
		JobItem	*item = findItem(j->id());
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

JobItem* KMJobViewer::findItem(int ID)
{
	QListIterator<JobItem>	it(m_items);
	for (;it.current();++it)
		if (it.current()->jobID() == ID) return it.current();
	return 0;
}

void KMJobViewer::slotSelectionChanged()
{
	int	acts = m_manager->actions();
	int	state(-1);
	int	thread(0);

	QListIterator<JobItem>	it(m_items);
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
		}
	}

	actionCollection()->action("job_remove")->setEnabled((thread == 1) || ((state >= 0) && (acts & KMJob::Remove)));
	actionCollection()->action("job_hold")->setEnabled((thread == 2) && (state > 0) && (state != KMJob::Held) && (acts & KMJob::Hold));
	actionCollection()->action("job_resume")->setEnabled((thread == 2) && (state > 0) && (state == KMJob::Held) && (acts & KMJob::Resume));
	actionCollection()->action("job_move")->setEnabled((thread == 2) && (state >= 0) && (acts & KMJob::Move));
}

void KMJobViewer::jobSelection(QList<KMJob>& l)
{
	l.setAutoDelete(false);
	QListIterator<JobItem>	it(m_items);
	for (;it.current();++it)
		if (it.current()->isSelected())
			l.append(it.current()->job());
}

void KMJobViewer::send(int cmd, const QString& name, const QString& arg)
{
	KMTimer::blockTimer();

	QList<KMJob>	l;
	jobSelection(l);
	if (!m_manager->sendCommand(l,cmd,arg))
		KMessageBox::error(this,i18n("Unable to perform action \"%1\" on selected jobs !").arg(name));
	refresh();

	KMTimer::releaseTimer();
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
	QListIterator<KMPrinter>	it(*(KMFactory::self()->manager()->printerList(false)));
	for (;it.current();++it)
	{
		// keep only real printers (no class, no instance, no implicit)
		if ((it.current()->isPrinter()) && (it.current()->name() == it.current()->printerName()))
			m_printers.append(it.current());
	}
}

void KMJobViewer::slotPrinterToggled(bool toggle)
{
	QString	name = sender()->name();
	name.replace(0,8,"");
	if (toggle)
		m_manager->addPrinter(name);
	else
		m_manager->removePrinter(name);
	refresh();
}

void KMJobViewer::selectAll()
{
	emit actionCollection()->action("filter_all")->activate();
}

void KMJobViewer::slotAllPrinters()
{
	bool	all_ = (sender()->name() == QString("filter_all"));
	QValueList<KAction*>	acts = actionCollection()->actions("printer_group");
	for (QValueList<KAction*>::ConstIterator it=acts.begin(); it!=acts.end(); ++it)
	{
		((KToggleAction*)(*it))->setChecked(all_);
		QString	actname = (*it)->name();
		actname.replace(0,8,"");
		if (all_)
			m_manager->addPrinter(actname);
		else
			m_manager->removePrinter(actname);
	}
	refresh();
}

void KMJobViewer::slotRefresh()
{
	initPrinterActions();
	refresh();
}

void KMJobViewer::slotShowMenu()
{
	KMTimer::blockTimer();
}

void KMJobViewer::slotHideMenu()
{
	KMTimer::releaseTimer();
}

void KMJobViewer::slotOnItem(QListViewItem*)
{
	m_view->setCursor(KCursor::handCursor());
}

void KMJobViewer::slotOnViewport()
{
	m_view->setCursor(KCursor::arrowCursor());
}
#include "kmjobviewer.moc"
