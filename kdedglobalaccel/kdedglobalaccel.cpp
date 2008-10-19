/*
    This file is part of the KDE libraries

    Copyright (c) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    Copyright (c) 2007 Michael Jansen <kde@michael-jansen.biz>

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

#include "component.h"
#include "globalshortcut.h"
#include "globalshortcutcontext.h"
#include "globalshortcutsregistry.h"

#include <QtCore/QTimer>
#include <QtCore/QMetaMethod>
#include <QtDBus/QDBusMetaType>


#include "kdedglobalaccel.h"
#include "kglobalaccel.h"
#include "kglobalshortcutinfo_p.h"
#include "kdebug.h"

#include "kpluginfactory.h"
#include "kpluginloader.h"

K_PLUGIN_FACTORY(KdedGlobalAccelFactory,
                 registerPlugin<KdedGlobalAccel>();
    )
K_EXPORT_PLUGIN(KdedGlobalAccelFactory("globalaccel"))

struct KdedGlobalAccelPrivate
{
    GlobalShortcut *findAction(const QStringList &actionId) const;
    GlobalShortcut *addAction(const QStringList &actionId);
    KdeDGlobalAccel::Component *component(const QStringList &actionId) const;
    QTimer writeoutTimer;

    void _k_initializeDBus(const QString &path);
};

void KdedGlobalAccelPrivate::_k_initializeDBus(const QString &path)
    {
    GlobalShortcutsRegistry::self()->setDBusPath(path);
    GlobalShortcutsRegistry::self()->loadSettings();
    }


GlobalShortcut *KdedGlobalAccelPrivate::findAction(const QStringList &actionId) const
    {
    // Check if actionId is valid
    if (actionId.size() != 4)
        {
        kDebug() << "Skipped because of invalid actionId";
        return NULL;
        }

    //! Get the component
    KdeDGlobalAccel::Component *component = GlobalShortcutsRegistry::self()->getComponent(
            actionId.at(KGlobalAccel::ComponentUnique));

    return component
        ? component->getShortcutByName(actionId.at(KGlobalAccel::ActionUnique))
        : NULL;
    }


KdeDGlobalAccel::Component *KdedGlobalAccelPrivate::component(const QStringList &actionId) const
{
    // Get the component for the action. If we have none create a new one
    KdeDGlobalAccel::Component *component = GlobalShortcutsRegistry::self()->getComponent(actionId.at(KGlobalAccel::ComponentUnique));
    if (!component)
        {
        component = new KdeDGlobalAccel::Component(
                actionId.at(KGlobalAccel::ComponentUnique),
                actionId.at(KGlobalAccel::ComponentFriendly),
                GlobalShortcutsRegistry::self());
        Q_ASSERT(component);
        }
    return component;
}


GlobalShortcut *KdedGlobalAccelPrivate::addAction(const QStringList &actionId)
{
    Q_ASSERT(actionId.size() >= 4);

    KdeDGlobalAccel::Component *component = this->component(actionId);
    Q_ASSERT(component);

    Q_ASSERT(!component->getShortcutByName(actionId.at(KGlobalAccel::ActionUnique)));

    return new GlobalShortcut(
            actionId.at(KGlobalAccel::ActionUnique),
            actionId.at(KGlobalAccel::ActionFriendly),
            component->currentContext());
}


KdedGlobalAccel::KdedGlobalAccel(QObject* parent, const QList<QVariant>&)
 : KDEDModule(parent),
   d(new KdedGlobalAccelPrivate)
{
    qDBusRegisterMetaType<QList<QStringList> >();
    qDBusRegisterMetaType<QStringList>();
    qDBusRegisterMetaType<QList<int> >();
    qDBusRegisterMetaType<QList<KGlobalShortcutInfo> >();

    qDBusRegisterMetaType<KGlobalShortcutInfo>();

    GlobalShortcutsRegistry *reg = GlobalShortcutsRegistry::self();
    Q_ASSERT(reg);

    connect(&d->writeoutTimer, SIGNAL(timeout()),
            reg, SLOT(writeSettings()));

    connect(this, SIGNAL(moduleRegistered(const QString &)),
            this, SLOT(_k_initializeDBus(const QString &)));

    d->writeoutTimer.setSingleShot(true);
    connect(this, SIGNAL(moduleDeleted(KDEDModule *)),
            reg, SLOT(writeSettings()));

    connect(reg, SIGNAL(invokeAction(const QStringList &, qlonglong)),
            this, SIGNAL(invokeAction(const QStringList &, qlonglong)));
}


KdedGlobalAccel::~KdedGlobalAccel()
{
    // Unregister all currently registered actions. Enables the module to be
    // loaded / unloaded by kded.
    GlobalShortcutsRegistry::self()->setInactive();
    delete d;
}

QList<QStringList> KdedGlobalAccel::allMainComponents() const
{
    QList<QStringList> ret;
    QStringList emptyList;
    for (int i = 0; i < 4; i++) {
        emptyList.append(QString());
    }

    foreach (const KdeDGlobalAccel::Component *component, GlobalShortcutsRegistry::self()->allMainComponents()) {
        QStringList actionId(emptyList);
        actionId[KGlobalAccel::ComponentUnique] = component->uniqueName();
        actionId[KGlobalAccel::ComponentFriendly] = component->friendlyName();
        ret.append(actionId);
    }

    return ret;
}


QList<QStringList> KdedGlobalAccel::allActionsForComponent(const QStringList &actionId) const
{
    //### Would it be advantageous to sort the actions by unique name?
    QList<QStringList> ret;

    KdeDGlobalAccel::Component *const component =
        GlobalShortcutsRegistry::self()->getComponent(actionId[KGlobalAccel::ComponentUnique]);
    if (!component) {
        return ret;
    }

    QStringList partialId(actionId[KGlobalAccel::ComponentUnique]);   //ComponentUnique
    partialId.append(QString());                                      //ActionUnique
    //Use our internal friendlyName, not the one passed in. We should have the latest data.
    partialId.append(component->friendlyName());                      //ComponentFriendly
    partialId.append(QString());                                      //ActionFriendly

    foreach (const GlobalShortcut *const shortcut, component->allShortcuts()) {
        if (shortcut->isFresh()) {
            // isFresh is only an intermediate state, not to be reported outside.
            continue;
        }
        QStringList actionId(partialId);
        actionId[KGlobalAccel::ActionUnique] = shortcut->uniqueName();
        actionId[KGlobalAccel::ActionFriendly] = shortcut->friendlyName();
        ret.append(actionId);
    }
    return ret;
}


QStringList KdedGlobalAccel::action(int key) const
{
    GlobalShortcut *shortcut = GlobalShortcutsRegistry::self()->getShortcutByKey(key);
    QStringList ret;
    if (shortcut) {
        ret.append(shortcut->context()->component()->uniqueName());
        ret.append(shortcut->uniqueName());
        ret.append(shortcut->context()->component()->friendlyName());
        ret.append(shortcut->friendlyName());
    }
    return ret;
}


void KdedGlobalAccel::activateGlobalShortcutContext(
            const QString &component,
            const QString &uniqueName)
{
    KdeDGlobalAccel::Component *const comp =
        GlobalShortcutsRegistry::self()->getComponent(component);

    comp->activateGlobalShortcutContext(uniqueName);
}


QList<int> KdedGlobalAccel::shortcut(const QStringList &action) const
{
    GlobalShortcut *shortcut = d->findAction(action);
    if (shortcut)
        return shortcut->keys();
    return QList<int>();
}


QList<int> KdedGlobalAccel::defaultShortcut(const QStringList &action) const
{
    GlobalShortcut *shortcut = d->findAction(action);
    if (shortcut)
        return shortcut->defaultKeys();
    return QList<int>();
}


// This method just registers the action. Nothing else. Shortcut has to be set
// later.
void KdedGlobalAccel::doRegister(const QStringList &actionId)
{
    if (actionId.size() < 4) {
        return;
    }

    GlobalShortcut *shortcut = d->findAction(actionId);
    if (!shortcut) {
        shortcut = d->addAction(actionId);
    } else {
        //a switch of locales is one common reason for a changing friendlyName
        if ((!actionId[KGlobalAccel::ActionFriendly].isEmpty()) && shortcut->friendlyName() != actionId[KGlobalAccel::ActionFriendly]) {
            shortcut->setFriendlyName(actionId[KGlobalAccel::ActionFriendly]);
            scheduleWriteSettings();
        }
        if ((!actionId[KGlobalAccel::ComponentFriendly].isEmpty())
                && shortcut->context()->component()->friendlyName() != actionId[KGlobalAccel::ComponentFriendly]) {
            shortcut->context()->component()->setFriendlyName(actionId[KGlobalAccel::ComponentFriendly]);
            scheduleWriteSettings();
        }
    }
}


QList<KGlobalShortcutInfo> KdedGlobalAccel::getGlobalShortcutsByKey(int key) const
    {
    QList<GlobalShortcut*> shortcuts =
        GlobalShortcutsRegistry::self()->getShortcutsByKey(key);

    QList<KGlobalShortcutInfo> rc;
    Q_FOREACH(GlobalShortcut const*sc, shortcuts)
        {
        rc.append(static_cast<KGlobalShortcutInfo>(*sc));
        }

    Q_FOREACH(KGlobalShortcutInfo inf, rc)
        {
        kDebug() << inf.uniqueName();
        kDebug() << inf.friendlyName();
        kDebug() << inf.contextFriendlyName();
        kDebug() << inf.contextUniqueName();
        kDebug() << inf.componentUniqueName();
        kDebug() << inf.componentFriendlyName();
        }
    return rc;
    }


bool KdedGlobalAccel::isGlobalShortcutAvailable(int shortcut, const QString &component) const
    {
    return GlobalShortcutsRegistry::self()->isShortcutAvailable(shortcut, component);
    }


void KdedGlobalAccel::setInactive(const QStringList &actionId)
    {
    GlobalShortcut *shortcut = d->findAction(actionId);
    if (shortcut)
        shortcut->setInactive();
    }


void KdedGlobalAccel::unRegister(const QStringList &actionId)
{
    // Stop grabbing the key
    GlobalShortcut *shortcut = d->findAction(actionId);
    if (shortcut) {
        delete shortcut;
    }

    scheduleWriteSettings();
}


QList<int> KdedGlobalAccel::setShortcut(const QStringList &actionId,
                                        const QList<int> &keys, uint flags)
{
    //spare the DBus framework some work
    const bool setPresent = (flags & SetPresent);
    const bool isAutoloading = !(flags & NoAutoloading);
    const bool isDefault = (flags & IsDefault);

    GlobalShortcut *shortcut = d->findAction(actionId);
    if (!shortcut) {
        return QList<int>();
    }

    //default shortcuts cannot clash because they don't do anything
    if (isDefault) {
        if (shortcut->defaultKeys() != keys) {
            shortcut->setDefaultKeys(keys);
            scheduleWriteSettings();
        }
        return keys;    //doesn't matter
    }

    if (isAutoloading && !shortcut->isFresh()) {
        //the trivial and common case - synchronize the action from our data
        //and exit.
        if (!shortcut->isActive() && setPresent) {
            shortcut->setActive();
        }
        // We are finished here. Return the list of current active keys.
        return shortcut->keys();
    }

    //now we are actually changing the shortcut of the action
    shortcut->setKeys(keys);

    if (setPresent) {
        shortcut->setActive();
    }

    //maybe isFresh should really only be set if setPresent, but only two things should use !setPresent:
    //- the global shortcuts KCM: very unlikely to catch KWin/etc.'s actions in isFresh state
    //- KGlobalAccel::stealGlobalShortcutSystemwide(): only applies to actions with shortcuts
    //  which can never be fresh if created the usual way
    shortcut->setIsFresh(false);

    scheduleWriteSettings();

    return shortcut->keys();
}


void KdedGlobalAccel::setForeignShortcut(const QStringList &actionId, const QList<int> &keys)
{
    GlobalShortcut *shortcut = d->findAction(actionId);
    if (!shortcut)
        return;

    QList<int> newKeys = setShortcut(actionId, keys, NoAutoloading);

    emit yourShortcutGotChanged(actionId, newKeys);
}


void KdedGlobalAccel::scheduleWriteSettings() const
    {
    if (!d->writeoutTimer.isActive())
        d->writeoutTimer.start(500);
    }


#include "moc_kdedglobalaccel.cpp"
