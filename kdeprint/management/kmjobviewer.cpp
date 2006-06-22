/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
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
#include "kprinter.h"

#include <k3listview.h>
#include <kstatusbar.h>
#include <ktoolbar.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kmenu.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kmenubar.h>
#include <kdebug.h>
#include <kwin.h>
#include <kio/netaccess.h>
#include <qtimer.h>
#include <qlayout.h>
#include <stdlib.h>
#include <qlineedit.h>
#include <kdialog.h>
#include <qcheckbox.h>
#include <kconfig.h>
#include <QDropEvent>
#include <QMenuItem>
#include <kactionmenu.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <k3widgetaction.h>

#undef m_manager
#define	m_manager	KMFactory::self()->jobManager()

class KJobListView : public QTreeWidget
{
public:
	KJobListView( QWidget *parent = 0 );

protected:
	virtual void dragEnterEvent( QDragEnterEvent* );
};

KJobListView::KJobListView( QWidget *parent)
	: QTreeWidget( parent )
{
	setAcceptDrops( true );
}

void KJobListView::dragEnterEvent( QDragEnterEvent *event )
{
	if ( KUrl::List::canDecode( event->mimeData() ) )
		event->acceptProposedAction();
}

KMJobViewer::KMJobViewer(QWidget *parent, const char *name)
: KMainWindow(parent,name)
{
	m_view = 0;
	m_pop = 0;
	m_type = KMJobManager::ActiveJobs;
	m_stickybox = 0;
	m_standalone = ( parent == NULL );

    // FIXME KMainWindow port - need to set each toolbar to not movable
	//setToolBarsMovable(false);
	init();

	if (m_standalone)
	{
		setCaption(i18n("No Printer"));
		KConfig *conf = KMFactory::self()->printConfig();
		QSize defSize( 550, 250 );
		conf->setGroup( "Jobs" );
		resize( conf->readEntry( "Size", defSize ) );
	}
}

KMJobViewer::~KMJobViewer()
{
	if (m_standalone)
	{
		kDebug( 500 ) << "Destroying stand-alone job viewer window" << endl;
		KConfig *conf = KMFactory::self()->printConfig();
		conf->setGroup( "Jobs" );
		conf->writeEntry( "Size", size() );
		emit viewerDestroyed(this);
	}
	removeFromManager();
}

void KMJobViewer::setPrinter(KMPrinter *p)
{
	setPrinter((p ? p->printerName() : QString()));
}

void KMJobViewer::setPrinter(const QString& prname)
{
	// We need to trigger a refresh even if the printer
	// has not changed, some jobs may have been canceled
	// outside kdeprint. We can't return simply if
	// prname == m_prname.
	if (m_prname != prname)
	{
		removeFromManager();
		m_prname = prname;
		addToManager();
		m_view->setAcceptDrops( prname != i18n( "All Printers" ) );
	}
	triggerRefresh();
}

void KMJobViewer::updateCaption()
{
	if (!m_standalone)
		return;

	QString	pixname("fileprint");
	if (!m_prname.isEmpty())
	{
		setCaption(i18n("Print Jobs for %1", m_prname));
		KMPrinter	*prt = KMManager::self()->findPrinter(m_prname);
		if (prt)
			pixname = prt->pixmap();
	}
	else
	{
		setCaption(i18n("No Printer"));
	}
#ifndef Q_OS_WIN
	KWin::setIcons(winId(), DesktopIcon(pixname), SmallIcon(pixname));
#endif
}

void KMJobViewer::updateStatusBar()
{
	if (!m_standalone)
		return;

	int	limit = m_manager->limit();
	if (limit == 0)
		statusBar()->changeItem(i18n("Max.: %1", i18n("Unlimited")), 0);
	else
		statusBar()->changeItem(i18n("Max.: %1", limit), 0);
}

