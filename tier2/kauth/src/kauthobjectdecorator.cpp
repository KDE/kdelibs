/* This file is part of the KDE libraries
    Copyright (C) 2009-2012 Dario Freddi <drf@kde.org>

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

#include "kauthobjectdecorator.h"

#include "kauthaction.h"
#include "kauthexecutejob.h"

#include <QAbstractButton>
#include <QAction>
#include <QDebug>
#include <QIcon>

namespace KAuth
{

class ObjectDecoratorPrivate
{
public:
    ObjectDecoratorPrivate(ObjectDecorator *parent)
        : q(parent),
          decoratedObject(parent->parent())
    {
    }

    ObjectDecorator * const q;

    QObject * const decoratedObject;
    KAuth::Action authAction;
    // TODO: Remove whenever QIcon overlays will get fixed
    QIcon oldIcon;

    void connectDecorated();
    void linkActionToWidget();
    void slotActivated();
    void authStatusChanged(KAuth::Action::AuthStatus status);
};

void ObjectDecoratorPrivate::connectDecorated()
{
    if (qobject_cast<QAbstractButton*>(decoratedObject)) {
        q->connect(decoratedObject, SIGNAL(clicked()),
                   q, SLOT(slotActivated()));
        return;
    }

    if (qobject_cast<QAction*>(decoratedObject)) {
        q->connect(decoratedObject, SIGNAL(triggered(bool)),
                   q, SLOT(slotActivated()));
        return;
    }

    qWarning() << Q_FUNC_INFO << "We're not decorating an action or a button";
}

void ObjectDecoratorPrivate::linkActionToWidget()
{
    QWidget *widget = qobject_cast<QWidget*>(decoratedObject);
    if (widget) {
        authAction.setParentWidget(widget);
        return;
    }

    QAction *action = qobject_cast<QAction*>(decoratedObject);
    if (action) {
        authAction.setParentWidget(action->parentWidget());
        return;
    }

    qWarning() << Q_FUNC_INFO << "We're not decorating an action or a widget";
}

void ObjectDecoratorPrivate::slotActivated()
{
    if (authAction.isValid()) {
        KAuth::ExecuteJob *job = authAction.execute(KAuth::Action::AuthorizeOnlyMode);
        q->connect(job, SIGNAL(statusChanged(KAuth::Action::AuthStatus)),
                   q, SLOT(authStatusChanged(KAuth::Action::AuthStatus)));
        if (job->exec()) {
            emit q->authorized(authAction);
        } else {
            decoratedObject->setProperty("enabled", false);
        }
    }
}

void ObjectDecoratorPrivate::authStatusChanged(KAuth::Action::AuthStatus status)
{
    switch (status) {
    case KAuth::Action::AuthorizedStatus:
        decoratedObject->setProperty("enabled", true);
        if(!oldIcon.isNull()) {
            decoratedObject->setProperty("icon", QVariant::fromValue(oldIcon));
            oldIcon = QIcon();
        }
        break;
    case KAuth::Action::AuthRequiredStatus:
        decoratedObject->setProperty("enabled", true);
        oldIcon = decoratedObject->property("icon").value<QIcon>();
        decoratedObject->setProperty("icon", QIcon::fromTheme(QLatin1String("dialog-password")));
        break;
    default:
        decoratedObject->setProperty("enabled", false);
        if(!oldIcon.isNull()) {
            decoratedObject->setProperty("icon", QVariant::fromValue(oldIcon));
            oldIcon = QIcon();
        }
    }
}


ObjectDecorator::ObjectDecorator( QObject *parent )
    : QObject( parent ), d( new ObjectDecoratorPrivate(this) )
{
    d->connectDecorated();
}

ObjectDecorator::~ObjectDecorator()
{
    delete d;
}

KAuth::Action ObjectDecorator::authAction() const
{
    return d->authAction;
}

void ObjectDecorator::setAuthAction(const QString &actionName)
{
    if (actionName.isEmpty()) {
        setAuthAction(KAuth::Action());
    } else {
        setAuthAction(KAuth::Action(actionName));
    }
}

void ObjectDecorator::setAuthAction(const KAuth::Action &action)
{
    if (d->authAction == action) {
        return;
    }

    if (d->authAction.isValid()) {
        if (!d->oldIcon.isNull()) {
            d->decoratedObject->setProperty("icon", QVariant::fromValue(d->oldIcon));
            d->oldIcon = QIcon();
        }
    }

    if (action.isValid()) {
        d->authAction = action;

        // Set the parent widget
        d->linkActionToWidget();

        d->authStatusChanged(d->authAction.status());
    }
}

} // namespace KAuth

#include "moc_kauthobjectdecorator.cpp"
