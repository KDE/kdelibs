/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kglobalaccel.h"
#include "kglobalaccel_p.h"

#include <memory>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMetaType>
#ifdef Q_WS_X11
#include <QtGui/QX11Info>
#include <netwm_def.h>
#endif

#include <kdebug.h>
#include <ktoolinvocation.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include "kaction.h"
#include "kaction_p.h"
#include "kmessagebox.h"
#include "kshortcut.h"

org::kde::kglobalaccel::Component *KGlobalAccelPrivate::getComponent(const QString &componentUnique, bool remember = false)
{
    // Check if we already have this component
    if (components.contains(componentUnique)) {
        return components[componentUnique];
    }

    // Connect to the kglobalaccel daemon
    org::kde::KGlobalAccel kglobalaccel(
            "org.kde.kglobalaccel",
            "/kglobalaccel",
            QDBusConnection::sessionBus());
    if (!kglobalaccel.isValid()) {
        kDebug() << "Failed to connect to the kglobalaccel daemon" << QDBusConnection::sessionBus().lastError();
        return NULL;
    }

    // Get the path for our component. We have to do that because
    // componentUnique is probably not a valid dbus object path
    QDBusReply<QDBusObjectPath> reply = kglobalaccel.getComponent(componentUnique);
    if (!reply.isValid()) {

        if (reply.error().name() == "org.kde.kglobalaccel.NoSuchComponent") {
            // No problem. The component doesn't exists. That's normal
            return NULL;
        }

        // An unknown error.
        kDebug() << "Failed to get dbus path for component " << componentUnique << reply.error();
        return NULL;
    }

    // Now get the component
    org::kde::kglobalaccel::Component *component = new org::kde::kglobalaccel::Component(
        "org.kde.kglobalaccel",
        reply.value().path(),
        QDBusConnection::sessionBus(),
        q);

    // No component no cleaning
    if (!component->isValid()) {
        kDebug() << "Failed to get component" << componentUnique << QDBusConnection::sessionBus().lastError();
        return NULL;
    }

    if (remember)
        {
        // Connect to the signals we are interested in.
        q->connect(component, SIGNAL(globalShortcutPressed(const QString &, const QString &, qlonglong)),
                SLOT(_k_invokeAction(const QString &, const QString &, qlonglong)));

        components[componentUnique] = component;
        }

    return component;
}



KGlobalAccelPrivate::KGlobalAccelPrivate(KGlobalAccel *q)
     :  isUsingForeignComponentName(false),
        enabled(true),
        iface("org.kde.kglobalaccel", "/kglobalaccel", QDBusConnection::sessionBus()),
        q(q)
{
    // Make sure kded is running. The iface declaration above somehow
    // works anyway.
    QDBusConnectionInterface* bus = QDBusConnection::sessionBus().interface();
    if (!bus->isServiceRegistered("org.kde.kglobalaccel")) {
        QString error;
        int ret = KToolInvocation::startServiceByDesktopPath(
                "kglobalaccel.desktop",
                QStringList(),
                &error);

        if (ret > 0) {
            kError() << "Couldn't start kglobalaccel from kglobalaccel.desktop: " << error << endl;
        }
    }
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(iface.service(),
                                                           QDBusConnection::sessionBus(),
                                                           QDBusServiceWatcher::WatchForOwnerChange,
                                                           q);
    q->connect(watcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                     q, SLOT(_k_serviceOwnerChanged(QString,QString,QString)));
}


void KGlobalAccelPrivate::readComponentData(const KComponentData &componentData)
{
    Q_ASSERT(!componentData.componentName().isEmpty());

    mainComponent = componentData;
    if (componentData.aboutData()->programName().isEmpty()) {
        kDebug(123) << componentData.componentName() << " has empty programName()";
    }
}


KGlobalAccel::KGlobalAccel()
    : d(new KGlobalAccelPrivate(this))
{
    qDBusRegisterMetaType<QList<int> >();
    qDBusRegisterMetaType<QList<QStringList> >();
    qDBusRegisterMetaType<KGlobalShortcutInfo>();
    qDBusRegisterMetaType<QList<KGlobalShortcutInfo> >();

    connect(&d->iface, SIGNAL(yourShortcutGotChanged(const QStringList &, const QList<int> &)),
            SLOT(_k_shortcutGotChanged(const QStringList &, const QList<int> &)));

    if (KGlobal::hasMainComponent()) {
        d->readComponentData( KGlobal::mainComponent() );
    }

}


