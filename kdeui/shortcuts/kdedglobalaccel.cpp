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

struct actionData
{
//TODO: clear isPresent when an action's application/mainComponent disappears
    bool isPresent : 1;
    bool isDefaultEmpty : 1;
    QStringList actionId;
    QList<int> keys;
    QList<int> defaultKeys;
};

enum IdField
{
    ComponentField = 0,
    ActionField = 1
};


class KdedGlobalAccelPrivate
{
public:
    KdedGlobalAccelPrivate();
    ~KdedGlobalAccelPrivate();
    actionData *findAction(int) const;
    actionData *findAction(const QStringList &actionId) const;
    actionData *addAction(const QStringList &actionId);
    actionData *takeAction(const QStringList &actionId);
    QList<actionData *> componentActions(const QString &mainComponentName);

    //helpers
    static bool isEmpty(const QList<int>&);
    static QList<int> nonemptyOnly(const QList<int> &);

    KGlobalAccelImpl *impl;

    QHash<int, actionData *> keyToAction;
    QHash<QString, QHash<QString, actionData *> *> mainComponentHashes;

    KConfig config;
    KConfigGroup configGroup;
    QTimer writeoutTimer;
};


KdedGlobalAccelPrivate::KdedGlobalAccelPrivate()
 : config("kglobalshortcutsrc"), configGroup(&config, "KDE Global Shortcuts")
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
    QHash<QString, actionData *> *componentHash = mainComponentHashes.value(actionId.at(ComponentField));
    if (!componentHash)
        return 0;
    return componentHash->value(actionId.at(ActionField));
}


actionData *KdedGlobalAccelPrivate::addAction(const QStringList &actionId)
{
    QHash<QString, actionData *> *componentHash =
          mainComponentHashes.value(actionId.at(ComponentField));
    if (!componentHash) {
        componentHash = new QHash<QString, actionData *>;
        mainComponentHashes.insert(actionId.at(ComponentField), componentHash);
    }
    Q_ASSERT(!componentHash->value(actionId.at(ActionField)));
    actionData *ad = new actionData;
    ad->actionId = actionId;
    componentHash->insert(actionId.at(ActionField), ad);
    return ad;
}


actionData *KdedGlobalAccelPrivate::takeAction(const QStringList &actionId)
{
    QHash<QString, actionData *> *componentHash = mainComponentHashes.value(actionId.at(ComponentField));
    if (!componentHash)
        return 0;
    actionData *ret = componentHash->take(actionId.at(ActionField));
    if (componentHash->isEmpty())
        delete mainComponentHashes.take(actionId.at(ComponentField));
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
    : KDEDModule(parent), d(new KdedGlobalAccelPrivate)
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
    //TODO: is this safe?
    delete d->impl;
    delete d;
}

QStringList KdedGlobalAccel::allComponents()
{
    return d->mainComponentHashes.keys();
}