void KMJobViewer::addToManager()
{
	if (m_prname == i18n("All Printers"))
	{
		loadPrinters();
		QListIterator<KMPrinter*>	it(m_printers);
		while ( it.hasNext() ) {
			KMPrinter *printer = it.next();
			m_manager->addPrinter(printer->printerName(), (KMJobManager::JobType)m_type, printer->isSpecial());
		}
	}
	else if (!m_prname.isEmpty())
	{
		KMPrinter *prt = KMManager::self()->findPrinter( m_prname );
		bool isSpecial = ( prt ? prt->isSpecial() : false );
		m_manager->addPrinter(m_prname, (KMJobManager::JobType)m_type, isSpecial);
	}
}

void KMJobViewer::removeFromManager()
{
	if (m_prname == i18n("All Printers"))
	{
		QListIterator<KMPrinter*>	it(m_printers);
		while ( it.hasNext() ) {
			KMPrinter *printer = it.next();
			m_manager->removePrinter(printer->printerName(), (KMJobManager::JobType)m_type);
		}
	}
	else if (!m_prname.isEmpty())
	{
		m_manager->removePrinter(m_prname, (KMJobManager::JobType)m_type);
	}
}

void KMJobViewer::refresh(bool reload)
{
	m_jobs.clear();
	QListIterator<KMJob*>	it(m_manager->jobList(reload));
	bool	all = (m_prname == i18n("All Printers")), active = (m_type == KMJobManager::ActiveJobs);
	while ( it.hasNext() ) {
		KMJob *job = it.next();
		if ((all || job->printer() == m_prname)
		    && ((job->state() >= KMJob::Cancelled && !active)
			    || (job->state() < KMJob::Cancelled && active))
		    && (m_username.isEmpty() || m_username == job->owner()))
			m_jobs.append(job);
	}
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
		m_view = new KJobListView(this);
    QStringList headerLabels;
    headerLabels << i18n("Job ID") << i18n("Owner") << i18n("Name")
                 << i18nc("Status", "State") << i18n("Size (KB)") << i18n("Page(s)");
    m_view->setHeaderLabels(headerLabels);
		connect( m_view, SIGNAL( dropped( QDropEvent*, QTreeWidgetItem* ) ), SLOT( slotDropped( QDropEvent*, QTreeWidgetItem* ) ) );
		KMFactory::self()->uiManager()->setupJobViewer(m_view);
		m_view->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
		m_view->setLineWidth(1);
		connect(m_view,SIGNAL(selectionChanged()),SLOT(slotSelectionChanged()));
		connect(m_view,SIGNAL(rightButtonPressed(QTreeWidgetItem*,const QPoint&,int)),SLOT(slotRightClicked(QTreeWidgetItem*,const QPoint&,int)));
		setCentralWidget(m_view);
	}

	initActions();
}

