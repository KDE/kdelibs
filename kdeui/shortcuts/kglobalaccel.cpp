/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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

#if 0
// For KGlobalAccelImpl
#ifdef Q_WS_X11
#include "kglobalaccel_x11.h"
#elif defined(Q_WS_MACX)
#include "kglobalaccel_mac.h"
#elif defined(Q_WS_WIN)
#include "kglobalaccel_win.h"
#elif defined(Q_WS_QWS)
#include "kglobalaccel_qws.h"
#else
#include "kglobalaccel_emb.h"
#endif
#endif

#include <QtCore/QCoreApplication>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMetaType>
#ifdef Q_WS_X11
#include <QtGui/QX11Info>
#include <netwm_def.h>
#include <X11/X.h>
#include <fixx11h.h>
#endif

#include <kdebug.h>
#include <klocale.h>
#include <ktoolinvocation.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include "kaction.h"
#include "kaction_p.h"
#include "kmessagebox.h"
#include "kshortcut.h"


//### copied over from kdedglobalaccel.cpp to avoid more includes
enum actionIdFields
{
    ComponentUnique = 0,
    ActionUnique = 1,
    ComponentFriendly = 2,
    ActionFriendly = 3
};


KGlobalAccelPrivate::KGlobalAccelPrivate(KGlobalAccel *q)
     : isUsingForeignComponentName(false),
       enabled(true),
       iface("org.kde.kded", "/modules/kdedglobalaccel", QDBusConnection::sessionBus())
{
    kDebug() << iface.isValid();

    // Make sure kded is running
    QDBusConnectionInterface* bus = QDBusConnection::sessionBus().interface();
    if (!bus->isServiceRegistered("org.kde.kded")) {
        KToolInvocation::klauncher(); // this calls startKdeinit
    }
    QObject::connect(bus, SIGNAL(serviceOwnerChanged(QString, QString, QString)),
                     q, SLOT(_k_serviceOwnerChanged(QString, QString, QString)));

    kDebug() << iface.isValid();
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

    kDebug() << d->iface.isValid();

    connect(&d->iface, SIGNAL(invokeAction(const QStringList &, qlonglong)),
            SLOT(_k_invokeAction(const QStringList &, qlonglong)));
    connect(&d->iface, SIGNAL(yourShortcutGotChanged(const QStringList &, const QList<int> &)),
            SLOT(_k_shortcutGotChanged(const QStringList &, const QList<int> &)));

    if (KGlobal::hasMainComponent()) {
        d->readComponentData( KGlobal::mainComponent() );
    }
    kDebug() << d->iface.isValid();

}


KGlobalAccel::~KGlobalAccel()
{
    //TODO *maybe* we need to ungrab/unregister all
    delete d;
}


bool KGlobalAccel::isEnabled() const
{
    return d->enabled;
}


void KGlobalAccel::setEnabled(bool enabled)
{
    d->enabled = enabled;

//TODO: implement this in KdedGlobalAccelInterface... or not at all
#if 0
    if (enabled) {
        foreach (KAction* action, d->actionsWithGlobalShortcuts)
            checkAction(action);

    } else {
        foreach (int key, d->grabbedKeys.keys())
            d->impl->grabKey(key, false);
        d->grabbedActions.clear();
        d->grabbedKeys.clear();
    }
#endif
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

    nameToAction.insertMulti(actionId.at(ActionUnique), action);
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

    nameToAction.remove(actionId.at(ActionUnique), action);
    actions.remove(action);

    if (removal == UnRegister) {
        // Complete removal of the shortcut is requested
        // (forgetGlobalShortcut)
        iface.unRegister(actionId);
    } else {
        // If the action is a configurationAction wen only remove it from our
        // internal registry. That happened above.
        if (!action->property("isConfigurationAction").toBool()) {
            iface.setInactive(actionId);
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
        setterFlags |= KdedGlobalAccelInterface::NoAutoloading;
    }

    if (flags & KAction::ActiveShortcut) {
        bool isConfigurationAction = isUsingForeignComponentName
            || action->property("isConfigurationAction").toBool();
        uint activeSetterFlags = setterFlags;

        // setPresent tells kdedglobalaccel that the shortcut is active
        if (!isConfigurationAction) {
            activeSetterFlags |= KdedGlobalAccelInterface::SetPresent;
        }

        // Sets the shortcut, returns the active/real keys
        const QList<int> result = iface.setShortcut(
                actionId,
                intListFromShortcut(activeShortcut),
                activeSetterFlags);
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
            // If kdedglobalaccel returned a shortcut that differs from the one we
            // sent, use that one. There must have been clashes or some other problem.
            action->d->setActiveGlobalShortcutNoEnable(scResult);
        }
    }

    if (flags & KAction::DefaultShortcut) {
        iface.setShortcut(actionId, intListFromShortcut(defaultShortcut),
                          setterFlags | KdedGlobalAccelInterface::IsDefault);
    }
}


QStringList KGlobalAccelPrivate::makeActionId(const KAction *action)
{
    QStringList ret(componentUniqueForAction(action));  // Component Unique Id ( see actionIdFields )
    Q_ASSERT(!ret.at(ComponentUnique).isEmpty());
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


void KGlobalAccelPrivate::_k_invokeAction(const QStringList &actionId, qlonglong timestamp)
{
    kDebug() << "##########################################################";

    // If overrideMainComponentData() is active the app can only have
    // configuration actions.
    if (isUsingForeignComponentName ) {
        return;
    }

    KAction *action = 0;
    QList<KAction *> candidates = nameToAction.values(actionId.at(ActionUnique));
    foreach (KAction *const a, candidates) {
        if (componentUniqueForAction(a) == actionId.at(ComponentUnique)) {
            action = a;
        }
    }

    // We do not trigger if 
    // - there is no action
    // - the action is not enabled
    // - the action is an configuration action
    if (!action || !action->isEnabled()
            || action->property("isConfigurationAction").toBool()) {
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
    KAction *action = nameToAction.value(actionId.at(ActionUnique));
    if (!action)
        return;

    action->d->setActiveGlobalShortcutNoEnable(shortcutFromIntList(keys));
}

void KGlobalAccelPrivate::_k_serviceOwnerChanged(const QString &name, const QString &oldOwner,
                                                 const QString &newOwner)
{
    Q_UNUSED(oldOwner);
    if (name == QLatin1String("org.kde.kded") && !newOwner.isEmpty()) {
        // kded was restarted (what? you mean it crashes sometimes?)
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
                           seq.toString(), actionIdentifier.at(ActionFriendly),
                           actionIdentifier.at(ComponentFriendly));

    return KMessageBox::warningContinueCancel(parent, message, title, KGuiItem(i18n("Reassign")))
           == KMessageBox::Continue;
}


//static
void KGlobalAccel::stealShortcutSystemwide(const QKeySequence &seq)
{
    //get the shortcut, remove seq, and set the new shorctut
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
#include "kdedglobalaccel_interface.moc"

