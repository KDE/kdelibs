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
#include <kdebug.h>
#include "kdedglobalaccel_adaptor.h"

// For KGlobalAccelImpl
#ifdef Q_WS_X11
#include "kglobalaccel_x11.h"
#elif defined(Q_WS_MACX)
#include "kglobalaccel_mac.h"
#elif defined(Q_WS_WIN)
#include "kglobalaccel_win.h"
#else
#include "kglobalaccel_qws.h"
#endif

#include <QtCore/QHash>
#include <QtCore/QTimer>

#ifdef Q_WS_X11
#include <QtGui/QX11Info>
#include <QtGui/QApplication>
#endif

#include <kconfiggroup.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

K_PLUGIN_FACTORY(KdedGlobalAccelFactory,
                 registerPlugin<KdedGlobalAccel>();
    )
K_EXPORT_PLUGIN(KdedGlobalAccelFactory("globalaccel"))

struct componentData
{
    QString uniqueName;
    //the name as it would be found in a magazine article about the application,
    //possibly localized if a localized name exists.
    QString friendlyName;
    QHash<QString, actionData *> actions;
};

struct actionData
{
//TODO: clear isPresent when an action's application/mainComponent disappears
    bool isPresent:1;
    bool isFresh:1;
    componentData *parent;
    QString uniqueName;
    QString friendlyName; //usually localized
    QList<int> keys;
    QList<int> defaultKeys;
};

enum actionIdFields
{
    ComponentUnique = 0,
    ActionUnique = 1,
    ComponentFriendly = 2,
    ActionFriendly = 3
};


//Consider to emit warnings if an actionId does not contain enough elements of that turns out
//to be a source of bugs.


class KdedGlobalAccelPrivate
{
public:
    KdedGlobalAccelPrivate();
    ~KdedGlobalAccelPrivate();
    actionData *findAction(int) const;
    actionData *findAction(const QStringList &actionId) const;
    actionData *addAction(const QStringList &actionId);
    actionData *takeAction(const QStringList &actionId);

    //helpers
    static bool isEmpty(const QList<int>&);
    static QList<int> nonemptyOnly(const QList<int> &);

    KGlobalAccelImpl *impl;

    QHash<int, actionData *> keyToAction;
    QHash<QString, componentData *> mainComponents;

    KConfig config;
    QTimer writeoutTimer;
};


KdedGlobalAccelPrivate::KdedGlobalAccelPrivate()
 : config("kglobalshortcutsrc", KConfig::SimpleConfig)
{
}


KdedGlobalAccelPrivate::~KdedGlobalAccelPrivate()
{
}


actionData *KdedGlobalAccelPrivate::findAction(int key) const
{
    return keyToAction.value(key);
}


actionData *KdedGlobalAccelPrivate::findAction(const QStringList &actionId) const
{
    if (actionId.count() < 2)
        return 0;
    componentData *cd = mainComponents.value(actionId.at(ComponentUnique));
    if (!cd)
        return 0;
    return cd->actions.value(actionId.at(ActionUnique));
}


actionData *KdedGlobalAccelPrivate::addAction(const QStringList &actionId)
{
    Q_ASSERT(actionId.size() >= 4);
    componentData *cd = mainComponents.value(actionId.at(ComponentUnique));
    if (!cd) {
        cd = new componentData;
        cd->uniqueName = actionId.at(ComponentUnique);
        cd->friendlyName = actionId.at(ComponentFriendly);
        mainComponents.insert(actionId.at(ComponentUnique), cd);
    }
    Q_ASSERT(!cd->actions.value(actionId.at(ActionUnique)));
    actionData *ad = new actionData;
    ad->parent = cd;
    ad->uniqueName = actionId.at(ActionUnique);
    ad->friendlyName = actionId.at(ActionFriendly);
    cd->actions.insert(actionId.at(ActionUnique), ad);
    return ad;
}


actionData *KdedGlobalAccelPrivate::takeAction(const QStringList &actionId)
{
    componentData *cd = mainComponents.value(actionId.at(ComponentUnique));
    if (!cd)
        return 0;
    actionData *ret = cd->actions.take(actionId.at(ActionUnique));
    if (cd->actions.isEmpty())
        delete mainComponents.take(actionId.at(ComponentUnique));
    return ret;
}


//return if a list of keys is *logically* empty
//static
bool KdedGlobalAccelPrivate::isEmpty(const QList<int>& keys)
{
    const int count = keys.count();
    for (int i = 0; i < count; i++)
        if (keys[i] != 0)
            return false;

    return true;
}


