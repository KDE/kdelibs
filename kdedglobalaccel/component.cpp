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

#include "globalshortcutcontext.h"

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

namespace KdeDGlobalAccel {

Component::Component(
            const QString &uniqueName,
            const QString &friendlyName,
            GlobalShortcutsRegistry *registry)
    :   _uniqueName(uniqueName)
        ,_friendlyName(friendlyName)
        ,_registry(registry)
    {
    // Register ourselve with the registry
    if (_registry)
        {
        _registry->addComponent(this);
        }

    createGlobalShortcutContext("default");
    _current = _contexts.value("default");
    }


Component::~Component()
    {
    // Remove ourselve from the registry
    if (_registry)
        {
        _registry->takeComponent(this);
        }

    // We delete all shortcuts from all contexts
    qDeleteAll(_contexts);
    }


bool Component::activateGlobalShortcutContext(const QString &context)
    {
    if (!_contexts.value(context))
        {
        createGlobalShortcutContext(context);
        return false;
        }

    // Deactivate the current contexts shortcuts
    deactivateShortcuts();
    // Switch the context
    _current = _contexts.value(context);
    // Activate the new contexts
    activateShortcuts();

    kDebug() << "New active shortcut context" << context << "for" << _uniqueName;
    return true;
    }


void Component::activateShortcuts()
    {
    Q_FOREACH (GlobalShortcut *shortcut, _current->_actions)
        {
        shortcut->setActive();
        }
    }


void Component::addShortcut(GlobalShortcut *shortcut)
    {
    _current->_actions.insert(shortcut->uniqueName(), shortcut);
    }


QList<GlobalShortcut*> Component::allShortcuts() const
    {
    return _current->_actions.values();
    }


bool Component::createGlobalShortcutContext(const QString &context)
    {
    if (_contexts.value(context))
        {
        kDebug() << "Shortcut Context " << context << "already exists for component " << _uniqueName;
        return false;
        }
    _contexts.insert(context, new GlobalShortcutContext(context));
    return true;
    }


QString Component::friendlyName() const
    {
    return _friendlyName;
    }


GlobalShortcut *Component::getShortcutByKey(int key)
    {
    Q_FOREACH(GlobalShortcut *sc, _current->_actions)
        {
        if (sc->keys().contains(key)) return sc;
        }
    return NULL;
    }


GlobalShortcut *Component::getShortcutByName(const QString &uniqueName)
    {
    return _current->_actions.value(uniqueName);
    }


void Component::loadSettings(KConfigGroup &configGroup)
    {
    // GlobalShortcutsRegistry::loadSettings handles contexts.

    kDebug() << "Loading context" << _current->name() << "for" << _uniqueName;

    Q_FOREACH (const QString &confKey, configGroup.keyList())
        {
        const QStringList entry = configGroup.readEntry(confKey, QStringList());
        if (entry.size() != 3)
            {
            continue;
            }

        // The shortcut will register itself with us
        GlobalShortcut *shortcut = new GlobalShortcut(
                confKey,
                entry[2],
                this);

        QList<int> keys = keysFromString(entry[0]);
        shortcut->setDefaultKeys(keysFromString(entry[1]));
        shortcut->setIsFresh(false);

        Q_FOREACH (int key, keys)
            {
            if (key != 0) 
                {
                if (GlobalShortcutsRegistry::self()->getShortcutByKey(key)) 
                    {
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


void Component::deactivateShortcuts()
    {
    Q_FOREACH (GlobalShortcut *shortcut, _current->_actions)
        {
        shortcut->setInactive();
        }
    }


GlobalShortcut *Component::takeShortcut(GlobalShortcut *shortcut)
    {
    // Try to take the shortcut. Result could be null if the shortcut doesn't
    // belong ti this component.
    shortcut = _current->_actions.take(shortcut->uniqueName());

    // If this was the last shortcut in the context remove it.
    if (_current->name() != "default" && _current->_actions.isEmpty())
        {
        QString toDelete = _current->name();
        _current = _contexts.value("default");
        delete _contexts.take(toDelete);
        }

    // If only the empty default context is left. Remove the component.
    if (_current->name() == "default" && _current->_actions.isEmpty() && _contexts.size() == 1)
        {
        delete this;
        }
    return shortcut;
    }


void Component::writeSettings(KConfigGroup& configGroup) const
    {
    // If we don't delete the current content global shortcut
    // registrations will never not deleted after forgetGlobalShortcut()
    configGroup.deleteGroup();

    // Write the friendly name
    configGroup.writeEntry("_k_friendly_name", friendlyName());

    // Now wrote all contexts
    Q_FOREACH( GlobalShortcutContext *context, _contexts)
        {
        KConfigGroup contextGroup;

        if (context->name() == "default")
            {
            contextGroup = configGroup;
            }
        else
            {
            contextGroup = KConfigGroup(&configGroup, context->name());
            }

        kDebug() << "writing group " << _uniqueName << ":" << context->name();

        Q_FOREACH(const GlobalShortcut *shortcut, context->_actions)
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

            contextGroup.writeEntry(shortcut->uniqueName(), entry);
            }
        }
    }

} // namespace KdeDGlobalAccel
