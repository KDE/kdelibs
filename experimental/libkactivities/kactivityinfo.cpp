/*
 * Copyright (c) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kactivityinfo.h"
#include "kactivityinfo_p.h"

#include <kdebug.h>

#include "kactivitymanager_p.h"

// Private

KActivityInfo::Private::Private(KActivityInfo *info, const QString &activityId)
    : q(info),
      state(KActivityInfo::Invalid),
      id(activityId)
{
    KActivityManager::self();
}

#define IMPLEMENT_SIGNAL_HANDLER(ORIGINAL, INTERNAL) \
    void KActivityInfo::Private::INTERNAL(const QString & _id) const  \
    {                                                                 \
        if (id == _id) emit q->INTERNAL();                            \
    }

IMPLEMENT_SIGNAL_HANDLER(ActivityAdded,   added)
IMPLEMENT_SIGNAL_HANDLER(ActivityRemoved, removed)
IMPLEMENT_SIGNAL_HANDLER(ActivityStarted, started)
IMPLEMENT_SIGNAL_HANDLER(ActivityStopped, stopped)
IMPLEMENT_SIGNAL_HANDLER(ActivityChanged, infoChanged)

#undef IMPLEMENT_SIGNAL_HANDLER

void KActivityInfo::Private::activityStateChanged(const QString & idChanged, int newState)
{
    if (idChanged == id) {
        state = static_cast<KActivityInfo::State>(newState);
        emit q->stateChanged(state);
    }
}

// KActivityInfo
KActivityInfo::KActivityInfo(const QString &activityId, QObject *parent)
    : QObject(parent),
      d(new Private(this, activityId))
{
    d->id = activityId;
    connect(KActivityManager::self(), SIGNAL(ActivityStateChanged(const QString &, int)),
            this, SLOT(activityStateChanged(const QString &, int)));

    connect(KActivityManager::self(), SIGNAL(ActivityChanged(const QString &)),
            this, SLOT(infoChanged(const QString &)));

    connect(KActivityManager::self(), SIGNAL(ActivityAdded(const QString &)),
            this, SLOT(added(const QString &)));

    connect(KActivityManager::self(), SIGNAL(ActivityRemoved(const QString &)),
            this, SLOT(removed(const QString &)));

    connect(KActivityManager::self(), SIGNAL(ActivityStarted(const QString &)),
            this, SLOT(started(const QString &)));

    connect(KActivityManager::self(), SIGNAL(ActivityStopped(const QString &)),
            this, SLOT(stopped(const QString &)));
}

KActivityInfo::~KActivityInfo()
{
    delete d;
}

bool KActivityInfo::isValid() const
{
    return (state() != Invalid);
}

// macro defines a shorthand for validating and returning a d-bus result
// @param REPLY_TYPE type of the d-bus result
// @param CAST_TYPE type to which to cast the result
// @param METHOD invocation of the d-bus method
#define KACTIVITYINFO_DBUS_CAST_RETURN(REPLY_TYPE, CAST_TYPE, METHOD)  \
    QDBusReply < REPLY_TYPE > dbusReply = METHOD;                      \
    if (dbusReply.isValid()) {                                         \
        return (CAST_TYPE)(dbusReply.value());                         \
    } else {                                                           \
        return CAST_TYPE();                                            \
    }


KUrl KActivityInfo::uri() const
{
    // TODO:
    return KUrl();
    // KACTIVITYINFO_DBUS_CAST_RETURN(
    //     QString, KUrl, Private::s_store->uri(d->id));
}

KUrl KActivityInfo::resourceUri() const
{
    // TODO:
    return KUrl();
    // KACTIVITYINFO_DBUS_CAST_RETURN(
    //     QString, KUrl, Private::s_store->resourceUri(d->id));
}

QString KActivityInfo::id() const
{
    return d->id;
}

QString KActivityInfo::name() const
{
    KACTIVITYINFO_DBUS_CAST_RETURN(
        QString, QString, KActivityManager::self()->ActivityName(d->id));
}

QString KActivityInfo::icon() const
{
    KACTIVITYINFO_DBUS_CAST_RETURN(
        QString, QString, KActivityManager::self()->ActivityIcon(d->id));
}

KActivityInfo::State KActivityInfo::state() const
{
    if (d->state == Invalid) {
        QDBusReply < int > dbusReply = KActivityManager::self()->ActivityState(d->id);

        if (dbusReply.isValid()) {
            d->state = (State)(dbusReply.value());
        }
    }

    return d->state;
}

QString KActivityInfo::name(const QString & id)
{
    KACTIVITYINFO_DBUS_CAST_RETURN(
            QString, QString, KActivityManager::self()->ActivityName(id));
}

#undef KACTIVITYINFO_DBUS_CAST_RETURN

KActivityInfo::Availability KActivityInfo::availability() const
{
    Availability result = Nothing;

    if (!KActivityManager::isActivityServiceRunning()) {
        return result;
    }

    if (KActivityManager::self()->ListActivities().value().contains(d->id)) {
        result = BasicInfo;

        if (KActivityManager::self()->IsBackstoreAvailable()) {
            result = Everything;
        }
    }

    return result;
}

#include "kactivityinfo.moc"