//static
QList<int> KdedGlobalAccelPrivate::nonemptyOnly(const QList<int> &keys)
{
    QList<int> ret;
    const int count = keys.count();
    for (int i = 0; i < count; i++)
        if (keys[i] != 0)
            ret.append(keys[i]);

    return ret;
}


KdedGlobalAccel::KdedGlobalAccel(QObject* parent, const QList<QVariant>&)
 : KDEDModule(parent),
   d(new KdedGlobalAccelPrivate)
{
    qDBusRegisterMetaType<QList<int> >();

    d->impl = new KGlobalAccelImpl(this);
    //TODO: Make this controllable from applications, for example to prevent
    //shortcuts from triggering when the user is entering a shortcut
    d->impl->setEnabled(true);
    connect(&d->writeoutTimer, SIGNAL(timeout()), SLOT(writeSettings()));
    d->writeoutTimer.setSingleShot(true);
    connect(this, SIGNAL(moduleDeleted(KDEDModule *)), SLOT(writeSettings()));

    loadSettings();
    new KdedGlobalAccelAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/KdedGlobalAccel", this);
}


KdedGlobalAccel::~KdedGlobalAccel()
{
    // TODO: Rip out all that StringLists and Lists
    // ... by providing classes for ActionId and Component and whatever is
    // implemented by a QStringList.

    // Unregister "/KdedGlobalAccel" explicit
    QDBusConnection::sessionBus().unregisterObject("/KdedGlobalAccel" );

    // Unregister all currently registered actions. Enables the module to be
    // loaded / unloaded by kded.
    Q_FOREACH (const QStringList &component, allComponents()) {
        Q_FOREACH (const QStringList &actionId, allActionsForComponent(component)) {
            setInactive(actionId);
        }
    }

    //TODO: is this safe?
    delete d->impl;
    delete d;
}

QList<QStringList> KdedGlobalAccel::allComponents()
{
    //### Would it be advantageous to sort the components by unique name?
    QList<QStringList> ret;
    QStringList emptyList;
    for (int i = 0; i < 4; i++) {
        emptyList.append(QString());
    }

    foreach (const componentData *const cd, d->mainComponents) {
        QStringList actionId(emptyList);
        actionId[ComponentUnique] = cd->uniqueName;
        actionId[ComponentFriendly] = cd->friendlyName;
        ret.append(actionId);
    }
    return ret;
}

QList<QStringList> KdedGlobalAccel::allActionsForComponent(const QStringList &actionId)
{
    //### Would it be advantageous to sort the actions by unique name?
    QList<QStringList> ret;

    componentData *const cd = d->mainComponents.value(actionId[ComponentUnique]);
    if (!cd) {
        return ret;
    }

    QStringList partialId(actionId[ComponentUnique]);   //ComponentUnique
    partialId.append(QString());                        //ActionUnique
    //Use our internal friendlyName, not the one passed in. We should have the latest data.
    partialId.append(cd->friendlyName);                 //ComponentFriendly
    partialId.append(QString());                        //ActionFriendly

    foreach (const actionData *const ad, cd->actions) {
        if (ad->isFresh) {
            // isFresh is only an intermediate state, not to be reported outside.
            continue;
        }
        QStringList actionId(partialId);
        actionId[ActionUnique] = ad->uniqueName;
        actionId[ActionFriendly] = ad->friendlyName;
        ret.append(actionId);
    }
    return ret;
}

QList<int> KdedGlobalAccel::allKeys()
{
    QList<int> ret = d->keyToAction.keys();
    kDebug() << ret;
    return ret;
}

QStringList KdedGlobalAccel::allKeysAsString()
{
    QStringList ret;
    foreach(int keyQt, d->keyToAction.keys())
        ret << QKeySequence(keyQt).toString();
    return ret;
}

QStringList KdedGlobalAccel::actionId(int key)
{
    QStringList ret;
    if (actionData *ad = d->findAction(key)) {
        ret.append(ad->parent->uniqueName);
        ret.append(ad->uniqueName);
        ret.append(ad->parent->friendlyName);
        ret.append(ad->friendlyName);
    }
    return ret;
}


QList<int> KdedGlobalAccel::shortcut(const QStringList &action)
{
    actionData *ad = d->findAction(action);
    if (ad)
        return ad->keys;
    return QList<int>();
}


QList<int> KdedGlobalAccel::defaultShortcut(const QStringList &action)
{
    actionData *ad = d->findAction(action);
    if (ad)
        return ad->defaultKeys;
    return QList<int>();
}


