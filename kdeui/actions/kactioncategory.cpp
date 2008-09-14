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

#include "kactioncategory.h"

#include <QtCore/QList>
#include <QtGui/QAction>

#include "kaction.h"
#include "kdebug.h"


struct KActionCategoryPrivate
    {

    KActionCategoryPrivate( KActionCategory *host );

    //! Our host
    KActionCategory *q;

    //! The text for this category
    QString text;

    //! List of actions
    QList<QAction*> actions;

    }; // class KActionCategoryPrivate


KActionCategory::KActionCategory(const QString &text, KActionCollection *parent)
    :   QObject(parent)
        ,d( new KActionCategoryPrivate(this) )
    {
    kDebug() << text;
    d->text = text;
    }


KActionCategory::~KActionCategory()
    {
    kDebug() << d->text;
    delete d; d=0;
    }


QAction * KActionCategory::addAction(const QString &name, QAction *action)
    {
    collection()->addAction(name, action);
    d->actions.append(action);
    return action;
    }


KAction * KActionCategory::addAction(const QString &name, KAction *action)
    {
    collection()->addAction(name, action);
    d->actions.append(action);
    return action;
    }


KAction * KActionCategory::addAction(
        KStandardAction::StandardAction actionType,
        const QObject *receiver,
        const char *member)
    {
    KAction *action = collection()->addAction(actionType, receiver, member);
    d->actions.append(action);
    return action;
    }


KAction * KActionCategory::addAction(
        KStandardAction::StandardAction actionType,
        const QString &name,
        const QObject *receiver,
        const char *member)
    {
    KAction *action = collection()->addAction(actionType, name, receiver, member);
    d->actions.append(action);
    return action;
    }


KAction *KActionCategory::addAction(
        const QString &name,
        const QObject *receiver,
        const char *member)
    {
    KAction *action = collection()->addAction(name, receiver, member);
    d->actions.append(action);
    return action;
    }


KActionCollection * KActionCategory::collection() const
    {
    return qobject_cast<KActionCollection*>(parent());
    }


QString KActionCategory::text() const
    {
    return d->text;
    }


void KActionCategory::setText(const QString &text)
    {
    d->text = text;
    }


KActionCategoryPrivate::KActionCategoryPrivate( KActionCategory *host )
    : q(host)
    {}


#include "moc_kactioncategory.cpp"
