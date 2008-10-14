/* Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

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

#include "kdedglobalaccel_p.h"
#include <kdebug.h>

static QList<int> keysFromString(const QString &str)
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


static QString stringFromKeys(const QList<int> &keys)
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


Component::Component( const QString &uniqueName, const QString &friendlyName)
        :   _uniqueName(uniqueName)
            ,_friendlyName(friendlyName)
            ,_actions()
    {}


Component::~Component()
    {
    qDeleteAll(_actions);
    }


void Component::addShortcut(GlobalShortcut *shortcut)
    {
    _actions.insert(shortcut->uniqueName(), shortcut);
    }


QList<GlobalShortcut*> Component::allShortcuts() const
    {
    return _actions.values();
    }


QString Component::friendlyName() const
    {
    return _friendlyName;
    }


GlobalShortcut *Component::getShortcutByKey(int key)
    {
    Q_FOREACH(GlobalShortcut *sc, _actions)
        {
        if (sc->keys().contains(key)) return sc;
        }
    return NULL;
    }


GlobalShortcut *Component::getShortcutByName(const QString &uniqueName)
    {
    return _actions.value(uniqueName);
    }


void Component::loadSettings(KConfigGroup &configGroup)
    {
        foreach (const QString &confKey, configGroup.keyList()) 
            {
            const QStringList entry = configGroup.readEntry(confKey, QStringList());
            if (entry.size() != 3) 
                {
                continue;
                }

            GlobalShortcut *shortcut = new GlobalShortcut(
                    confKey,
                    entry[2],
                    this);

            QList<int> keys = keysFromString(entry[0]);
            shortcut->setDefaultKeys(keysFromString(entry[1]));
            shortcut->setIsFresh(false);

            foreach (int key, keys) {
                if (key != 0) {
                    if (GlobalShortcutsRegistry::instance()->getShortcutByKey(key)) {
                        // The shortcut is already used. The config file is
                        // broken. Ignore the request.
                        keys.removeAll(key);
                        kWarning() << "Shortcut found twice in kglobalshortcutsrc.";
                    }
                }
            }
            shortcut->setKeys(keys);
        }
    }


void Component::setFriendlyName(const QString &name)
    {
    _friendlyName = name;
    }


QString Component::uniqueName() const
    {
    return _uniqueName;
    }


void Component::setInactive()
    {
    Q_FOREACH (GlobalShortcut *shortcut, _actions)
        {
        shortcut->setInactive();
        }
    }


GlobalShortcut *Component::takeAction(GlobalShortcut *shortcut)
    {
    if (shortcut->component() == this)
        {
        return _actions.take(shortcut->uniqueName());
        }
    return NULL;
    }


void Component::writeSettings(KConfigGroup& configGroup) const
    {
    // If we don't delete the current content global shortcut
    // registrations will never not deleted after forgetGlobalShortcut()
    configGroup.deleteGroup();

    KConfigGroup friendlyGroup(&configGroup, "Friendly Name");
    friendlyGroup.writeEntry("Friendly Name", friendlyName());

    foreach (const GlobalShortcut *shortcut, allShortcuts()) 
        {
        // We do not write fresh shortcuts.
        // We do not write session shortcuts
        if (shortcut->isFresh() || shortcut->uniqueName().startsWith("_k_session:"))
            {
            continue;
            }

        QStringList entry(stringFromKeys(shortcut->keys()));
        entry.append(stringFromKeys(shortcut->defaultKeys()));
        entry.append(shortcut->friendlyName());

        configGroup.writeEntry(shortcut->uniqueName(), entry);
        }
    }