KGlobalAccel::~KGlobalAccel()
{
    delete d;
}


void KGlobalAccel::activateGlobalShortcutContext(
        const QString &contextUnique,
        const QString &contextFriendly,
        const KComponentData &component)
{
    Q_UNUSED(contextFriendly);
    // TODO: provide contextFriendly
    self()->d->iface.activateGlobalShortcutContext(component.aboutData()->programName(), contextUnique);
}


// static
bool KGlobalAccel::cleanComponent(const QString &componentUnique)
{
    org::kde::kglobalaccel::Component* component = self()->getComponent(componentUnique);
    if (!component) return false;

    return component->cleanUp();
}


// static
bool KGlobalAccel::isComponentActive(const QString &componentUnique)
{
    org::kde::kglobalaccel::Component* component = self()->getComponent(componentUnique);
    if (!component) return false;

    return component->isActive();
}


bool KGlobalAccel::isEnabled() const
{
    return d->enabled;
}


org::kde::kglobalaccel::Component *KGlobalAccel::getComponent(const QString &componentUnique)
{
    return d->getComponent(componentUnique);
}


void KGlobalAccel::setEnabled(bool enabled)
{
    d->enabled = enabled;
}


void KGlobalAccel::overrideMainComponentData(const KComponentData &kcd)
{
    d->readComponentData(kcd);
    d->isUsingForeignComponentName = true;
}


KGlobalAccel *KGlobalAccel::self()
{
    K_GLOBAL_STATIC(KGlobalAccel, s_instance)
    return s_instance;
}


void KGlobalAccelPrivate::doRegister(KAction *action)
{
    if (!action || action->objectName().isEmpty()) {
        return;
    }

    const bool isRegistered = actions.contains(action);
    if (isRegistered)
        return;

    // Under configuration mode - deprecated - we ignore the component given
    // from the action and use our own.
    if (isUsingForeignComponentName) {
        action->d->componentData = mainComponent;
    }
    QStringList actionId = makeActionId(action);

    nameToAction.insertMulti(actionId.at(KGlobalAccel::ActionUnique), action);
    actions.insert(action);
    iface.doRegister(actionId);
}


void KGlobalAccelPrivate::remove(KAction *action, Removal removal)
{
    if (!action  || action->objectName().isEmpty()) {
        return;
    }

    const bool isRegistered = actions.contains(action);
    if (!isRegistered) {
        return;
    }

    QStringList actionId = makeActionId(action);

    nameToAction.remove(actionId.at(KGlobalAccel::ActionUnique), action);
    actions.remove(action);

    if (removal == UnRegister) {
        // Complete removal of the shortcut is requested
        // (forgetGlobalShortcut)
        iface.unRegister(actionId);
    } else {
        // If the action is a configurationAction wen only remove it from our
        // internal registry. That happened above.
        if (!action->property("isConfigurationAction").toBool()) {
            // If it's a session shortcut unregister it.
            action->objectName().startsWith(QLatin1String("_k_session:"))
                ? iface.unRegister(actionId)
                : iface.setInactive(actionId);
        }
    }
}


void KGlobalAccelPrivate::updateGlobalShortcut(KAction *action, uint flags)
{
    // No action or no objectname -> Do nothing
    // KAction::setGlobalShortcut informs the user
    if (!action || action->objectName().isEmpty()) {
        return;
    }

    QStringList actionId = makeActionId(action);
    const KShortcut activeShortcut = action->globalShortcut();
    const KShortcut defaultShortcut = action->globalShortcut(KAction::DefaultShortcut);

    uint setterFlags = 0;
    if (flags & KAction::NoAutoloading) {
        setterFlags |= NoAutoloading;
    }

    if (flags & KAction::ActiveShortcut) {
        bool isConfigurationAction = isUsingForeignComponentName
            || action->property("isConfigurationAction").toBool();
        uint activeSetterFlags = setterFlags;

        // setPresent tells kglobalaccel that the shortcut is active
        if (!isConfigurationAction) {
            activeSetterFlags |= SetPresent;
        }

        // Sets the shortcut, returns the active/real keys
        const QList<int> result = iface.setShortcut(
                actionId,
                intListFromShortcut(activeShortcut),
                activeSetterFlags);

        // Make sure we get informed about changes in the component by kglobalaccel
        getComponent(componentUniqueForAction(action), true);

        // Create a shortcut from the result
        const KShortcut scResult(shortcutFromIntList(result));

        if (isConfigurationAction && (flags & KAction::NoAutoloading)) {
            // If this is a configuration action and we have set the shortcut,
            // inform the real owner of the change.
            // Note that setForeignShortcut will cause a signal to be sent to applications
            // even if it did not "see" that the shortcut has changed. This is Good because
            // at the time of comparison (now) the action *already has* the new shortcut.
            // We called setShortcut(), remember?
            // Also note that we will see our own signal so we may not need to call
            // setActiveGlobalShortcutNoEnable - _k_shortcutGotChanged() does it.
            // In practice it's probably better to get the change propagated here without
            // DBus delay as we do below.
            iface.setForeignShortcut(actionId, result);
        }
        if (scResult != activeShortcut) {
            // If kglobalaccel returned a shortcut that differs from the one we
            // sent, use that one. There must have been clashes or some other problem.
            action->d->setActiveGlobalShortcutNoEnable(scResult);
        }
    }

    if (flags & KAction::DefaultShortcut) {
        iface.setShortcut(actionId, intListFromShortcut(defaultShortcut),
                          setterFlags | IsDefault);
    }
}


