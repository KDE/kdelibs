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

#include <kmenubar.h>
#include <QtCore/QTimer>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <kmessagebox.h>
#include <kaction.h>
#include <kicon.h>
#include <klocale.h>
#include <kconfig.h>
#include <ktoolbar.h>
#include <kdebug.h>
#include <kmenu.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kdialog.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <ktoolinvocation.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kactionmenu.h>
#include <kdesktopfile.h>

#undef m_manager
#define m_manager KMFactory::self()->manager()

int kdeprint_management_add_printer_wizard(QWidget* parent)
{
    KMWizard dlg(parent);
    int  flag(0);
    if (dlg.exec()) {
        flag = 1;
        // check if the printer already exists, and ask confirmation if needed.
        if (KMFactory::self()->manager()->findPrinter(dlg.printer()->name()) != 0)
            if (KMessageBox::warningContinueCancel(parent, i18n("The printer %1 already exists. Continuing will overwrite existing printer. Do you want to continue?", dlg.printer()->name())) == KMessageBox::Cancel)
                flag = 0;
        // try to add printer only if flag is true.
        if (flag && !KMFactory::self()->manager()->createPrinter(dlg.printer()))
            flag = -1;
    }
    return flag;
}

KMMainView::KMMainView(QWidget *parent, KActionCollection *coll)
        : QWidget(parent)
{
    m_current = 0;
    m_first = true;

    // create widgets
    m_printerview = new KMPrinterView(this);
    m_printerview->setObjectName("PrinterView");
    m_printerpages = new KMPages(this);
    m_printerpages->setObjectName("PrinterPages");
    m_pop = new QMenu(this);
    m_toolbar = new KToolBar(this, "ToolBar");
    m_toolbar->setMovable(false);
    m_plugin = new PluginComboBox(this);
    m_plugin->setObjectName("Plugin");
    /*
    m_menubar = new KMenuBar( this );
    static_cast<KMenuBar*>( m_menubar )->setTopLevelMenu( false );
    */
    m_menubar = new KToolBar(this, false, false);
    m_menubar->setObjectName("MenuBar");
    m_menubar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_menubar->setMovable(false);

    // layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_toolbar);
    layout->addWidget(m_menubar);
    m_boxlayout = new QBoxLayout(QBoxLayout::TopToBottom);
    layout->addLayout(m_boxlayout);
    m_boxlayout->setSpacing(KDialog::spacingHint());
    m_boxlayout->addWidget(m_printerview);
    m_boxlayout->addWidget(m_printerpages);
    layout->addSpacing(5);
    layout->addWidget(m_plugin, 0);

    // connections
    connect(KMTimer::self(), SIGNAL(timeout()), SLOT(slotTimer()));
    connect(m_printerview, SIGNAL(printerSelected(const QString&)), SLOT(slotPrinterSelected(const QString&)));
    connect(m_printerview, SIGNAL(rightButtonClicked(const QString&, const QPoint&)), SLOT(slotRightButtonClicked(const QString&, const QPoint&)));
    connect(m_pop, SIGNAL(aboutToShow()), KMTimer::self(), SLOT(hold()));
    connect(m_pop, SIGNAL(aboutToHide()), KMTimer::self(), SLOT(release()));
    connect(m_manager, SIGNAL(updatePossible(bool)), SLOT(slotUpdatePossible(bool)));

    // actions
    if (coll)
        m_actions = coll;
    else
        m_actions = new KActionCollection(this);
    initActions();

    // first update
    restoreSettings();
    loadParameters();

    // delay first update until KMManager is ready
    reset(i18n("Initializing manager..."), true, true);
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
    KConfig *cf = KMFactory::self()->printConfig();
    KConfigGroup conf = cf->group("General");

    setViewType((KMPrinterView::ViewType)conf.readEntry("ViewType", int(KMPrinterView::Icons)));
    setOrientation(conf.readEntry("Orientation", 1));
    bool  view = conf.readEntry("ViewToolBar", false);
    slotToggleToolBar(view);
    ((KToggleAction*)m_actions->action("view_toolbar"))->setChecked(view);
    view = conf.readEntry("ViewMenuBar", true);
    slotToggleMenuBar(view);
    static_cast<KToggleAction*>(m_actions->action("view_menubar"))->setChecked(view);
    view = conf.readEntry("ViewPrinterInfos", true);
    slotShowPrinterInfos(view);
    ((KToggleAction*)m_actions->action("view_printerinfos"))->setChecked(view);
}