QStringList KdedGlobalAccel::allActionsForComponent(const QString& component)
{
    return d->mainComponentHashes[component]->keys();
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
    actionData *ad = d->findAction(key);
    if (ad)
        return ad->actionId;
    return QStringList();
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


//TODO: make sure and document that we don't want trailing zero shortcuts in the list
QList<int> KdedGlobalAccel::setShortcut(const QStringList &actionId,
                                        const QList<int> &keys, uint flags)
{
    //spare the DBus framework some work
    const bool isDefaultEmpty = (flags & IsDefaultEmpty);
    const bool setPresent = (flags & SetPresent);
    const bool isAutoloading = !(flags & NoAutoloading);
    const bool isDefault = (flags & IsDefault);

    actionData *ad = d->findAction(actionId);

    //the trivial and common case - synchronize the action from our data and exit
    bool loadKeys = (isAutoloading && ad);
    if (loadKeys) {
        if (!ad->isPresent && setPresent) {
            ad->isPresent = true;
            foreach (int key, ad->keys)
                if (key != 0) {
                    Q_ASSERT(d->keyToAction.value(key) == ad);
                    d->impl->grabKey(key, true);
                }
        }
        ad->isDefaultEmpty = isDefaultEmpty;
        return ad->keys;
    }

    //now we are actually changing the shortcut of the action

    QList<int> added = d->nonemptyOnly(keys);

    bool didCreate = false;
    if (!ad) {
        didCreate = true;
        ad = d->addAction(actionId);
        ad->isPresent = false;
        //the rest will be initialized below
    }
    if (!ad)
        return QList<int>();

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
                if (ad->isPresent)
                    d->impl->grabKey(oldKey, false);
            }
        }
    }

    //update ad
    //note that ad->keys may still get changed later if conflicts are found
    ad->isDefaultEmpty = isDefaultEmpty;
    if (setPresent)
        ad->isPresent = true;
    if (isDefault)
        ad->defaultKeys = keys;
    ad->keys = keys;

    //update keyToAction and find conflicts with other actions
    //this code inherently does the right thing for duplicates in added
    for (int i = 0; i < added.count(); i++) {
        if (!d->keyToAction.contains(added[i])) {
            d->keyToAction.insert(added[i], ad);
        } else {
            //conflict
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

    if (ad->isPresent)
        foreach (int key, added) {
            Q_ASSERT(d->keyToAction.value(key) == ad);
            d->impl->grabKey(key, true);
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
    if (ad->isDefaultEmpty)
        setterFlags |= IsDefaultEmpty;

    QList<int> oldKeys = ad->keys;
    QList<int> newKeys = setShortcut(actionId, keys, setterFlags);

    if (oldKeys != newKeys)
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
    typedef QHash<QString, actionData*> adHash; //avoid comma in macro arguments
    foreach (const adHash *const mc, d->mainComponentHashes) {
        foreach (const actionData *const ad, *mc) {
            QString confKey = ad->actionId.join("\01");
            if (ad->keys == ad->defaultKeys)
            {
                // If this is a default key, make sure we don't keep an old
                // custom key in the config file
                d->configGroup.deleteEntry(confKey);
            }
            else if (!d->isEmpty(ad->keys))
                d->configGroup.writeEntry(confKey, stringFromKeys(ad->keys));
            else
                d->configGroup.writeEntry(confKey, "none");
        }
    }

    d->configGroup.sync();
}


void KdedGlobalAccel::loadSettings()
{
    //TODO: more sanity checks
    QMap<QString, QString> entries = d->configGroup.entryMap();
    QString empty;
    QStringList lActionId(empty);
    lActionId.append(empty);

    QMap<QString, QString>::const_iterator it;
    for (it = entries.constBegin(); it != entries.constEnd(); ++it) {
        //cut at the first occurrence *only*, so no split('\01')
        int snip = it.key().indexOf('\01');
        //TODO: definitely more sanity checks, like bounds check of (snip + 1) :)
        lActionId[ComponentField] = it.key().left(snip);
        lActionId[ActionField] = it.key().mid(snip + 1);
        QList<int> lKeys = keysFromString(it.value());

        actionData *ad = d->addAction(lActionId);
        ad->keys = lKeys;
        ad->isPresent = false;
        //If we loaded an empty action, that action must have been saved
        //because it was *not* empty by default. This boolean does not propagate
        //out of this class, so it's ok to mess with it as we like.
        ad->isDefaultEmpty = false;

        foreach (int key, lKeys)
            if (key != 0)
                d->keyToAction.insert(key, ad);
    }
}


QList<int> KdedGlobalAccel::keysFromString(const QString &str)
{
    QList<int> ret;
    if (str == "none")
        return ret;

    QStringList strList = str.split('\01');
    foreach (const QString &s, strList)
        ret.append(QKeySequence(s)[0]);

    return ret;
}


QString KdedGlobalAccel::stringFromKeys(const QList<int> &keys)
{
    //the special output "none" is generated at the caller
    QString ret;
    foreach (int key, keys) {
        ret.append(QKeySequence(key).toString());
        ret.append('\01');
    }
    //this is safe if the index is out of bounds
    ret.truncate(ret.length() - 1);

    return ret;
}


bool KdedGlobalAccel::keyPressed(int keyQt)
{
    actionData *ad = d->keyToAction.value(keyQt);
    if (!ad || !ad->isPresent)
        return false;

    QStringList data = ad->actionId;
#ifdef Q_WS_X11
    // pass X11 timestamp
    data.append(QString::number(QX11Info::appTime()));
#endif
    emit invokeAction(data);
    return true;
}

#include "kdedglobalaccel.moc"
#include "kdedglobalaccel_adaptor.moc"
