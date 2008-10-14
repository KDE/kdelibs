/*
    This file is part of the KDE libraries

    Copyright (c) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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


#include "kdedglobalaccel.h"
#include "kdedglobalaccel_p.h"
#include "kdebug.h"


#include <QtCore/QTimer>
#include <QtDBus/QDBusMetaType>

#ifdef Q_WS_X11
#include <QtGui/QX11Info>
#include <QtGui/QApplication>
#endif

#include "kconfiggroup.h"
#include "ksharedconfig.h"
#include "kpluginfactory.h"
#include "kpluginloader.h"

K_PLUGIN_FACTORY(KdedGlobalAccelFactory,
                 registerPlugin<KdedGlobalAccel>();
    )
K_EXPORT_PLUGIN(KdedGlobalAccelFactory("globalaccel"))

enum actionIdFields
{
    ComponentUnique = 0,
    ActionUnique = 1,
    ComponentFriendly = 2,
    ActionFriendly = 3
};

class KdedGlobalAccelPrivate
{
public:
    KdedGlobalAccelPrivate();
    ~KdedGlobalAccelPrivate();

    GlobalShortcut *findAction(int) const;
    GlobalShortcut *findAction(const QStringList &actionId) const;
    GlobalShortcut *addAction(const QStringList &actionId);

    Component *component(const QStringList &actionId) const;

    //! Returns true if the list only contains zeros
    static bool isEmpty(const QList<int>&);

    QTimer writeoutTimer;

    KGlobalAccelImpl *impl;
};


KdedGlobalAccelPrivate::KdedGlobalAccelPrivate()
    :   impl(NULL)
    {
    }


KdedGlobalAccelPrivate::~KdedGlobalAccelPrivate()
    {
    }


GlobalShortcut *KdedGlobalAccelPrivate::findAction(int key) const
    {
    return GlobalShortcutsRegistry::instance()->getShortcutByKey(key);
    }


GlobalShortcut *KdedGlobalAccelPrivate::findAction(const QStringList &actionId) const
{
    if (actionId.size() < 4) {
        kDebug(125) << "Skipped because of invalid actionId";
        return 0;
    }
    Component *component = GlobalShortcutsRegistry::instance()->getComponent(actionId.at(ComponentUnique));
    if (!component)
        return 0;
    return component->getShortcutByName(actionId.at(ActionUnique));
}


Component *KdedGlobalAccelPrivate::component(const QStringList &actionId) const
{
    // Get the component for the action. If we have none create a new one
    Component *component = GlobalShortcutsRegistry::instance()->getComponent(actionId.at(ComponentUnique));
    if (!component)
        {
        component = new Component(actionId.at(ComponentUnique), actionId.at(ComponentFriendly));
        GlobalShortcutsRegistry::instance()->addComponent(component);
        Q_ASSERT(component);
        }
    return component;
}


GlobalShortcut *KdedGlobalAccelPrivate::addAction(const QStringList &actionId)
{
    Q_ASSERT(actionId.size() >= 4);

    Component *component = this->component(actionId);
    Q_ASSERT(component);

    Q_ASSERT(!component->getShortcutByName(actionId.at(ActionUnique)));

    return new GlobalShortcut(
            actionId.at(ActionUnique),
            actionId.at(ActionFriendly),
            component);
}


bool KdedGlobalAccelPrivate::isEmpty(const QList<int>& keys)
{
    return keys.count() == keys.count(0);
}


KdedGlobalAccel::KdedGlobalAccel(QObject* parent, const QList<QVariant>&)
 : KDEDModule(parent),
   d(new KdedGlobalAccelPrivate)
{
    qDBusRegisterMetaType<QList<QStringList> >();
    qDBusRegisterMetaType<QList<int> >();

    d->impl = new KGlobalAccelImpl(this);
    GlobalShortcutsRegistry::instance()->setAccelManager(d->impl);
    d->impl->setEnabled(true);

    //TODO: Make this controllable from applications, for example to prevent
    //shortcuts from triggering when the user is entering a shortcut
    connect(&d->writeoutTimer, SIGNAL(timeout()), SLOT(writeSettings()));
    d->writeoutTimer.setSingleShot(true);
    connect(this, SIGNAL(moduleDeleted(KDEDModule *)), SLOT(writeSettings()));

    GlobalShortcutsRegistry::instance()->loadSettings();
}


KdedGlobalAccel::~KdedGlobalAccel()
{
    kDebug(125);
    // Unregister all currently registered actions. Enables the module to be
    // loaded / unloaded by kded.
    GlobalShortcutsRegistry::instance()->setInactive();
    d->impl->setEnabled(false);

    delete d->impl;
    delete d;
}

QList<QStringList> KdedGlobalAccel::allMainComponents() const
{
    kDebug();

    //### Would it be advantageous to sort the components by unique name?
    QList<QStringList> ret;
    QStringList emptyList;
    for (int i = 0; i < 4; i++) {
        emptyList.append(QString());
    }

    foreach (const Component *component, GlobalShortcutsRegistry::instance()->allMainComponents()) {
        QStringList actionId(emptyList);
        actionId[ComponentUnique] = component->uniqueName();
        actionId[ComponentFriendly] = component->friendlyName();
        ret.append(actionId);
    }

    kDebug() << ret;

    return ret;
}

QList<QStringList> KdedGlobalAccel::allActionsForComponent(const QStringList &actionId) const
{
    //### Would it be advantageous to sort the actions by unique name?
    QList<QStringList> ret;

    Component *const component = GlobalShortcutsRegistry::instance()->getComponent(actionId[ComponentUnique]);
    if (!component) {
        return ret;
    }

    QStringList partialId(actionId[ComponentUnique]);   //ComponentUnique
    partialId.append(QString());                        //ActionUnique
    //Use our internal friendlyName, not the one passed in. We should have the latest data.
    partialId.append(component->friendlyName());                 //ComponentFriendly
    partialId.append(QString());                        //ActionFriendly

    foreach (const GlobalShortcut *const shortcut, component->allShortcuts()) {
        if (shortcut->isFresh()) {
            // isFresh is only an intermediate state, not to be reported outside.
            continue;
        }
        QStringList actionId(partialId);
        actionId[ActionUnique] = shortcut->uniqueName();
        actionId[ActionFriendly] = shortcut->friendlyName();
        ret.append(actionId);
    }
    return ret;
}

QStringList KdedGlobalAccel::action(int key) const
{
    GlobalShortcut *shortcut = GlobalShortcutsRegistry::instance()->getShortcutByKey(key);
    QStringList ret;
    if (shortcut) {
        ret.append(shortcut->component()->uniqueName());
        ret.append(shortcut->uniqueName());
        ret.append(shortcut->component()->friendlyName());
        ret.append(shortcut->friendlyName());
    }
    return ret;
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
        if ((!actionId[ActionFriendly].isEmpty()) && shortcut->friendlyName() != actionId[ActionFriendly]) {
            shortcut->setFriendlyName(actionId[ActionFriendly]);
            scheduleWriteSettings();
        }
        if ((!actionId[ComponentFriendly].isEmpty()) && shortcut->component()->friendlyName() != actionId[ComponentFriendly]) {
            shortcut->component()->setFriendlyName(actionId[ComponentFriendly]);
            scheduleWriteSettings();
        }
    }
}


void KdedGlobalAccel::setInactive(const QStringList &actionId)
    {
    GlobalShortcut *shortcut = d->findAction(actionId);
    if (shortcut)
        shortcut->setInactive();
    }


void KdedGlobalAccel::unRegister(const QStringList &actionId)
{
    kDebug(125) << actionId;

    Q_ASSERT(actionId.size()==4);
    if (actionId.size() < 4) {
        return;
    }

    // Stop grabbing the key
    GlobalShortcut *shortcut = d->findAction(actionId);
    if (shortcut) {
        delete shortcut;
    }

    scheduleWriteSettings();
}


//TODO: make sure and document that we don't want trailing zero shortcuts in the list
QList<int> KdedGlobalAccel::setShortcut(const QStringList &actionId,
                                        const QList<int> &keys, uint flags)
{
    kDebug(125) << actionId;
    Q_FOREACH(int key, keys)
        {
        kDebug() << QKeySequence(key).toString();
        }

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
    kDebug(125) << actionId;

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


void KdedGlobalAccel::writeSettings() const
    {
    GlobalShortcutsRegistry::instance()->writeSettings();
    }

bool KdedGlobalAccel::keyPressed(int keyQt)
{
    kDebug(125) << keyQt;

    GlobalShortcut *shortcut = GlobalShortcutsRegistry::instance()->getShortcutByKey(keyQt);
    if (!shortcut || !shortcut->isActive()) {
        kDebug(125) << "skipping because action is not active";
        return false;
    }

    kDebug() << shortcut->uniqueName();

    QStringList data(shortcut->component()->uniqueName());
    data.append(shortcut->uniqueName());
    data.append(shortcut->component()->friendlyName());
    data.append(shortcut->friendlyName());
#ifdef Q_WS_X11
    // pass X11 timestamp
    long timestamp = QX11Info::appTime();
    // Make sure kded has ungrabbed the keyboard after receiving the keypress,
    // otherwise actions in application that try to grab the keyboard (e.g. in kwin)
    // may fail to do so. There is still a small race condition with this being out-of-process.
    qApp->syncX();
#else
    long timestamp = 0;
#endif
    kDebug() << data;
    emit invokeAction(data, timestamp);
    return true;
}

#include "kdedglobalaccel.moc"
#include "kdedglobalaccel_p.moc"