void KMMainView::saveSettings()
{
    KConfig *cf = KMFactory::self()->printConfig();
    KConfigGroup conf = cf->group("General");
    conf.writeEntry("ViewType", (int)m_printerview->viewType());
    conf.writeEntry("Orientation", orientation() == Qt::Horizontal ? 1 : 0);
    conf.writeEntry("ViewToolBar", ((KToggleAction*)m_actions->action("view_toolbar"))->isChecked());
    conf.writeEntry("ViewMenuBar", static_cast<KToggleAction*>(m_actions->action("view_menubar"))->isChecked());
    conf.writeEntry("ViewPrinterInfos", ((KToggleAction*)m_actions->action("view_printerinfos"))->isChecked());
    conf.sync();
}

void KMMainView::initActions()
{
    KIconSelectAction *vact = new KIconSelectAction(i18n("&View"), this);
    m_actions->addAction("view_change", vact);
    QStringList iconlst;
    iconlst << "view_icon" << "fileview-detailed" << "view_tree";
    vact->setItems(i18n("&Icons,&List,&Tree").split(',', QString::SkipEmptyParts), iconlst);
    vact->setCurrentItem(0);
    connect(vact, SIGNAL(triggered(int)), SLOT(slotChangeView(int)));

    KActionMenu *stateAct = new KActionMenu(KIcon("kdeprint-printstate"), i18n("Start/Stop Printer"), this);
    m_actions->addAction("printer_state_change", stateAct);
    stateAct->setDelayed(false);
    KAction *action = new KAction(i18n("&Start Printer"), this);
    m_actions->addAction("printer_start", action);
    action->setIcon(KIcon("kdeprint_enableprinter"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotChangePrinterState()));
    stateAct->addAction(action);

    action = new KAction(i18n("Sto&p Printer"), this);
    m_actions->addAction("printer_stop", action);
    action->setIcon(KIcon("kdeprint_stopprinter"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotChangePrinterState()));
    stateAct->addAction(action);

    stateAct = new KActionMenu(KIcon("printer-queue-state"), i18n("Enable/Disable Job Spooling"), this);
    m_actions->addAction("printer_spool_change", stateAct);
    stateAct->setDelayed(false);

    action = new KAction(i18n("&Enable Job Spooling"), this);
    m_actions->addAction("printer_enable", action);
    action->setIcon(KIcon("kdeprint_enableprinter"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotChangePrinterState()));
    stateAct->addAction(action);

    action = new KAction(i18n("&Disable Job Spooling"), this);
    m_actions->addAction("printer_disable", action);
    action->setIcon(KIcon("kdeprint_stopprinter"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotChangePrinterState()));
    stateAct->addAction(action);

    action = new KAction(i18n("&Remove"), this);
    m_actions->addAction("printer_remove", action);
    action->setIcon(KIcon("user-trash"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotRemove()));

    action = new KAction(i18n("&Configure..."), this);
    m_actions->addAction("printer_configure", action);
    action->setIcon(KIcon("configure"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotConfigure()));

    action = new KAction(i18n("Add &Printer/Class..."), this);
    m_actions->addAction("printer_add", action);
    action->setIcon(KIcon("kdeprint-addprinter"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotAdd()));

    action = new KAction(i18n("Add &Special (pseudo) Printer..."), this);
    m_actions->addAction("printer_add_special", action);
    action->setIcon(KIcon("kdeprint-addpseudo"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotAddSpecial()));

    action = new KAction(i18n("Set as &Local Default"), this);
    m_actions->addAction("printer_hard_default", action);
    action->setIcon(KIcon("kdeprint-defaulthard"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotHardDefault()));

    action = new KAction(i18n("Set as &User Default"), this);
    m_actions->addAction("printer_soft_default", action);
    action->setIcon(KIcon("kdeprint-defaultsoft"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotSoftDefault()));

    action = new KAction(i18n("&Test Printer..."), this);
    m_actions->addAction("printer_test", action);
    action->setIcon(KIcon("kdeprint-testprinter"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotTest()));

    action = new KAction(i18n("Configure &Manager..."), this);
    m_actions->addAction("manager_configure", action);
    action->setIcon(KIcon("kdeprint-configmgr"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotManagerConfigure()));

    action = new KAction(i18n("Initialize Manager/&View"), this);
    m_actions->addAction("view_refresh", action);
    action->setIcon(KIcon("view-refresh"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotInit()));

    KIconSelectAction *dact = new KIconSelectAction(i18n("&Orientation"), this);
    m_actions->addAction("orientation_change", dact);
    iconlst.clear();
    iconlst << "view-top-bottom" << "view-left-right";
    dact->setItems(i18n("&Vertical,&Horizontal").split(',', QString::SkipEmptyParts), iconlst);
    dact->setCurrentItem(0);
    connect(dact, SIGNAL(triggered(int)), SLOT(slotChangeDirection(int)));

    action = new KAction(i18n("R&estart Server"), this);
    m_actions->addAction("server_restart", action);
    action->setIcon(KIcon("kdeprint-restartsrv"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotServerRestart()));

    action = new KAction(i18n("Configure &Server..."), this);
    m_actions->addAction("server_configure", action);
    action->setIcon(KIcon("kdeprint-configsrv"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotServerConfigure()));


    KToggleAction *tact = new KToggleAction(i18n("Show &Toolbar"), this);
    m_actions->addAction("view_toolbar", tact);
    connect(tact, SIGNAL(toggled(bool)), SLOT(slotToggleToolBar(bool)));
    tact = new KToggleAction(i18n("Show Me&nu Toolbar"), this);
    m_actions->addAction("view_menubar", tact);
    connect(tact, SIGNAL(toggled(bool)), SLOT(slotToggleMenuBar(bool)));
    tact = new KToggleAction(i18n("Show Pr&inter Details"), this);
    m_actions->addAction("view_printerinfos", tact);
    tact->setIcon(KIcon("kdeprint-printer-infos"));
    tact->setChecked(true);
    connect(tact, SIGNAL(toggled(bool)), SLOT(slotShowPrinterInfos(bool)));

    tact = new KToggleAction(i18n("Toggle Printer &Filtering"), this);
    m_actions->addAction("view_pfilter", tact);
    tact->setIcon(KIcon("search-filter"));
    tact->setChecked(KMManager::self()->isFilterEnabled());
    connect(tact, SIGNAL(toggled(bool)), SLOT(slotToggleFilter(bool)));

    action = new KAction(i18n("%1 &Handbook" , QString("KDEPrint")), this);
    m_actions->addAction("invoke_help", action);
    action->setIcon(KIcon("help-contents"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotHelp()));

    action = new KAction(i18n("%1 &Web Site" , QString("KDEPrint")), this);
    m_actions->addAction("invoke_web", action);
    action->setIcon(KIcon("network-wired"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotHelp()));

    KActionMenu *mact = new KActionMenu(KIcon("package-utilities"), i18n("Pri&nter Tools"), this);
    m_actions->addAction("printer_tool", mact);
    mact->setDelayed(false);
    connect(mact->menu(), SIGNAL(triggered(QAction*)), SLOT(slotToolSelected(QAction*)));
    QStringList files = KGlobal::dirs()->findAllResources("data", "kdeprint/tools/*.desktop");
    for (QStringList::ConstIterator it = files.begin(); it != files.end(); ++it) {
        const KDesktopFile conf(*it);
        const KConfigGroup cg = conf.desktopGroup();
        QAction *action = mact->menu()->addAction(cg.readEntry("Name", "Unnamed"));
        action->setData(mact->menu()->actions().count());
        m_toollist << cg.readEntry("X-KDE-Library");
    }

    // add actions to the toolbar
    m_toolbar->addAction(m_actions->action("printer_add"));
    m_toolbar->addAction(m_actions->action("printer_add_special"));
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_actions->action("printer_state_change"));
    m_toolbar->addAction(m_actions->action("printer_spool_change"));
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_actions->action("printer_hard_default"));
    m_toolbar->addAction(m_actions->action("printer_soft_default"));
    m_toolbar->addAction(m_actions->action("printer_remove"));
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_actions->action("printer_configure"));
    m_toolbar->addAction(m_actions->action("printer_test"));
    m_toolbar->addAction(m_actions->action("printer_tool"));
    m_pactionsindex = m_toolbar->addSeparator();
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_actions->action("server_restart"));
    m_toolbar->addAction(m_actions->action("server_configure"));
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_actions->action("manager_configure"));
    m_toolbar->addAction(m_actions->action("view_refresh"));
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_actions->action("view_printerinfos"));
    m_toolbar->addAction(m_actions->action("view_change"));
    m_toolbar->addAction(m_actions->action("orientation_change"));
    m_toolbar->addAction(m_actions->action("view_pfilter"));

    // add actions to the menu bar
    KMenu *menu = new KMenu(this);
    menu->addAction(m_actions->action("printer_add"));
    menu->addAction(m_actions->action("printer_add_special"));
    //m_menubar->insertItem( i18n( "Add" ), menu );
    KActionMenu* actionMenu = new KActionMenu(i18n("Add"), this);
    m_actions->addAction("wizard", actionMenu);
    actionMenu->setMenu(menu);
    actionMenu->setDelayed(false);
    m_menubar->addAction(actionMenu);
    menu = new KMenu(this);
    menu->addAction(m_actions->action("printer_state_change"));
    menu->addAction(m_actions->action("printer_spool_change"));
    menu->addSeparator();
    menu->addAction(m_actions->action("printer_hard_default"));
    menu->addAction(m_actions->action("printer_soft_default"));
    menu->addAction(m_actions->action("printer_remove"));
    menu->addSeparator();
    menu->addAction(m_actions->action("printer_configure"));
    menu->addAction(m_actions->action("printer_test"));
    menu->addAction(m_actions->action("printer_tool"));
    menu->addSeparator();
    //m_menubar->insertItem( i18n( "Printer" ), menu );
    actionMenu = new KActionMenu(i18n("Printer"), this);
    m_actions->addAction("printer1", actionMenu);
    actionMenu->setMenu(menu);
    actionMenu->setDelayed(false);
    m_menubar->addAction(actionMenu);
    menu = new KMenu(this);
    menu->addAction(m_actions->action("server_restart"));
    menu->addAction(m_actions->action("server_configure"));
    //m_menubar->insertItem( i18n( "Server" ), menu );
    actionMenu = new KActionMenu(i18n("Print Server"), this);
    m_actions->addAction("misc", actionMenu);
    actionMenu->setMenu(menu);
    actionMenu->setDelayed(false);
    m_menubar->addAction(actionMenu);
    menu = new KMenu(this);
    menu->addAction(m_actions->action("manager_configure"));
    menu->addAction(m_actions->action("view_refresh"));
    //m_menubar->insertItem( i18n( "Manager" ), menu );
    actionMenu = new KActionMenu(i18n("Print Manager"), this);
    m_actions->addAction("kdeprint_configmgr", actionMenu);
    actionMenu->setDelayed(false);
    actionMenu->setMenu(menu);
    m_menubar->addAction(actionMenu);
    menu = new KMenu(this);
    actionMenu->setDelayed(false);
    menu->addAction(m_actions->action("view_printerinfos"));
    menu->addAction(m_actions->action("view_change"));
    menu->addAction(m_actions->action("orientation_change"));
    menu->addAction(m_actions->action("view_toolbar"));
    menu->addAction(m_actions->action("view_menubar"));
    menu->addSeparator();
    menu->addAction(m_actions->action("view_pfilter"));
    //m_menubar->insertItem( i18n( "View" ), menu );
    actionMenu = new KActionMenu(i18n("View"), this);
    m_actions->addAction("view_remove", actionMenu);
    actionMenu->setMenu(menu);
    actionMenu->setDelayed(false);
    m_menubar->addAction(actionMenu);
    //m_menubar->setMinimumHeight( m_menubar->heightForWidth( 1000 ) );
    menu = new KMenu(this);
    menu->addAction(m_actions->action("invoke_help"));
    menu->addAction(m_actions->action("invoke_web"));
    actionMenu = new KActionMenu(i18n("Documentation"), this);
    actionMenu->setDelayed(false);
    m_actions->addAction("help", actionMenu);
    actionMenu->setMenu(menu);
    m_menubar->addAction(actionMenu);

    loadPluginActions();
    slotPrinterSelected(QString());
}