QStringList KGlobalAccelPrivate::makeActionId(const KAction *action)
{
    QStringList ret(componentUniqueForAction(action));  // Component Unique Id ( see actionIdFields )
    Q_ASSERT(!ret.at(KGlobalAccel::ComponentUnique).isEmpty());
    Q_ASSERT(!action->objectName().isEmpty());
    ret.append(action->objectName());                   // Action Unique Name
    ret.append(componentFriendlyForAction(action));     // Component Friendly name
    ret.append(action->text());                         // Action Friendly Name
    return ret;
}


QList<int> KGlobalAccelPrivate::intListFromShortcut(const KShortcut &cut)
{
    QList<int> ret;
    ret.append(cut.primary()[0]);
    ret.append(cut.alternate()[0]);
    while (!ret.isEmpty() && ret.last() == 0)
        ret.removeLast();
    return ret;
}


KShortcut KGlobalAccelPrivate::shortcutFromIntList(const QList<int> &list)
{
    KShortcut ret;
    if (list.count() > 0)
        ret.setPrimary(list[0]);
    if (list.count() > 1)
        ret.setAlternate(list[1]);
    return ret;
}


QString KGlobalAccelPrivate::componentUniqueForAction(const KAction *action)
{
    Q_ASSERT(action->d->componentData.isValid());
    return action->d->componentData.componentName();
}


QString KGlobalAccelPrivate::componentFriendlyForAction(const KAction *action)
{
    Q_ASSERT(action->d->componentData.isValid());
    return action->d->componentData.aboutData()->programName();
}


void KGlobalAccelPrivate::_k_invokeAction(
        const QString &componentUnique,
        const QString &actionUnique,
        qlonglong timestamp)
{
    // If overrideMainComponentData() is active the app can only have
    // configuration actions.
    if (isUsingForeignComponentName ) {
        return;
    }

    KAction *action = 0;
    QList<KAction *> candidates = nameToAction.values(actionUnique);
    foreach (KAction *const a, candidates) {
        if (componentUniqueForAction(a) == componentUnique) {
            action = a;
        }
    }

    // We do not trigger if
    // - there is no action
    // - the action is not enabled
    // - the action is an configuration action
    if (!action || !action->isEnabled() || action->property("isConfigurationAction").toBool()) {
        return;
    }

#ifdef Q_WS_X11
    // Update this application's X timestamp if needed.
    // TODO The 100%-correct solution should probably be handling this action
    // in the proper place in relation to the X events queue in order to avoid
    // the possibility of wrong ordering of user events.
    if( NET::timestampCompare(timestamp, QX11Info::appTime()) > 0)
        QX11Info::setAppTime(timestamp);
    if( NET::timestampCompare(timestamp, QX11Info::appUserTime()) > 0)
        QX11Info::setAppUserTime(timestamp);
#else
    Q_UNUSED(timestamp);
#endif

    action->trigger();
}


void KGlobalAccelPrivate::_k_shortcutGotChanged(const QStringList &actionId,
                                                const QList<int> &keys)
{
    KAction *action = nameToAction.value(actionId.at(KGlobalAccel::ActionUnique));
    if (!action)
        return;

    action->d->setActiveGlobalShortcutNoEnable(shortcutFromIntList(keys));
}

