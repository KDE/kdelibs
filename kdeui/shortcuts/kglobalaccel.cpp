/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

// For KGlobalAccelImpl
#ifdef Q_WS_X11
#include "kglobalaccel_x11.h"
#elif defined(Q_WS_MACX)
#include "kglobalaccel_mac.h"
#elif defined(Q_WS_WIN)
#include "kglobalaccel_win.h"
#else
#include "kglobalaccel_emb.h"
#endif

#include <QHash>
#include <QStringList>

#include <kdebug.h>
#include <klocale.h>
#include "kaction.h"
#include "kactioncollection.h"
#include "kmessagebox.h"
#include <kconfig.h>
#include <kconfiggroup.h>


class KGlobalAccelPrivate
{
public:
    KGlobalAccelPrivate(KGlobalAccelImpl *impl)
     : configGroup("Global Shortcuts")
     , enabled(true)
     , implEnabled(false)
     , impl(impl)
    {
    }

    QHash<int, KAction*> grabbedKeys;
    QMultiHash<KAction*, int> grabbedActions;

    //for all actions with (isEnabled() && globalShortcutAllowed())
    QHash<QString, KAction *> nameToAction;
    QHash<KAction *, QString> actionToName;
    //for all registered global shortcuts including those not present ATM
    QHash<QKeySequence, QStringList> systemWideGlobalShortcuts;

    QString mainComponentName;
    QString configGroup;
    bool enabled, implEnabled;
    KGlobalAccelImpl *const impl;
};

KGlobalAccel::KGlobalAccel()
    : d(new KGlobalAccelPrivate(new KGlobalAccelImpl(this)))
{
    //TODO: is fromUtf8 correct?
    if (KGlobal::hasMainComponent())
        d->mainComponentName = QString::fromUtf8(KGlobal::mainComponent().componentName());
}

KGlobalAccel::~KGlobalAccel()
{
    foreach (int key, d->grabbedKeys.keys())
        d->impl->grabKey(key, false);

    delete d->impl;
    delete d;
}

bool KGlobalAccel::isEnabled()
{
    return d->enabled;
}