void KMJobViewer::initActions()
{
	// job actions
	KAction	*hact = new KAction(i18n("&Hold"), actionCollection(),"job_hold");
  hact->setIcon( KIcon( "stop" ) );
  connect( hact, SIGNAL( triggered( bool ) ), this,SLOT(slotHold()) );

	KAction	*ract = new KAction(i18n("&Resume"), actionCollection(),"job_resume");
  ract->setIcon( KIcon( "run" ) );
  connect( ract, SIGNAL( triggered( bool ) ), this,SLOT(slotResume()) );

	KAction	*dact = new KAction(i18n("Remo&ve"), actionCollection(),"job_remove");
  dact->setIcon( KIcon( "edittrash" ) );
  dact->setShortcut( Qt::Key_Delete );
  connect( dact, SIGNAL( triggered( bool ) ), this,SLOT(slotRemove()) );

	KAction *sact = new KAction(i18n("Res&tart"), actionCollection(),"job_restart");
  sact->setIcon( KIcon( "redo" ) );
  connect( sact, SIGNAL( triggered( bool ) ), this,SLOT(slotRestart()) );

	KActionMenu *mact = new KActionMenu(KIcon("fileprint"),i18n("&Move to Printer"),actionCollection(),"job_move");
	mact->setDelayed(false);
	connect(mact->popupMenu(),SIGNAL(triggered(QAction*)),SLOT(slotMove(QAction*)));
	connect(mact->popupMenu(),SIGNAL(aboutToShow()),KMTimer::self(),SLOT(hold()));
	connect(mact->popupMenu(),SIGNAL(aboutToHide()),KMTimer::self(),SLOT(release()));
	connect(mact->popupMenu(),SIGNAL(aboutToShow()),SLOT(slotShowMoveMenu()));
	KToggleAction	*tact = new KToggleAction(i18n("&Toggle Completed Jobs"), actionCollection(),"view_completed");
  tact->setIcon( KIcon( "history" ) );
	tact->setEnabled(m_manager->actions() & KMJob::ShowCompleted);
	connect(tact,SIGNAL(toggled(bool)),SLOT(slotShowCompleted(bool)));
	KToggleAction	*uact = new KToggleAction(i18n("Show Only User Jobs"), actionCollection(), "view_user_jobs");
  uact->setIcon( KIcon( "personal" ) );
	uact->setCheckedState(KGuiItem(i18n("Hide Only User Jobs"),"personal"));
	connect(uact, SIGNAL(toggled(bool)), SLOT(slotUserOnly(bool)));
	m_userfield = new QLineEdit(0);
	m_userfield->setText(getenv("USER"));
	connect(m_userfield, SIGNAL(returnPressed()), SLOT(slotUserChanged()));
	connect(uact, SIGNAL(toggled(bool)), m_userfield, SLOT(setEnabled(bool)));
	m_userfield->setEnabled(false);
	m_userfield->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	K3WidgetAction *ufact = new K3WidgetAction(m_userfield, i18n("User Name"), 0, 0, 0, actionCollection(), "view_username");

	if (!m_pop)
	{
		m_pop = new QMenu(this);
		connect(m_pop,SIGNAL(aboutToShow()),KMTimer::self(),SLOT(hold()));
		connect(m_pop,SIGNAL(aboutToHide()),KMTimer::self(),SLOT(release()));
		m_pop->addAction( hact );
		m_pop->addAction( ract );
		m_pop->addSeparator();
		m_pop->addAction( dact );
		m_pop->addAction( mact );
		m_pop->addSeparator();
		m_pop->addAction( sact );
	}

	// Filter actions
	KActionMenu	*fact = new KActionMenu(KIcon("kdeprint_printer"), i18n("&Select Printer"), actionCollection(), "filter_modify");
	fact->setDelayed(false);
	connect(fact->popupMenu(),SIGNAL(triggered(QAction*)),SLOT(slotPrinterSelected(QAction*)));
	connect(fact->popupMenu(),SIGNAL(aboutToShow()),KMTimer::self(),SLOT(hold()));
	connect(fact->popupMenu(),SIGNAL(aboutToHide()),KMTimer::self(),SLOT(release()));
	connect(fact->popupMenu(),SIGNAL(aboutToShow()),SLOT(slotShowPrinterMenu()));

	if (!m_standalone)
	{
		KToolBar	*toolbar = toolBar();
		toolbar->setMovable(false);
		toolbar->addAction( hact );
		toolbar->addAction( ract );
		toolbar->addSeparator();
		toolbar->addAction( dact );
		toolbar->addAction( mact );
		toolbar->addSeparator();
		toolbar->addAction( sact );
		toolbar->addSeparator();
		toolbar->addAction( tact );
		toolbar->addAction( uact );
		toolbar->addAction( ufact );
	}
	else
	{// stand-alone application
		KStdAction::quit(kapp,SLOT(quit()),actionCollection());
		KStdAction::close(this,SLOT(slotClose()),actionCollection());
		KStdAction::preferences(this, SLOT(slotConfigure()), actionCollection());

		// refresh action
		KAction *action = new KAction(i18n("Refresh"), actionCollection(),"refresh");
    action->setIcon( KIcon( "reload" ) );
    connect( action, SIGNAL( triggered( bool ) ), this, SLOT(slotRefresh() ) );

		// create status bar
		KStatusBar	*statusbar = statusBar();
		m_stickybox = new QCheckBox( i18n( "Keep window permanent" ), statusbar );
		statusbar->addWidget( m_stickybox, 1 );
		statusbar->insertItem(" " + i18n("Max.: %1", i18n("Unlimited"))+ " ", 0, 0);
		statusbar->setItemFixed(0);
		updateStatusBar();

		createGUI();
	}

	loadPluginActions();
	slotSelectionChanged();
}