void KGlobalAccelPrivate::_k_serviceOwnerChanged(const QString &name, const QString &oldOwner,
                                                 const QString &newOwner)
{
    Q_UNUSED(oldOwner);
    if (name == QLatin1String("org.kde.kglobalaccel") && !newOwner.isEmpty()) {
        // kglobalaccel was restarted
        kDebug(123) << "detected kglobalaccel restarting, re-registering all shortcut keys";
        reRegisterAll();
    }
}

void KGlobalAccelPrivate::reRegisterAll()
{
    //### Special case for isUsingForeignComponentName?

    //We clear all our data, assume that all data on the other side is clear too,
    //and register each action as if it just was allowed to have global shortcuts.
    //If the kded side still has the data it doesn't matter because of the
    //autoloading mechanism. The worst case I can imagine is that an action's
    //shortcut was changed but the kded side died before it got the message so
    //autoloading will now assign an old shortcut to the action. Particularly
    //picky apps might assert or misbehave.
    QSet<KAction *> allActions = actions;
    nameToAction.clear();
    actions.clear();
    foreach(KAction *const action, allActions) {
        doRegister(action);
        updateGlobalShortcut(action, KAction::Autoloading | KAction::ActiveShortcut);
    }
}


QList<QStringList> KGlobalAccel::allMainComponents()
{
    return d->iface.allMainComponents();
}


QList<QStringList> KGlobalAccel::allActionsForComponent(const QStringList &actionId)
{
    return d->iface.allActionsForComponent(actionId);
}


//static
QStringList KGlobalAccel::findActionNameSystemwide(const QKeySequence &seq)
{
    return self()->d->iface.action(seq[0]);
}


QList<KGlobalShortcutInfo> KGlobalAccel::getGlobalShortcutsByKey(const QKeySequence &seq)
{
    return self()->d->iface.getGlobalShortcutsByKey(seq[0]);
}


bool KGlobalAccel::isGlobalShortcutAvailable(const QKeySequence &seq, const QString &comp)
{
        return self()->d->iface.isGlobalShortcutAvailable(seq[0], comp);
}


//static
bool KGlobalAccel::promptStealShortcutSystemwide(QWidget *parent, const QStringList &actionIdentifier,
                                                 const QKeySequence &seq)
{
    if (actionIdentifier.size() < 4) {
        return false;
    }
    QString title = i18n("Conflict with Global Shortcut");
    QString message = i18n("The '%1' key combination has already been allocated "
                           "to the global action \"%2\" in %3.\n"
                           "Do you want to reassign it from that action to the current one?",
                           seq.toString(), actionIdentifier.at(KGlobalAccel::ActionFriendly),
                           actionIdentifier.at(KGlobalAccel::ComponentFriendly));

    return KMessageBox::warningContinueCancel(parent, message, title, KGuiItem(i18n("Reassign")))
           == KMessageBox::Continue;
}


//static
bool KGlobalAccel::promptStealShortcutSystemwide(
        QWidget *parent,
        const QList<KGlobalShortcutInfo> &shortcuts,
        const QKeySequence &seq)
{
    if (shortcuts.isEmpty()) {
        // Usage error. Just say no
        return false;
    }

    QString component = shortcuts[0].componentFriendlyName();

    QString message;
    if (shortcuts.size()==1) {
        message = i18n("The '%1' key combination is registered by application %2 for action %3:",
                seq.toString(),
                component,
                shortcuts[0].friendlyName());
    } else {
        QString actionList;
        Q_FOREACH(const KGlobalShortcutInfo &info, shortcuts) {
            actionList += i18n("In context '%1' for action '%2'\n",
                    info.contextFriendlyName(),
                    info.friendlyName());
        }
        message = i18n("The '%1' key combination is registered by application %2.\n%3",
                           seq.toString(),
                           component,
                           actionList);
    }

    QString title = i18n("Conflict With Registered Global Shortcut");

    return KMessageBox::warningContinueCancel(parent, message, title, KGuiItem(i18n("Reassign")))
           == KMessageBox::Continue;
}


//static
void KGlobalAccel::stealShortcutSystemwide(const QKeySequence &seq)
{
    //get the shortcut, remove seq, and set the new shortcut
    const QStringList actionId = self()->d->iface.action(seq[0]);
    if (actionId.size() < 4) // not a global shortcut
        return;
    QList<int> sc = self()->d->iface.shortcut(actionId);

    for (int i = 0; i < sc.count(); i++)
        if (sc[i] == seq[0])
            sc[i] = 0;

    self()->d->iface.setForeignShortcut(actionId, sc);
}

#include "kglobalaccel.moc"