void KMMainView::slotRefresh()
{
    // TODO: remove me
}

void KMMainView::slotTimer()
{
    kDebug() << "KMMainView::slotTimer";
    QList<KMPrinter*> printerlist = m_manager->printerList();
    bool ok = m_manager->errorMsg().isEmpty();
    m_printerview->setPrinterList(&printerlist);
    if (m_first) {
        if (!ok)
            showErrorMsg(i18n("An error occurred while retrieving the printer list."));
        else {
            /* try to select the most appropriate printer:
             *    - soft default owner printer
             *    - hard default printer
             *    - first printer
             */
            QListIterator<KMPrinter*> it(printerlist);
            KMPrinter *p1 = 0, *p2 = 0, *p3 = 0;
            while (it.hasNext()) {
                KMPrinter *printer(it.next());
                if (!printer->isVirtual()) {
                    if (printer->ownSoftDefault()) {
                        p1 = printer;
                        break;
                    } else if (printer->isHardDefault())
                        p2 = printer;
                    else if (!p3)
                        p3 = printer;
                }
            }
            if (p1 || p2 || p3)
                m_printerview->setPrinter(p1 ? p1 : (p2 ? p2 : p3));
        }
        m_first = false;
    }
}

void KMMainView::slotPrinterSelected(const QString& prname)
{
    KMPrinter *p = KMManager::self()->findPrinter(prname);
    m_current = p;
    if (p && !p->isSpecial())
        KMFactory::self()->manager()->completePrinter(p);
    m_printerpages->setPrinter(p);

    // update actions state (only if toolbar enabled, workaround for toolbar
    // problem).
    //if (m_toolbar->isEnabled())
    //{
    int  mask = (m_manager->hasManagement() ? m_manager->printerOperationMask() : 0);
    bool sp = !(p && p->isSpecial());
    m_actions->action("printer_remove")->setEnabled(!sp || ((mask & KMManager::PrinterRemoval) && p && p->isLocal() && !p->isImplicit()));
    m_actions->action("printer_configure")->setEnabled(!sp || ((mask & KMManager::PrinterConfigure) && p && !p->isClass(true) /*&& p->isLocal()*/));
    m_actions->action("printer_hard_default")->setEnabled((sp && (mask & KMManager::PrinterDefault) && p && !p->isClass(true) && !p->isHardDefault() && p->isLocal()));
    m_actions->action("printer_soft_default")->setEnabled((p && !p->isSoftDefault()));
    m_actions->action("printer_test")->setEnabled((sp && (mask & KMManager::PrinterTesting) && p && !p->isClass(true)));
    bool stmask = (sp && (mask & KMManager::PrinterEnabling) && p);
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
{
    return m_printerview->viewType();
}

void KMMainView::slotChangeView(int ID)
{
    kDebug() << "KMMainView::slotChangeView";
    if (ID >= KMPrinterView::Icons && ID <= KMPrinterView::Tree)
        m_printerview->setViewType((KMPrinterView::ViewType)ID);
}

void KMMainView::slotRightButtonClicked(const QString& prname, const QPoint& p)
{
    KMPrinter *printer = KMManager::self()->findPrinter(prname);
    // construct popup menu
    m_pop->clear();
    if (printer) {
        m_current = printer;
        if (!printer->isSpecial()) {
            if (printer->isLocal())
                m_pop->addAction(m_actions->action((printer->state() == KMPrinter::Stopped ? "printer_start" : "printer_stop")));
            m_pop->addAction(m_actions->action((printer->acceptJobs() ? "printer_disable" : "printer_enable")));
            m_pop->addSeparator();
        }
        if (!printer->isSoftDefault())
            m_pop->addAction(m_actions->action("printer_soft_default"));
        if (printer->isLocal() && !printer->isImplicit()) {
            if (!printer->isHardDefault())
                m_pop->addAction(m_actions->action("printer_hard_default"));
            m_pop->addAction(m_actions->action("printer_remove"));
            m_pop->addSeparator();
            if (!printer->isClass(true)) {
                m_pop->addAction(m_actions->action("printer_configure"));
                m_pop->addAction(m_actions->action("printer_test"));
                m_pop->addAction(m_actions->action("printer_tool"));
                m_pop->addSeparator();
            }
        } else {
            if (!printer->isClass(true)) {
                m_pop->addAction(m_actions->action("printer_configure"));
                m_pop->addAction(m_actions->action("printer_test"));
            }
            m_pop->addSeparator();
        }
        if (!printer->isSpecial()) {
            QList<QAction*> pactions = m_manager->pluginGroup()->actions();
            foreach(QAction* action, pactions)
            m_pop->addAction(action);
            if (pactions.count() > 0)
                m_pop->addSeparator();
        }
    } else {
        m_pop->addAction(m_actions->action("printer_add"));
        m_pop->addAction(m_actions->action("printer_add_special"));
        m_pop->addSeparator();
        m_pop->addAction(m_actions->action("server_restart"));
        m_pop->addAction(m_actions->action("server_configure"));
        m_pop->addSeparator();
        m_pop->addAction(m_actions->action("manager_configure"));
        m_pop->addAction(m_actions->action("view_refresh"));
        m_pop->addSeparator();
    }
    m_pop->addAction(m_actions->action("view_printerinfos"));
    m_pop->addAction(m_actions->action("view_change"));
    m_pop->addAction(m_actions->action("orientation_change"));
    m_pop->addAction(m_actions->action("view_toolbar"));
    m_pop->addAction(m_actions->action("view_menubar"));
    m_pop->addSeparator();
    m_pop->addAction(m_actions->action("view_pfilter"));

    // pop the menu
    m_pop->popup(p);
}

void KMMainView::slotChangePrinterState()
{
    QString opname = sender()->objectName();
    if (m_current && opname.startsWith("printer_")) {
        opname = opname.mid(8);
        KMTimer::self()->hold();
        bool result(false);
        if (opname == "enable")
            result = m_manager->enablePrinter(m_current, true);
        else if (opname == "disable")
            result = m_manager->enablePrinter(m_current, false);
        else if (opname == "start")
            result = m_manager->startPrinter(m_current, true);
        else if (opname == "stop")
            result = m_manager->startPrinter(m_current, false);
        if (!result)
            showErrorMsg(i18n("Unable to modify the state of printer %1.", m_current->printerName()));
        KMTimer::self()->release(result);
    }
}

void KMMainView::slotRemove()
{
    if (m_current) {
        KMTimer::self()->hold();
        bool result(false);
        if (KMessageBox::warningYesNo(this, i18n("Do you really want to remove %1?", m_current->printerName())) == KMessageBox::Yes)
            if (m_current->isSpecial()) {
                if (!(result = m_manager->removeSpecialPrinter(m_current)))
                    showErrorMsg(i18n("Unable to remove special printer %1.", m_current->printerName()));
            } else if (!(result = m_manager->removePrinter(m_current)))
                showErrorMsg(i18n("Unable to remove printer %1.", m_current->printerName()));
        KMTimer::self()->release(result);
    }
}

void KMMainView::slotConfigure()
{
    if (m_current) {
        KMTimer::self()->hold();
        bool needRefresh(false);
        if (m_current->isSpecial()) {
            KMSpecialPrinterDlg dlg(this);
            dlg.setPrinter(m_current);
            if (dlg.exec()) {
                KMPrinter *prt = dlg.printer();
                if (prt->name() != m_current->name())
                    m_manager->removeSpecialPrinter(m_current);
                m_manager->createSpecialPrinter(prt);
                needRefresh = true;
            }
        } else {
            DrMain *driver = m_manager->loadPrinterDriver(m_current, true);
            if (driver) {
                KMDriverDialog dlg(this);
                dlg.setCaption(i18n("Configure %1", m_current->printerName()));
                dlg.setDriver(driver);
                // disable OK button for remote printer (read-only dialog)
                if (m_current->isRemote())
                    dlg.enableButtonOk(false);
                if (dlg.exec())
                    if (!m_manager->savePrinterDriver(m_current, driver))
                        showErrorMsg(i18n("Unable to modify settings of printer %1.", m_current->printerName()));
                delete driver;
            } else
                showErrorMsg(i18n("Unable to load a valid driver for printer %1.", m_current->printerName()));
        }
        KMTimer::self()->release(needRefresh);
    }
}

void KMMainView::slotAdd()
{
    KMTimer::self()->hold();

    int result(0);
    if ((result = kdeprint_management_add_printer_wizard(this)) == -1)
        showErrorMsg(i18n("Unable to create printer."));

    KMTimer::self()->release((result == 1));
}

void KMMainView::slotHardDefault()
{
    if (m_current) {
        KMTimer::self()->hold();
        bool result = m_manager->setDefaultPrinter(m_current);
        if (!result)
            showErrorMsg(i18n("Unable to define printer %1 as default.", m_current->printerName()));
        KMTimer::self()->release(result);
    }
}

void KMMainView::slotSoftDefault()
{
    if (m_current) {
        KMTimer::self()->hold();
        KMFactory::self()->virtualManager()->setAsDefault(m_current, QString());
        KMTimer::self()->release(true);
    }
}

void KMMainView::setOrientation(int o)
{
    int  ID = (o == Qt::Horizontal ? 1 : 0);
    ((KSelectAction*)m_actions->action("orientation_change"))->setCurrentItem(ID);
    slotChangeDirection(ID);
}

int KMMainView::orientation() const
{
    return (m_boxlayout->direction() == QBoxLayout::LeftToRight ? Qt::Horizontal : Qt::Vertical);
}

void KMMainView::slotChangeDirection(int d)
{
    m_boxlayout->setDirection(d == 1 ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
}

void KMMainView::slotTest()
{
    if (m_current) {
        KMTimer::self()->hold();
        if (KMessageBox::warningContinueCancel(this, i18n("You are about to print a test page on %1. Do you want to continue?", m_current->printerName()), QString(), KGuiItem(i18n("Print Test Page")), KStandardGuiItem::cancel(), "printTestPage") == KMessageBox::Continue) {
            if (KMFactory::self()->manager()->testPrinter(m_current))
                KMessageBox::information(this, i18n("Test page successfully sent to printer %1.", m_current->printerName()));
            else
                showErrorMsg(i18n("Unable to test printer %1.", m_current->printerName()));
        }
        KMTimer::self()->release(true);
    }
}

void KMMainView::showErrorMsg(const QString& msg, bool usemgr)
{
    QString s(msg);
    if (usemgr) {
        s.prepend("<p>");
        s.append(" ");
        QString err;
        if (m_manager->errorMsg().isEmpty())
            err = i18n("Internal error (no error message).");
        else
            err = m_manager->errorMsg();
        s += i18n("<p>Error message received from manager:</p><p>%1</p>", err);
        // clean up error message
        m_manager->setErrorMsg(QString());
    }
    s.prepend("<qt>").append("</qt>");
    KMTimer::self()->hold();
    KMessageBox::error(this, s);
    KMTimer::self()->release();
}

void KMMainView::slotServerRestart()
{
    KMTimer::self()->hold();
    bool result = m_manager->restartServer();
    if (!result) {
        showErrorMsg(i18n("Unable to restart print server."));
        KMTimer::self()->release(false);
    } else {
        reset(i18n("Restarting server..."), false, false);
    }
}

void KMMainView::slotServerConfigure()
{
    KMTimer::self()->hold();
    bool result = m_manager->configureServer(this);
    if (!result) {
        showErrorMsg(i18n("Unable to configure print server."));
        KMTimer::self()->release(false);
    } else {
        reset(i18n("Configuring server..."), false, false);
    }
}

void KMMainView::slotToggleToolBar(bool on)
{
    if (on) m_toolbar->show();
    else m_toolbar->hide();
}

void KMMainView::slotToggleMenuBar(bool on)
{
    if (on)
        m_menubar->show();
    else
        m_menubar->hide();
}

void KMMainView::slotManagerConfigure()
{
    KMTimer::self()->hold();
    KMConfigDialog dlg(this, "ConfigDialog");
    if (dlg.exec()) {
        loadParameters();
    }
    /* when "OK":
     *  => the config file is saved
     *  => triggering a DCOP signal
     *  => configChanged() called
     * hence no need to refresh, just release the timer
     */
    KMTimer::self()->release(false);
}

void KMMainView::slotAddSpecial()
{
    KMTimer::self()->hold();
    KMSpecialPrinterDlg dlg(this);
    if (dlg.exec()) {
        KMPrinter *prt = dlg.printer();
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
    KToggleAction *act = (KToggleAction*)m_actions->action("view_toolbar");
    m_toolbar->setEnabled(on);
    act->setEnabled(on);
    if (on && act->isChecked())
        m_toolbar->show();
    else
        m_toolbar->hide();
}

QAction* KMMainView::action(const char *name)
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
    connect(m_manager, SIGNAL(updatePossible(bool)), SLOT(slotUpdatePossible(bool)));

    // We must delay the refresh such that all objects has been
    // correctly reloaded (otherwise, crash in KMJobViewer).
    reset(i18n("Initializing manager..."), true, true);
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
    QList<QAction*> pactions = KMFactory::self()->manager()->pluginGroup()->actions();
    QAction* before = m_pactionsindex;
    //QPopupMenu *menu = m_menubar->findItem( m_menubar->idAt( 1 ) )->popup();
    QMenu *menu = static_cast<KActionMenu*>(m_menubar->actions().at(1))->menu();
    foreach(QAction* action, pactions) {
        m_toolbar->insertAction(before, action);
        menu->addAction(action);
        before = action;
    }
}

void KMMainView::removePluginActions()
{
    QList<QAction*> pactions = KMFactory::self()->manager()->pluginGroup()->actions();
    qDeleteAll(pactions);
}

void KMMainView::slotToolSelected(QAction *action)
{
    int ID = action->data().toInt();

    KMTimer::self()->hold();

    QString libname = m_toollist[ID];
    libname.prepend("kdeprint_tool_");
    if (m_current && !m_current->device().isEmpty() && !libname.isEmpty()) {
        KPluginFactory *factory = KPluginLoader(libname.toLocal8Bit()).factory();
        if (factory) {
            QVariantList args;
            args << m_current->device() << m_current->printerName();
            KDialog *dlg = factory->create<KDialog>(this, args);
            if (dlg)
                dlg->exec();
            delete dlg;
        }
    } else
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
    reset(i18n("Initializing manager..."), false, true);
}

void KMMainView::slotUpdatePossible(bool flag)
{
    destroyMessageWindow();
    if (!flag)
        showErrorMsg(i18n("Unable to retrieve the printer list."));
    KMTimer::self()->release(true);
}

void KMMainView::createMessageWindow(const QString& txt, int delay)
{
    destroyMessageWindow();
    MessageWindow::add(m_printerview, txt, delay);
}

void KMMainView::destroyMessageWindow()
{
    MessageWindow::remove(m_printerview);
}

void KMMainView::slotInit()
{
    reset(i18n("Initializing manager..."), true, true);
}

void KMMainView::reset(const QString& msg, bool useDelay, bool holdTimer)
{
    if (holdTimer)
        KMTimer::self()->hold();
    m_printerview->setPrinterList(0);
    if (!msg.isEmpty())
        createMessageWindow(msg, (useDelay ? 500 : 0));
    m_first = true;
    m_manager->checkUpdatePossible();
}

void KMMainView::slotHelp()
{
    QString s = sender()->objectName();
    if (s == "invoke_help")
        KToolInvocation::invokeHelp(QString(), "kdeprint");
    else if (s == "invoke_web") {
        QStringList args;
        args << "exec" << "http://printing.kde.org";
        KToolInvocation::kdeinitExec("kfmclient", args);
    } else
        kDebug(500) << "Unknown help invokator: " << s;
}

#include "kmmainview.moc"
