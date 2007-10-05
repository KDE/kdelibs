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
#include "kdedglobalaccel.h"

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
#include <kconfig.h>
#include <kconfiggroup.h>
#include "kaction.h"
#include "kaction_p.h"
#include "kactioncollection.h"
#include "kmessagebox.h"
#include "kshortcut.h"


//TODO what was the problem that got fixed recently in the old version? - forward port if necessary

KGlobalAccelPrivate::KGlobalAccelPrivate()
     : isUsingForeignComponentName(false),
       enabled(true),
       iface("org.kde.kded", "/modules/kdedglobalaccel", QDBusConnection::sessionBus())
{
    // Make sure kded is running
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kded")) {
        KToolInvocation::klauncher(); // this calls startKdeinit
    }
}

KGlobalAccel::KGlobalAccel()
    : d(new KGlobalAccelPrivate)
{
    qDBusRegisterMetaType<QList<int> >();

    connect(&d->iface, SIGNAL(invokeAction(const QStringList &)),
            SLOT(_k_invokeAction(const QStringList &)));
    connect(&d->iface, SIGNAL(yourShortcutGotChanged(const QStringList &, const QList<int> &)),
            SLOT(_k_shortcutGotChanged(const QStringList &, const QList<int> &)));

    if (KGlobal::hasMainComponent())
        d->mainComponentName = KGlobal::mainComponent().componentName();
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

void KGlobalAccel::setEnabled( bool enabled )
{
    d->enabled = enabled;

//TODO: implement this in KdedGlobalAccel... or not at all
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
    d->mainComponentName = kcd.componentName();
    d->isUsingForeignComponentName = true;
}


KGlobalAccel *KGlobalAccel::self( )
{
    K_GLOBAL_STATIC(KGlobalAccel, s_instance)
    return s_instance;
}


void KGlobalAccelPrivate::updateGlobalShortcutAllowed(KAction *action, uint flags)
{
    if (!action)
        return;

    bool oldEnabled = actionToName.contains(action);
    bool newEnabled = action->globalShortcutAllowed();

    if (oldEnabled == newEnabled)
        return;

    if (action->text().isEmpty())
        return;
    QStringList actionId(mainComponentName);
    actionId.append(action->text());
    //TODO: what about i18ned names?

    if (!oldEnabled && newEnabled) {
        uint setterFlags = KdedGlobalAccel::SetPresent;
        if (flags & KAction::NoAutoloading)
            setterFlags |= KdedGlobalAccel::NoAutoloading;
        if (action->globalShortcut(KAction::DefaultShortcut).isEmpty())
            setterFlags |= KdedGlobalAccel::IsDefaultEmpty;

        nameToAction.insert(actionId.at(1), action);
        actionToName.insert(action, actionId.at(1));
        QList<int> result = iface.setShortcut(actionId,
                                              intListFromShortcut(action->globalShortcut()),
                                              setterFlags);
        KShortcut scResult(shortcutFromIntList(result));

        if (scResult != action->globalShortcut())
            action->d->setActiveGlobalShortcutNoEnable(scResult);
    }

    if (oldEnabled && !newEnabled) {
        nameToAction.remove(actionToName.take(action));
        iface.setInactive(actionId);
    }
}


void KGlobalAccelPrivate::updateGlobalShortcut(KAction *action, uint flags)
{
    if (!action)
        return;

    if (action->text().isEmpty())
        return;
    QStringList actionId(mainComponentName);
    actionId.append(action->text());
    //TODO: what about i18ned names?

    uint setterFlags = 0;
    if (flags & KAction::NoAutoloading)
        setterFlags |= KdedGlobalAccel::NoAutoloading;
    if (action->globalShortcut(KAction::DefaultShortcut).isEmpty())
        setterFlags |= KdedGlobalAccel::IsDefaultEmpty;

    QList<int> result = iface.setShortcut(actionId,
                                          intListFromShortcut(action->globalShortcut()),
                                          setterFlags);
    KShortcut scResult(shortcutFromIntList(result));

    if (scResult != action->globalShortcut()) {
        action->d->setActiveGlobalShortcutNoEnable(scResult);
    }

    //We might be able to avoid that call sometimes, but it's neither worth the effort nor
    //the bytes to determine the cases where it's safe to avoid it.
    if (isUsingForeignComponentName) {
        iface.setForeignShortcut(actionId, result);
    }
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


void KGlobalAccelPrivate::_k_invokeAction(const QStringList &actionId)
{
    //TODO: can we make it so that we don't have to check the mainComponentName? (i.e. targeted signals)
    if (actionId.at(0) != mainComponentName || isUsingForeignComponentName)
        return;

    KAction *action = nameToAction.value(actionId.at(1));
    if (!action)
        return;

#ifdef Q_WS_X11
    // Update this application's X timestamp if needed.
    // TODO The 100%-correct solution should probably be handling this action
    // in the proper place in relation to the X events queue in order to avoid
    // the possibility of wrong ordering of user events.
    Time timestamp = actionId.at( 2 ).toULong();
    if( NET::timestampCompare( timestamp, QX11Info::appTime()) > 0 )
        QX11Info::setAppTime( timestamp );
    if( NET::timestampCompare( timestamp, QX11Info::appUserTime()) > 0 )
        QX11Info::setAppUserTime( timestamp );
#endif

    action->trigger();
}

void KGlobalAccelPrivate::_k_shortcutGotChanged(const QStringList &actionId,
                                                const QList<int> &keys)
{
    KAction *action = nameToAction.value(actionId.at(1));
    if (!action)
        return;

    action->d->setActiveGlobalShortcutNoEnable(shortcutFromIntList(keys));
}


//static
QStringList KGlobalAccel::findActionNameSystemwide(const QKeySequence &seq)
{
    return self()->d->iface.action(seq[0]);
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
    //get the shortcut, remove &seq, and set the new shorctut
    QStringList actionId = self()->d->iface.action(seq[0]);
    QList<int> sc = self()->d->iface.shortcut(actionId);

    for (int i = 0; i < sc.count(); i++)
        if (sc[i] == seq[0])
            sc[i] = 0;

    self()->d->iface.setForeignShortcut(actionId, sc);
}

#include "kglobalaccel.moc"
#include "kdedglobalaccel_interface.moc"