void KMJobViewer::buildPrinterMenu(QMenu *menu, bool use_all, bool use_specials)
{
	loadPrinters();
	menu->clear();

	QListIterator<KMPrinter*>	it(m_printers);
	int	i(0);
	if (use_all)
	{
		QAction *action = menu->addAction(QIcon(SmallIcon("fileprint")), i18n("All Printers"));
		action->setData(i++);
		menu->addSeparator();
	}
	while ( it.hasNext() )
	{
		i++;
		KMPrinter *printer( it.next() );
		if ( !printer->instanceName().isEmpty() ||
				( printer->isSpecial() && !use_specials ) )
			continue;
		QAction *action = menu->addAction(QIcon(SmallIcon(printer->pixmap())), printer->printerName());
		action->setData(i);
	}
}

void KMJobViewer::slotShowMoveMenu()
{
	QMenu	*menu = static_cast<KActionMenu*>(actionCollection()->action("job_move"))->popupMenu();
	buildPrinterMenu(menu, false, false);
}

void KMJobViewer::slotShowPrinterMenu()
{
	QMenu	*menu = static_cast<KActionMenu*>(actionCollection()->action("filter_modify"))->popupMenu();
	buildPrinterMenu(menu, true, true);
}

void KMJobViewer::updateJobs()
{
	QListIterator<JobItem*>	jit(m_items);
	while (jit.hasNext())
		jit.next()->setDiscarded(true);

	QListIterator<KMJob*>	it(m_jobs);
	while (it.hasNext())
	{
		KMJob	*j(it.next());
		JobItem	*item = findItem(j->uri());
		if (item)
		{
			item->setDiscarded(false);
			item->init(j);
		}
		else
			m_items.append(new JobItem(m_view,j));
	}

	for (int i=0; i<m_items.count(); i++)
		if (m_items.at(i)->isDiscarded())
		{
			delete m_items.takeAt(i);
			i--;
		}

	slotSelectionChanged();
}

JobItem* KMJobViewer::findItem(const QString& uri)
{
	QListIterator<JobItem*>	it(m_items);
	while (it.hasNext()) {
		JobItem *item(it.next());
		if (item->jobUri() == uri) return item;
	}
	return 0;
}