void KdedGlobalAccel::doRegister(const QStringList &actionId)
{
    if (actionId.size() < 4) {
        return;
    }
    actionData *ad = d->findAction(actionId);
    if (!ad) {
        ad = d->addAction(actionId);
        //addAction only fills in the names
        ad->isPresent = false;
        ad->isFresh = true;
        //scheduleWriteSettings();  //we don't write out isFresh actions, cf. writeSettings()
    } else {
        //a switch of locales is one common reason for a changing friendlyName
        if ((!actionId[ActionFriendly].isEmpty()) && ad->friendlyName != actionId[ActionFriendly]) {
            ad->friendlyName = actionId[ActionFriendly];
            scheduleWriteSettings();
        }
        if ((!actionId[ComponentFriendly].isEmpty()) && ad->parent->friendlyName != actionId[ComponentFriendly]) {
            ad->parent->friendlyName = actionId[ComponentFriendly];
            scheduleWriteSettings();
        }
    }
}


void KdedGlobalAccel::unRegister(const QStringList &actionId)
{
    kDebug(125) << actionId;

    Q_ASSERT(actionId.size()==4);
    if (actionId.size() < 4) {
        return;
    }

    // Stop grabbing the key
    setInactive(actionId);
    actionData *ad = d->takeAction(actionId);
    // Don't let dangling pointers behind
    Q_FOREACH(int key, d->keyToAction.keys(ad)) {
        d->keyToAction.remove(key);
    }
    delete ad;

    scheduleWriteSettings();
}


//TODO: make sure and document that we don't want trailing zero shortcuts in the list
QList<int> KdedGlobalAccel::setShortcut(const QStringList &actionId,
                                        const QList<int> &keys, uint flags)
{
    //spare the DBus framework some work
    const bool setPresent = (flags & SetPresent);
    const bool isAutoloading = !(flags & NoAutoloading);
    const bool isDefault = (flags & IsDefault);

    actionData *ad = d->findAction(actionId);
    if (!ad) {
        return QList<int>();
    }

    //default shortcuts cannot clash because they don't do anything
    if (isDefault) {
        if (ad->defaultKeys != keys) {
            ad->defaultKeys = keys;
            scheduleWriteSettings();
        }
        return keys;    //doesn't matter
    }

    //the trivial and common case - synchronize the action from our data and exit
    if (isAutoloading && !ad->isFresh) {
        if (!ad->isPresent && setPresent) {
            ad->isPresent = true;
            foreach (int key, ad->keys) {
                if (key != 0) {
                    Q_ASSERT(d->keyToAction.value(key) == ad);
                    d->impl->grabKey(key, true);
                }
            }
        }
        return ad->keys;
    }

    //now we are actually changing the shortcut of the action

    QList<int> added = d->nonemptyOnly(keys);

    //take care of stale keys and remove from added these that remain.
    foreach(int oldKey, ad->keys) {
        if (oldKey != 0) {
            bool remains = false;
            for (int i = 0; i < added.count(); i++) {
                if (oldKey == added[i]) {
                    added.removeAt(i);
                    i--;
                    remains = true;
                    //no break; - remove possible duplicates
                }
            }
            if (!remains) {
                d->keyToAction.remove(oldKey);
                if (ad->isPresent) {
                    d->impl->grabKey(oldKey, false);
                }
            }
        }
    }

    //update ad
    //note that ad->keys may still get changed later if conflicts are found
    if (setPresent) {
        ad->isPresent = true;
    }
    ad->keys = keys;
    //maybe isFresh should really only be set if setPresent, but only two things should use !setPresent:
    //- the global shortcuts KCM: very unlikely to catch KWin/etc.'s actions in isFresh state
    //- KGlobalAccel::stealGlobalShortcutSystemwide(): only applies to actions with shortcuts
    //  which can never be fresh if created the usual way
    ad->isFresh = false;

    //update keyToAction and find conflicts with other actions
    //this code inherently does the right thing for duplicates in added
    for (int i = 0; i < added.count(); i++) {
        if (!d->keyToAction.contains(added[i])) {
            d->keyToAction.insert(added[i], ad);
        } else {
            //clash
            for (int j = 0; j < ad->keys.count(); j++) {
                if (ad->keys[j] == added[i]) {
                    if (ad->keys.last() == added[i]) {
                        ad->keys.removeLast();
                        j--;
                    } else
                        ad->keys[j] = 0;
                }
            }
            added.removeAt(i);
            i--;
        }
    }

    if (ad->isPresent) {
        foreach (int key, added) {
            Q_ASSERT(d->keyToAction.value(key) == ad);
            d->impl->grabKey(key, true);
        }
    }

    scheduleWriteSettings();

    return ad->keys;
}