void KGlobalAccel::setEnabled( bool enabled )
{
    d->enabled = enabled;

//TODO: think of something sensible :|
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


void KGlobalAccel::readSettings()
{
    KConfigBase *config = KGlobal::config().data();
    QStringList actionIdentifier("");
    actionIdentifier.append(QString());
    QString mainComponentName;
    QString actionName;
    KAction *action;
    KShortcut cut;

    QMap<QString, QString> entryMap = config->entryMap( d->configGroup );
    for (QMap<QString, QString>::const_iterator it = entryMap.constBegin();
        it != entryMap.constEnd(); ++it) {
        int splitpos;

        //TODO: what if mainComponentName contains a colon?
        if ((splitpos = it.key().indexOf(':')) < 1)
            continue;
        mainComponentName = it.key().left(splitpos);
        actionName = it.key().right(it.key().length() - splitpos - 1);

        if (it.value() != "none") {
            cut = KShortcut(it.value());
            if (!cut.isEmpty()) {
                actionIdentifier[0] = mainComponentName;
                actionIdentifier[1] = actionName;
                foreach (QKeySequence seq, cut.toList())
                    d->systemWideGlobalShortcuts.insert(seq, actionIdentifier);
            }
        } else
            cut = KShortcut();

        if (mainComponentName == d->mainComponentName && (action = d->nameToAction.value(actionName)))
            action->setActiveGlobalShortcutNoEnable(cut);
    }

//The old version of this function looked for actions in KActionCollection::allCollections().
//This seemed weird - if an action is programmatically forbidden to have a global shortcut then
//so be it. -- ahartmetz
}

void KGlobalAccel::writeSettings(KAction *oneAction) const
{
    KConfigGroup cg(KGlobal::config().data(), d->configGroup);
    QStringList actionIdentifier(d->mainComponentName);
    actionIdentifier.append(QString());
    QString fullName;
    QList<KAction *> actions;

    //TODO: does KConfigBase::deleteEntry fail gracefully on a nonexistent entry?
    //if (cg.hasKey(fullName)) cg.deleteEntry(fullName); looks redundant.
    if (oneAction) {
        actions.append(oneAction);
    } else
        actions = d->actionToName.keys();

    foreach (KAction* action, actions) {
        actionIdentifier[1] = action->objectName();
        fullName = actionIdentifier.join(":");

        if (!action->globalShortcut(KAction::ActiveShortcut).isEmpty())
            cg.writeEntry(fullName, action->globalShortcut().toString());
        else if (!action->globalShortcut(KAction::DefaultShortcut).isEmpty())
            cg.writeEntry(fullName, "none");
        else if (cg.hasKey(fullName))
            cg.deleteEntry(fullName);
    }

    KGlobal::config()->sync();
}

KGlobalAccel * KGlobalAccel::self( )
{
    K_GLOBAL_STATIC(KGlobalAccel, s_instance)
    return s_instance;
}


//This function is supposed to update the *internal* data of this KGlobalAccel -
//kdeglobals will already be updated by the signaller.
void KGlobalAccel::DBusShortcutChangeListener(const QStringList &actionIdentifier, const QList<QKeySequence> &oldCuts, const QList<QKeySequence> &newCuts)
{
    QString mainComponentName = actionIdentifier.at(0);
    QString actionName = actionIdentifier.at(1);
    KAction *kaction;

    foreach (QKeySequence oldCut, oldCuts)
        d->systemWideGlobalShortcuts.remove(oldCut);

    if (mainComponentName == d->mainComponentName) {
        kaction = d->nameToAction.value(actionName);
        if (kaction) {
            kaction->setActiveGlobalShortcutNoEnable(KShortcut(newCuts));
            //KAction will call updateGlobalShortcut() which will do what's needed
            return;
        }
    }

    foreach (QKeySequence newCut, newCuts) {
        if (!newCut.isEmpty())
            d->systemWideGlobalShortcuts.insert(newCut, actionIdentifier);
    }
    
}


//Sadly, this cannot be done using DBusShortcutChangeListener() because it is unknown
//to the thief which shortcuts the victim possesses.
void KGlobalAccel::DBusShortcutTheftListener(const QKeySequence &loot)
{
    KAction *kaction;
    QStringList shortcutIdentifier = d->systemWideGlobalShortcuts.value(loot);
    //QString mainComponentName = shortcutIdentfier.at(0);
    //QString actionName = shortcutIdentfier.at(1);

    if (shortcutIdentifier.at(0) == d->mainComponentName) {
        kaction = d->nameToAction.value(shortcutIdentifier.at(1));
        if (kaction) {
            KShortcut cut = kaction->globalShortcut();

            if (loot == cut.primary())
                cut.primary() = QKeySequence();
            if (loot == cut.alternate())
                cut.alternate() = QKeySequence();

            //KAction will call updateGlobalShortcut() which will publish an update via DBus...
            kaction->setActiveGlobalShortcutNoEnable(cut);
        }
    }
}


//It is only allowed to call this in case of real changes. Save some cycles...
void KGlobalAccel::updateGlobalShortcut(KAction *action, const KShortcut &oldShortcut)
{
    //abort if any of the new shortcuts are already taken
    foreach (QKeySequence oldSeq, oldShortcut.toList()) {
        QStringList oldOwner;
        if (!d->systemWideGlobalShortcuts.contains(oldSeq))
            continue;

        oldOwner = d->systemWideGlobalShortcuts.take(oldSeq);
        if (oldOwner.at(0) != d->mainComponentName || oldOwner.at(1) != action->objectName()) {
            //TODO: emit a warning
            return;
        }
    }

    //perform local changes right away to avoid inconsistent data until a DBus roundtrip has completed
    foreach (QKeySequence oldCut, oldShortcut.toList())
        d->systemWideGlobalShortcuts.remove(oldCut);

    QStringList actionId(d->mainComponentName);
    actionId.append(QString());
    foreach (QKeySequence newCut, action->globalShortcut().toList()) {
        actionId[1] = action->objectName();
        d->systemWideGlobalShortcuts.insert(newCut, actionId);
    }

    changeGrab(action, action->globalShortcut());

    //update kdeglobals...
    KConfigGroup cg(KGlobal::config().data(), d->configGroup);
    QString fullName = d->mainComponentName;
    fullName.append(':');
    fullName.append(action->objectName());

    if (!action->globalShortcut(KAction::ActiveShortcut).isEmpty())
        cg.writeEntry(fullName, action->globalShortcut().toString());
    else if (!action->globalShortcut(KAction::DefaultShortcut).isEmpty())
        cg.writeEntry(fullName, "none");
    else if (cg.hasKey(fullName))
        cg.deleteEntry(fullName);

    //publish changes to all KDE programs in our session
    //TODO
    //d->dbus.shortcutUpdate(d->mainComponentName, action->objectName());
}


//Note that the shortcut's entry in kdeglobals will NOT be touched.
//This entry will document the usage status of this action's global shortcut.
void KGlobalAccel::enableAction(KAction *action, bool enable)
{
    if (enable) {
        //make sure that all necessary keys are grabbed
        changeGrab(action, action->globalShortcut());

        if (d->actionToName.contains(action))
            return;

        d->nameToAction.insert(action->objectName(), action);
        d->actionToName.insert(action, action->objectName());

    } else {
        //ungrab any grabbed keys
        changeGrab(action, KShortcut());

        if (!d->actionToName.contains(action))
            return;

        d->nameToAction.remove(d->actionToName.take(action));
    }
}


//slot
void KGlobalAccel::actionChanged()
{
    KAction* action = qobject_cast<KAction*>(sender());
    if (action)
        enableAction(action, (action->globalShortcutAllowed() && action->isEnabled()));
}


void KGlobalAccel::updateGlobalShortcutAllowed(KAction *action)
{
    if (!action)
        return;

    enableAction (action, action->globalShortcutAllowed() && action->isEnabled());
    if (action->globalShortcutAllowed())
        connect(action, SIGNAL(changed()), this, SLOT(actionChanged()));
    else
        disconnect(action, SIGNAL(changed()), this, SLOT(actionChanged()));
}


//ungrab oldGrab, grab newGrab, and leave unchanged what's in both
void KGlobalAccel::changeGrab(KAction *action, const KShortcut &newGrab)
{
    //FIXME: very bad workaround to avoid a bug that kills keyboard input.
    return;
    QList<int> needToGrab;
    foreach (const QKeySequence &seq, newGrab.toList())
        needToGrab.append(seq);

    QList<int> staleGrabs = d->grabbedActions.values(action);

    for (int i = 0, end = staleGrabs.count(); i < end; i++) {
        if (int found = needToGrab.indexOf(staleGrabs.at(i)) != -1) {
            needToGrab.removeAt(found);
            staleGrabs.removeAt(i);
            end--;
            i--;
        }
    }

    foreach (int staleGrab, staleGrabs) {
        d->grabbedKeys.remove(staleGrab);

        //remove entry action->staleGrab
        QMultiHash<KAction *, int>::iterator it;
        it = d->grabbedActions.find(action);
        while (it != d->grabbedActions.end() && it.key() == action) {
            if (it.value() == staleGrab)
                it = d->grabbedActions.erase(it);
            else
                ++it;
        }

        d->impl->grabKey(staleGrab, false);
    }

    foreach (int newGrab, needToGrab) {
        d->grabbedKeys.insert(newGrab, action);
        d->grabbedActions.insert(action, newGrab);
        d->impl->grabKey(newGrab, true);
    }
}

bool KGlobalAccel::keyPressed( int key )
{
    bool consumed = false;
    foreach (KAction* action, d->grabbedKeys.values(key)) {
        consumed = true;
        action->trigger();
    }
    return consumed;
}

bool KGlobalAccel::isHandled( int key )
{
    return !d->grabbedKeys.values(key).isEmpty();
}

void KGlobalAccel::regrabKeys( )
{
    QMutableHashIterator<int, KAction*> it2 = d->grabbedKeys;
    while (it2.hasNext()) {
        it2.next();
        if (!d->impl->grabKey(it2.key(), true)) {
            QMultiHash<KAction*, int>::Iterator it = d->grabbedActions.find(it2.value());
            while (it != d->grabbedActions.end() && it.key() == it2.value()) {
                if (it.value() == it2.key()) {
                    d->grabbedActions.erase(it);
                    break;
                }
                ++it;
            }

            it2.remove();
        }
    }
}

void KGlobalAccel::enableImpl( bool enable )
{
    if (d->implEnabled != enable) {
        d->implEnabled = enable;
        d->impl->setEnabled(enable);
    }
}


//static
QStringList KGlobalAccel::findActionNameSystemwide(const QKeySequence &seq)
{
    return self()->d->systemWideGlobalShortcuts.value(seq);
}


//static
bool KGlobalAccel::promptStealShortcutSystemwide(QWidget *parent, const QStringList &actionIdentifier, const QKeySequence &seq)
{
    QString title = i18n("Conflict with Global Shortcut");
    QString message = i18n("The '%1' key combination has already been allocated "
                           "to the global action \"%2\" in %3.\n"
                           "Do you want to reassign it from that action to the current one?",
                           seq.toString(), actionIdentifier.at(1), actionIdentifier.at(0));

    return KMessageBox::warningContinueCancel(parent, message, title, KGuiItem(i18n("Reassign")))
           == KMessageBox::Continue;
}


//static
void KGlobalAccel::stealShortcutSystemwide(const QKeySequence &seq)
{
    //TODO: mainly notification work. Use DBUS. The other side is already implemented.
    QStringList shortcutIdentifier = self()->d->systemWideGlobalShortcuts.value(seq);
    //TODO: something like this... self()->d->dbus.requestTheft(seq);
}

#include "kglobalaccel.moc"