void KMJobViewer::slotSelectionChanged()
{
	int	acts = m_manager->actions();
	int	state(-1);
	int	thread(0);
	bool	completed(true), remote(false);

	QListIterator<JobItem*>	it(m_items);
	QList<KMJob*>	joblist;

  QList<QTreeWidgetItem*> selectedItems = m_view->selectedItems();
	while (it.hasNext())
	{
		JobItem *item(it.next());
		if (selectedItems.contains(item))
		{
			// check if threaded job. "thread" value will be:
			//	0 -> no jobs
			//	1 -> only thread jobs
			//	2 -> only system jobs
			//	3 -> thread and system jobs
			if (item->job()->type() == KMJob::Threaded) thread |= 0x1;
			else thread |= 0x2;

			if (state == -1) state = item->job()->state();
			else if (state != 0 && state != item->job()->state()) state = 0;

			completed = (completed && item->job()->isCompleted());
			joblist.append(item->job());
			if (item->job()->isRemote())
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

void KMJobViewer::jobSelection(QList<KMJob*>& l)
{
  QList<QTreeWidgetItem*> selectedItems = m_view->selectedItems();

	QListIterator<JobItem*>	it(m_items);
	while (it.hasNext()) {
		JobItem *item(it.next());
		if (selectedItems.contains(item))
			l.append(item->job());
	}
}

void KMJobViewer::send(int cmd, const QString& name, const QString& arg)
{
	KMTimer::self()->hold();

	QList<KMJob*>	l;
	jobSelection(l);
	if (!m_manager->sendCommand(l,cmd,arg))
	{
		KMessageBox::error(this,"<qt>"+i18n("Unable to perform action \"%1\" on selected jobs. Error received from manager:", name)+"<p>"+KMManager::self()->errorMsg()+"</p></qt>");
		// error reported, clean it
		KMManager::self()->setErrorMsg(QString());
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

void KMJobViewer::slotMove(QAction *action)
{
  int prID = action->data().toInt();

	if (prID >= 0 && prID < (int)(m_printers.count()))
	{
		KMPrinter	*p = m_printers.at(prID);
		send(KMJob::Move,i18n("Move to %1", p->printerName()),p->printerName());
	}
}

void KMJobViewer::slotRightClicked(QTreeWidgetItem*,const QPoint& p,int)
{
	if (m_pop) m_pop->popup(p);
}

void KMJobViewer::loadPrinters()
{
	m_printers.clear();

	// retrieve printer list without reloading it (faster)
	QListIterator<KMPrinter*>	it(KMFactory::self()->manager()->printerList(false));
	while (it.hasNext())
	{
		KMPrinter *printer(it.next());
		// keep only real printers (no instance, no implicit) and special printers
		if ((printer->isPrinter() || printer->isClass(false) ||
					( printer->isSpecial() && printer->isValid() ) )
				&& (printer->name() == printer->printerName()))
			m_printers.append(printer);
	}
}

void KMJobViewer::slotPrinterSelected(QAction *action)
{
  int prID = action->data().toInt();

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
	if (!m_standalone)
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
	int	mpopindex(7), toolbarindex(!m_standalone?7:8), menuindex(7);
    QAction* item = 0L;

	if (m_standalone)
	{
		// standalone window, insert actions into main menubar
		KAction	*act = actionCollection()->action("job_restart");
        item = act;
		foreach (QWidget* container, act->associatedWidgets())
		{
			if (container == menuBar())
			{
				menuindex = mpopindex = container->actions().indexOf(act);
				break;
			}
		}
	}

	QList<KAction*>	acts = m_manager->createPluginActions(actionCollection());
	foreach (KAction* action, acts)
	{
		// connect the action to this
		connect(action, SIGNAL(triggered(QAction*)), SLOT(pluginActionActivated(QAction*)));

		// should add it to the toolbar and menubar
		action->plug(toolBar(), toolbarindex++);
		if (m_pop)
			action->plug(m_pop, mpopindex++);
		if (item && item->menu())
			action->plug(item->menu(), menuindex++);
	}
}

void KMJobViewer::removePluginActions()
{
	QList<QAction*> actions = KMFactory::self()->manager()->pluginGroup()->actions();
	qDeleteAll(actions);
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
	KMFactory::self()->uiManager()->setupJobViewer(m_view);

	// update the "History" action state
	actionCollection()->action("view_completed")->setEnabled(m_manager->actions() & KMJob::ShowCompleted);
	static_cast<KToggleAction*>(actionCollection()->action("view_completed"))->setChecked(false);
}

void KMJobViewer::closeEvent(QCloseEvent *e)
{
	if (m_standalone)
	{
		hide();
		e->ignore();
	}
	else
		e->accept();
}

void KMJobViewer::pluginActionActivated(QAction *action)
{
  const int ID = action->data().toInt();

	KMTimer::self()->hold();

	QList<KMJob*>	joblist;
	jobSelection(joblist);
	if (!m_manager->doPluginAction(ID, joblist))
		KMessageBox::error(this, "<qt>"+i18n("Operation failed.")+"<p>"+KMManager::self()->errorMsg()+"</p></qt>");

	triggerRefresh();
	KMTimer::self()->release();
}

void KMJobViewer::slotUserOnly(bool on)
{
	m_username = (on ? m_userfield->text() : QString());
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

	KDialog	dlg(this);
  dlg.setCaption( i18n("Print Job Settings") );
  dlg.setButtons( KDialog::Ok|KDialog::Cancel );
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

void KMJobViewer::slotDropped( QDropEvent *e, QTreeWidgetItem* )
{
	QStringList files;
	QString target;

        KUrl::List uris = KUrl::List::fromMimeData( e->mimeData() );
	for ( KUrl::List::ConstIterator it = uris.begin();
	      it != uris.end(); ++it)
	{
		if ( KIO::NetAccess::download( *it, target, 0 ) )
			files << target;
	}

	if ( files.count() > 0 )
	{
		KPrinter prt;
		if ( prt.autoConfigure( m_prname, this ) )
			prt.printFiles( files, false, false );
	}
}

#include "kmjobviewer.moc"