void KdedGlobalAccel::setForeignShortcut(const QStringList &actionId, const QList<int> &keys)
{
    actionData *ad = d->findAction(actionId);
    if (!ad)
        return;

    uint setterFlags = NoAutoloading;

    QList<int> oldKeys = ad->keys;
    QList<int> newKeys = setShortcut(actionId, keys, setterFlags);

    // if (oldKeys == newKeys)
    //     return;
    // We cannot make that comparison or we break KGlobalAccel which first
    // calls setShortcut() and *then* setForeignShortcut. 
    emit yourShortcutGotChanged(actionId, newKeys);
}


void KdedGlobalAccel::setInactive(const QStringList &actionId)
{
    actionData *ad = d->findAction(actionId);
    if (!ad)
        return;
    ad->isPresent = false;

    const int len = ad->keys.count();
    for (int i = 0; i < len; i++)
        if (ad->keys[i] != 0)
            d->impl->grabKey(ad->keys[i], false);
}


void KdedGlobalAccel::scheduleWriteSettings()
{
    if (!d->writeoutTimer.isActive())
        d->writeoutTimer.start(500);
}


//slot
void KdedGlobalAccel::writeSettings()
{
    foreach (const componentData *const cd, d->mainComponents) {
        KConfigGroup configGroup(&d->config, cd->uniqueName);

        KConfigGroup friendlyGroup(&configGroup, "Friendly Name");  // :)
        friendlyGroup.writeEntry("Friendly Name", cd->friendlyName);

        foreach (const actionData *const ad, cd->actions) {
            if (ad->isFresh) {
                //no shortcut assignement took place, the action was only registered
                //(we could still write it out to document its existence, but the "fresh"
                //state should be regarded as transitional *only*.)
                continue;
            }
            QStringList entry(stringFromKeys(ad->keys));
            entry.append(stringFromKeys(ad->defaultKeys));
            entry.append(ad->friendlyName);

            configGroup.writeEntry(ad->uniqueName, entry);
        }
    }

    d->config.sync();
}


void KdedGlobalAccel::loadSettings()
{
    QStringList lActionId;
    for (int i = 0; i < 4; i++) {
        lActionId.append(QString());
    }

    foreach (const QString &groupName, d->config.groupList()) {
        KConfigGroup configGroup(&d->config, groupName);
        lActionId[ComponentUnique] = groupName;

        KConfigGroup friendlyGroup(&configGroup, "Friendly Name");
        lActionId[ComponentFriendly] = friendlyGroup.readEntry("Friendly Name");

        foreach (const QString &confKey, configGroup.keyList()) {
            const QStringList entry = configGroup.readEntry(confKey, QStringList());
            if (entry.size() != 3) {
                continue;
            }
            lActionId[ActionUnique] = confKey;
            lActionId[ActionFriendly] = entry[2];

            actionData *ad = d->addAction(lActionId);
            ad->keys = keysFromString(entry[0]);
            ad->defaultKeys = keysFromString(entry[1]);
            ad->isPresent = false;
            ad->isFresh = false;
    
            foreach (int key, ad->keys) {
                if (key != 0) {
                    if (d->keyToAction.contains(key)) {
                        // The shortcut is already used. The config file is
                        // broken. Ignore the request.
                        ad->keys.removeAll(key);
                        kWarning() << "Shortcut found twice in kglobalshortcutsrc.";
                    } else {
                        d->keyToAction.insert(key, ad);
                    }
                }
            }
        }
    }
}


QList<int> KdedGlobalAccel::keysFromString(const QString &str)
{
    QList<int> ret;
    if (str == "none") {
        return ret;
    }
    QStringList strList = str.split('\t');
    foreach (const QString &s, strList) {
        int key = QKeySequence(s)[0];
        if (key != -1) {     //sanity check just in case
            ret.append(key);
        }
    }
    return ret;
}


QString KdedGlobalAccel::stringFromKeys(const QList<int> &keys)
{
    if (keys.isEmpty()) {
        return "none";
    }
    QString ret;
    foreach (int key, keys) {
        ret.append(QKeySequence(key).toString());
        ret.append('\t');
    }
    ret.chop(1);
    return ret;
}


bool KdedGlobalAccel::keyPressed(int keyQt)
{
    actionData *ad = d->keyToAction.value(keyQt);
    if (!ad || !ad->isPresent)
        return false;

    QStringList data(ad->parent->uniqueName);
    data.append(ad->uniqueName);
    data.append(ad->parent->friendlyName);
    data.append(ad->friendlyName);
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
    emit invokeAction(data, timestamp);
    return true;
}

#include "kdedglobalaccel.moc"
#include "kdedglobalaccel_adaptor.moc"
