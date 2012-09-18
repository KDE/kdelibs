/*
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2007-2011 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Riccardo Iaconelli <riccardo@kde.org>
 *   Copyright (c) 2009 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "coronabase.h"
#include "private/coronabase_p.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsView>
#include <QGraphicsGridLayout>
#include <QMimeData>
#include <QPainter>
#include <QTimer>

#include <cmath>

#include <kaction.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kshortcutsdialog.h>
#include <kwindowsystem.h>

#include "abstractdialogmanager.h"
#include "abstracttoolbox.h"
#include "containment.h"
#include "containmentactionspluginsconfig.h"
#include "pluginloader.h"
#include "private/applet_p.h"
#include "private/containment_p.h"
#include "tooltipmanager.h"
#include "view.h"

using namespace Plasma;

namespace Plasma
{

bool CoronaBasePrivate::s_positioningContainments = false;

CoronaBase::CoronaBase(QObject *parent)
    : QObject(parent),
      d(new CoronaBasePrivate(this))
{
#ifndef NDEBUG
    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Corona ctor start";
#endif
    d->init();
    //setViewport(new QGLWidget(QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)));
}

CoronaBase::~CoronaBase()
{
    KConfigGroup trans(KSharedConfig::openConfig(), "PlasmaTransientsConfig");
    trans.deleteGroup();

    delete d;
}

void CoronaBase::setAppletMimeType(const QString &type)
{
    d->mimetype = type;
}

QString CoronaBase::appletMimeType()
{
    return d->mimetype;
}

void CoronaBase::setDefaultContainmentPlugin(const QString &name)
{
    // we could check if it is in:
    // Containment::listContainments().contains(name) ||
    // Containment::listContainments(QString(), QCoreApplication::instance()->applicationName()).contains(name)
    // but that seems like overkill
    d->defaultContainmentPlugin = name;
}

QString CoronaBase::defaultContainmentPlugin() const
{
    return d->defaultContainmentPlugin;
}

void CoronaBase::saveLayout(const QString &configName) const
{
    KSharedConfigPtr c;

    if (configName.isEmpty() || configName == d->configName) {
        c = config();
    } else {
        c = KSharedConfig::openConfig(configName, KConfig::SimpleConfig);
    }

    d->saveLayout(c);
}

void CoronaBase::exportLayout(KConfigGroup &config, QList<Containment*> containments)
{
    foreach (const QString &group, config.groupList()) {
        KConfigGroup cg(&config, group);
        cg.deleteGroup();
    }

    //temporarily unlock so that removal works
    ImmutabilityType oldImm = immutability();
    d->immutability = Mutable;

    KConfigGroup dest(&config, "Containments");
    KConfigGroup dummy;
    foreach (Plasma::Containment *c, containments) {
        c->save(dummy);
        c->config().reparent(&dest);

        //ensure the containment is unlocked
        //this is done directly because we have to bypass any SystemImmutable checks
        c->Applet::d->immutability = Mutable;
        foreach (Applet *a, c->applets()) {
            a->d->immutability = Mutable;
        }

        c->destroy(false);
    }

    //restore immutability
    d->immutability = oldImm;

    config.sync();
}

void CoronaBase::requestConfigSync()
{
    // constant controlling how long between requesting a configuration sync
    // and one happening should occur. currently 10 seconds
    static const int CONFIG_SYNC_TIMEOUT = 10000;

    // TODO: should we check into our immutability before doing this?

    //NOTE: this is a pretty simplistic model: we simply save no more than CONFIG_SYNC_TIMEOUT
    //      after the first time this is called. not much of a heuristic for save points, but
    //      it should at least compress these activities a bit and provide a way for applet
    //      authors to ween themselves from the sync() disease. A more interesting/dynamic
    //      algorithm for determining when to actually sync() to disk might be better, though.
    if (!d->configSyncTimer.isActive()) {
        d->configSyncTimer.start(CONFIG_SYNC_TIMEOUT);
    }
}

void CoronaBase::requireConfigSync()
{
    d->syncConfig();
}

void CoronaBase::initializeLayout(const QString &configName)
{
    clearContainments();
    loadLayout(configName);

    if (d->containments.isEmpty()) {
        loadDefaultLayout();
        if (!d->containments.isEmpty()) {
            requestConfigSync();
        }
    }

    if (config()->isImmutable()) {
        setImmutability(SystemImmutable);
    } else {
        KConfigGroup coronaConfig(config(), "General");
        setImmutability((ImmutabilityType)coronaConfig.readEntry("immutability", (int)Mutable));
    }
}

void CoronaBase::loadLayout(const QString &configName)
{
    if (!configName.isEmpty() && configName != d->configName) {
        // if we have a new config name passed in, then use that as the config file for this Corona
        d->config = 0;
        d->configName = configName;
    }

    KConfigGroup conf(config(), QString());
    d->importLayout(conf, false);
}

QList<Plasma::Containment *> CoronaBase::importLayout(const KConfigGroup &conf)
{
    return d->importLayout(conf, true);
}

Containment *CoronaBase::containmentForScreen(int screen, int desktop) const
{
    foreach (Containment *containment, d->containments) {
        if (containment->screen() == screen &&
            (desktop < 0 || containment->desktop() == desktop) &&
            (containment->containmentType() == Containment::DesktopContainment ||
             containment->containmentType() == Containment::CustomContainment)) {
            return containment;
        }
    }

    return 0;
}

Containment *CoronaBase::containmentForScreen(int screen, int desktop,
                                          const QString &defaultPluginIfNonExistent, const QVariantList &defaultArgs)
{
    Containment *containment = containmentForScreen(screen, desktop);
    if (!containment && !defaultPluginIfNonExistent.isEmpty()) {
        // screen requests are allowed to bypass immutability
        if (screen >= 0 && screen < numScreens() &&
            desktop >= -1 && desktop < KWindowSystem::numberOfDesktops()) {
            containment = d->addContainment(defaultPluginIfNonExistent, defaultArgs, 0, false);
            if (containment) {
                containment->setScreen(screen, desktop);
            }
        }
    }

    return containment;
}

QList<Containment*> CoronaBase::containments() const
{
    return d->containments;
}

void CoronaBase::clearContainments()
{
    foreach (Containment *containment, d->containments) {
        containment->clearApplets();
    }
}

KSharedConfigPtr CoronaBase::config() const
{
    if (!d->config) {
        d->config = KSharedConfig::openConfig(d->configName, KConfig::SimpleConfig);
    }

    return d->config;
}

Containment *CoronaBase::addContainment(const QString &name, const QVariantList &args)
{
    if (d->immutability == Mutable) {
        return d->addContainment(name, args, 0, false);
    }

    return 0;
}

Containment *CoronaBase::addContainmentDelayed(const QString &name, const QVariantList &args)
{
    if (d->immutability == Mutable) {
        return d->addContainment(name, args, 0, true);
    }

    return 0;
}

int CoronaBase::numScreens() const
{
    return 1;
}

QRect CoronaBase::screenGeometry(int id) const
{
    //This is unreliable, give better implementations in subclasses
    return qApp->desktop()->screenGeometry(id);
}

QRegion CoronaBase::availableScreenRegion(int id) const
{
    return QRegion(screenGeometry(id));
}

void CoronaBase::loadDefaultLayout()
{
}

void CoronaBase::setPreferredToolBoxPlugin(const Containment::Type type, const QString &plugin)
{
    d->toolBoxPlugins[type] = plugin;
    //TODO: react to plugin changes on the fly? still don't see the use case (maybe for laptops that become tablets?)
}

QString CoronaBase::preferredToolBoxPlugin(const Containment::Type type) const
{
    return d->toolBoxPlugins.value(type);
}

ImmutabilityType CoronaBase::immutability() const
{
    return d->immutability;
}

void CoronaBase::setImmutability(const ImmutabilityType immutable)
{
    if (d->immutability == immutable || d->immutability == SystemImmutable) {
        return;
    }

#ifndef NDEBUG
    kDebug() << "setting immutability to" << immutable;
#endif
    d->immutability = immutable;
    d->updateContainmentImmutability();
    //tell non-containments that might care (like plasmaapp or a custom corona)
    emit immutabilityChanged(immutable);

    //update our actions
    QAction *action = d->actions.action("lock widgets");
    if (action) {
        if (d->immutability == SystemImmutable) {
            action->setEnabled(false);
            action->setVisible(false);
        } else {
            bool unlocked = d->immutability == Mutable;
            action->setText(unlocked ? i18n("Lock Widgets") : i18n("Unlock Widgets"));
            action->setIcon(KDE::icon(unlocked ? "object-locked" : "object-unlocked"));
            action->setEnabled(true);
            action->setVisible(true);
        }
    }

    if (d->immutability != SystemImmutable) {
        KConfigGroup cg(config(), "General");

        // we call the dptr member directly for locked since isImmutable()
        // also checks kiosk and parent containers
        cg.writeEntry("immutability", (int)d->immutability);
        requestConfigSync();
    }
}

QList<Plasma::Location> CoronaBase::freeEdges(int screen) const
{
    QList<Plasma::Location> freeEdges;
    freeEdges << Plasma::TopEdge << Plasma::BottomEdge
              << Plasma::LeftEdge << Plasma::RightEdge;

    foreach (Containment *containment, containments()) {
        if (containment->screen() == screen &&
            freeEdges.contains(containment->location())) {
            freeEdges.removeAll(containment->location());
        }
    }

    return freeEdges;
}

QAction *CoronaBase::action(QString name) const
{
    return d->actions.action(name);
}

void CoronaBase::addAction(QString name, QAction *action)
{
    d->actions.addAction(name, action);
}

KAction* CoronaBase::addAction(QString name)
{
    return d->actions.addAction(name);
}

QList<QAction*> CoronaBase::actions() const
{
    return d->actions.actions();
}

void CoronaBase::enableAction(const QString &name, bool enable)
{
    QAction *action = d->actions.action(name);
    if (action) {
        action->setEnabled(enable);
        action->setVisible(enable);
    }
}

void CoronaBase::updateShortcuts()
{
    QMutableListIterator<QWeakPointer<KActionCollection> > it(d->actionCollections);
    while (it.hasNext()) {
        it.next();
        KActionCollection *collection = it.value().data();
        if (!collection) {
            // get rid of KActionCollections that have been deleted behind our backs
            it.remove();
            continue;
        }

        collection->readSettings();
        if (d->shortcutsDlg) {
            d->shortcutsDlg.data()->addCollection(collection);
        }
    }
}

void CoronaBase::addShortcuts(KActionCollection *newShortcuts)
{
    d->actionCollections << newShortcuts;
    if (d->shortcutsDlg) {
        d->shortcutsDlg.data()->addCollection(newShortcuts);
    }
}

void CoronaBase::setContainmentActionsDefaults(Containment::Type containmentType, const ContainmentActionsPluginsConfig &config)
{
    d->containmentActionsDefaults.insert(containmentType, config);
}

ContainmentActionsPluginsConfig CoronaBase::containmentActionsDefaults(Containment::Type containmentType)
{
    return d->containmentActionsDefaults.value(containmentType);
}

void CoronaBase::setDialogManager(AbstractDialogManager *dialogManager)
{
    d->dialogManager = dialogManager;
}

AbstractDialogManager *CoronaBase::dialogManager()
{
    return d->dialogManager.data();
}

CoronaBasePrivate::CoronaBasePrivate(CoronaBase *corona)
    : q(corona),
      immutability(Mutable),
      mimetype("text/x-plasmoidservicename"),
      defaultContainmentPlugin("desktop"),
      config(0),
      actions(corona)
{
    if (QCoreApplication::instance()) {
        configName = QCoreApplication::instance()->applicationName() + "-appletsrc";
    } else {
        configName = "plasma-appletsrc";
    }
}

CoronaBasePrivate::~CoronaBasePrivate()
{
    qDeleteAll(containments);
}

void CoronaBasePrivate::init()
{
    configSyncTimer.setSingleShot(true);
    QObject::connect(&configSyncTimer, SIGNAL(timeout()), q, SLOT(syncConfig()));

    //some common actions
    actions.setConfigGroup("Shortcuts");

    KAction *lockAction = actions.addAction("lock widgets");
    QObject::connect(lockAction, SIGNAL(triggered(bool)), q, SLOT(toggleImmutability()));
    lockAction->setText(i18n("Lock Widgets"));
    lockAction->setAutoRepeat(true);
    lockAction->setIcon(KDE::icon("object-locked"));
    lockAction->setData(AbstractToolBox::ControlTool);
    lockAction->setShortcut(KShortcut("alt+d, l"));
    lockAction->setShortcutContext(Qt::ApplicationShortcut);

    //FIXME this doesn't really belong here. desktop KCM maybe?
    //but should the shortcuts be per-app or really-global?
    //I don't know how to make kactioncollections use plasmarc
    KAction *action = actions.addAction("configure shortcuts");
    QObject::connect(action, SIGNAL(triggered()), q, SLOT(showShortcutConfig()));
    action->setText(i18n("Shortcut Settings"));
    action->setIcon(KDE::icon("configure-shortcuts"));
    action->setAutoRepeat(false);
    action->setData(AbstractToolBox::ConfigureTool);
    //action->setShortcut(KShortcut("ctrl+h"));
    action->setShortcutContext(Qt::ApplicationShortcut);

    //fake containment/applet actions
    KActionCollection *containmentActions = AppletPrivate::defaultActions(q); //containment has to start with applet stuff
    ContainmentPrivate::addDefaultActions(containmentActions); //now it's really containment
    actionCollections << &actions << AppletPrivate::defaultActions(q) << containmentActions;
    q->updateShortcuts();
}

void CoronaBasePrivate::showShortcutConfig()
{
    //show a kshortcutsdialog with the actions
    KShortcutsDialog *dlg = shortcutsDlg.data();
    if (!dlg) {
        dlg = new KShortcutsDialog();
        dlg->setModal(false);
        dlg->setAttribute(Qt::WA_DeleteOnClose, true);
        QObject::connect(dlg, SIGNAL(saved()), q, SIGNAL(shortcutsChanged()));

        dlg->addCollection(&actions);
        QMutableListIterator<QWeakPointer<KActionCollection> > it(actionCollections);
        while (it.hasNext()) {
            it.next();
            KActionCollection *collection = it.value().data();
            if (!collection) {
                // get rid of KActionCollections that have been deleted behind our backs
                it.remove();
                continue;
            }

            dlg->addCollection(collection);
        }
    }

    KWindowSystem::setOnDesktop(dlg->winId(), KWindowSystem::currentDesktop());
    dlg->configure();
    dlg->raise();
}

void CoronaBasePrivate::toggleImmutability()
{
    if (immutability == Mutable) {
        q->setImmutability(UserImmutable);
    } else {
        q->setImmutability(Mutable);
    }
}

void CoronaBasePrivate::saveLayout(KSharedConfigPtr cg) const
{
    KConfigGroup containmentsGroup(cg, "Containments");
    foreach (const Containment *containment, containments) {
        QString cid = QString::number(containment->id());
        KConfigGroup containmentConfig(&containmentsGroup, cid);
        containment->save(containmentConfig);
    }
}

void CoronaBasePrivate::updateContainmentImmutability()
{
    foreach (Containment *c, containments) {
        // we need to tell each containment that immutability has been altered
        c->updateConstraints(ImmutableConstraint);
    }
}

void CoronaBasePrivate::containmentDestroyed(QObject *obj)
{
        // we do a static_cast here since it really isn't an Containment by this
        // point anymore since we are in the qobject dtor. we don't actually
        // try and do anything with it, we just need the value of the pointer
        // so this unsafe looking code is actually just fine.
        Containment* containment = static_cast<Plasma::Containment*>(obj);
        int index = containments.indexOf(containment);

        if (index > -1) {
            containments.removeAt(index);
            q->requestConfigSync();
        }
    }

void CoronaBasePrivate::syncConfig()
{
    q->config()->sync();
    emit q->configSynced();
}

Containment *CoronaBasePrivate::addContainment(const QString &name, const QVariantList &args, uint id, bool delayedInit)
{
    QString pluginName = name;
    Containment *containment = 0;
    Applet *applet = 0;

    //kDebug() << "Loading" << name << args << id;

    if (pluginName.isEmpty() || pluginName == "default") {
        // default to the desktop containment
        pluginName = defaultContainmentPlugin;
    }

    bool loadingNull = pluginName == "null";
    if (!loadingNull) {
        applet = PluginLoader::self()->loadApplet(pluginName, id, args);
        containment = dynamic_cast<Containment*>(applet);
    }

    if (!containment) {
        if (!loadingNull) {
#ifndef NDEBUG
            kDebug() << "loading of containment" << name << "failed.";
#endif
        }

        // in case we got a non-Containment from Applet::loadApplet or
        // a null containment was requested
        if (applet) {
            // the applet probably doesn't know what's hit it, so let's pretend it can be
            // initialized to make assumptions in the applet's dtor safer
            applet->init();
            delete applet;
        }
        applet = containment = new Containment(0, 0, id);

        if (loadingNull) {
            containment->setDrawWallpaper(false);
        } else {
            containment->setFailedToLaunch(false);
        }

        // we want to provide something and don't care about the failure to launch
        containment->setFormFactor(Plasma::Planar);
    }

    // if this is a new containment, we need to ensure that there are no stale
    // configuration data around
    if (id == 0) {
        KConfigGroup conf(q->config(), "Containments");
        conf = KConfigGroup(&conf, QString::number(containment->id()));
        conf.deleteGroup();
    }

    applet->d->isContainment = true;
    applet->d->setIsContainment(true, true);
    containments.append(containment);

    if (!delayedInit) {
        containment->init();
        KConfigGroup cg = containment->config();
        containment->restore(cg);
        containment->updateConstraints(Plasma::StartupCompletedConstraint);
        containment->save(cg);
        q->requestConfigSync();
        containment->flushPendingConstraintsEvents();
    }

    QObject::connect(containment, SIGNAL(destroyed(QObject*)),
            q, SLOT(containmentDestroyed(QObject*)));
    QObject::connect(containment, SIGNAL(configNeedsSaving()),
            q, SLOT(requestConfigSync()));
    QObject::connect(containment, SIGNAL(releaseVisualFocus()),
            q, SIGNAL(releaseVisualFocus()));
    QObject::connect(containment, SIGNAL(screenChanged(int,int,Plasma::Containment*)),
            q, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)));

    if (!delayedInit) {
        emit q->containmentAdded(containment);
    }

    return containment;
}

QList<Plasma::Containment *> CoronaBasePrivate::importLayout(const KConfigGroup &conf, bool mergeConfig)
{
    if (!conf.isValid()) {
        return QList<Containment *>();
    }

    QList<Plasma::Containment *> newContainments;
    QSet<uint> containmentsIds;

    foreach (Containment *containment, containments) {
        containmentsIds.insert(containment->id());
    }

    KConfigGroup containmentsGroup(&conf, "Containments");

    foreach (const QString &group, containmentsGroup.groupList()) {
        KConfigGroup containmentConfig(&containmentsGroup, group);

        if (containmentConfig.entryMap().isEmpty()) {
            continue;
        }

        uint cid = group.toUInt();
        if (containmentsIds.contains(cid)) {
            cid = ++AppletPrivate::s_maxAppletId;
        } else if (cid > AppletPrivate::s_maxAppletId) {
            AppletPrivate::s_maxAppletId = cid;
        }

        if (mergeConfig) {
            KConfigGroup realConf(q->config(), "Containments");
            realConf = KConfigGroup(&realConf, QString::number(cid));
            // in case something was there before us
            realConf.deleteGroup();
            containmentConfig.copyTo(&realConf);
        }

        //kDebug() << "got a containment in the config, trying to make a" << containmentConfig.readEntry("plugin", QString()) << "from" << group;
#ifndef NDEBUG
        kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Adding Containment" << containmentConfig.readEntry("plugin", QString());
#endif
        Containment *c = addContainment(containmentConfig.readEntry("plugin", QString()), QVariantList(), cid, true);
        if (!c) {
            continue;
        }

        newContainments.append(c);
        containmentsIds.insert(c->id());

        c->init();
#ifndef NDEBUG
        kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Init Containment" << c->pluginName();
#endif
        c->restore(containmentConfig);
#ifndef NDEBUG
        kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Restored Containment" << c->pluginName();
#endif
    }

    foreach (Containment *containment, newContainments) {
        containment->updateConstraints(Plasma::StartupCompletedConstraint);
        containment->d->initApplets();
        emit q->containmentAdded(containment);
#ifndef NDEBUG
        kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Containment" << containment->name();
#endif
    }

    return newContainments;
}

} // namespace Plasma



#include "moc_coronabase.cpp"
