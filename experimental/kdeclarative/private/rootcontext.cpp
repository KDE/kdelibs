/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "rootcontext_p.h"

#include <QDebug>

#include <klocalizedstring.h>

RootContext::RootContext(QObject *parent)
  : QObject(parent)
{
}

RootContext::~RootContext()
{
}

QString RootContext::i18n(const QString &message, const QString &param1, const QString &param2, const QString &param3, const QString &param4, const QString &param5, const QString &param6, const QString &param7, const QString &param8, const QString &param9, const QString &param10) const
{
    if (message.isNull()) {
        qWarning() << "i18n() needs at least one parameter";
        return QString();
    }

    KLocalizedString trMessage = ki18n(message.toUtf8().constData());

    if (!param1.isNull()) {
        trMessage = trMessage.subs(param1);
    }
    if (!param2.isNull()) {
        trMessage = trMessage.subs(param2);
    }
    if (!param3.isNull()) {
        trMessage = trMessage.subs(param3);
    }
    if (!param4.isNull()) {
        trMessage = trMessage.subs(param4);
    }
    if (!param5.isNull()) {
        trMessage = trMessage.subs(param5);
    }
    if (!param6.isNull()) {
        trMessage = trMessage.subs(param6);
    }
    if (!param7.isNull()) {
        trMessage = trMessage.subs(param7);
    }
    if (!param8.isNull()) {
        trMessage = trMessage.subs(param8);
    }
    if (!param9.isNull()) {
        trMessage = trMessage.subs(param9);
    }
    if (!param10.isNull()) {
        trMessage = trMessage.subs(param10);
    }

    return trMessage.toString();
}


QString RootContext::i18nc(const QString &context, const QString &message, const QString &param1, const QString &param2, const QString &param3, const QString &param4, const QString &param5, const QString &param6, const QString &param7, const QString &param8, const QString &param9, const QString &param10) const
{
    if (context.isNull() || message.isNull()) {
        qWarning() << "i18nc() needs at least two arguments";
        return QString();
    }

    KLocalizedString trMessage = ki18nc(context.toUtf8().constData(), message.toUtf8().constData());

    if (!param1.isNull()) {
        trMessage = trMessage.subs(param1);
    }
    if (!param2.isNull()) {
        trMessage = trMessage.subs(param2);
    }
    if (!param3.isNull()) {
        trMessage = trMessage.subs(param3);
    }
    if (!param4.isNull()) {
        trMessage = trMessage.subs(param4);
    }
    if (!param5.isNull()) {
        trMessage = trMessage.subs(param5);
    }
    if (!param6.isNull()) {
        trMessage = trMessage.subs(param6);
    }
    if (!param7.isNull()) {
        trMessage = trMessage.subs(param7);
    }
    if (!param8.isNull()) {
        trMessage = trMessage.subs(param8);
    }
    if (!param9.isNull()) {
        trMessage = trMessage.subs(param9);
    }
    if (!param10.isNull()) {
        trMessage = trMessage.subs(param10);
    }

    return trMessage.toString();
}

QString RootContext::i18np(const QString &singular, const QString &plural, const QString &param1, const QString &param2, const QString &param3, const QString &param4, const QString &param5, const QString &param6, const QString &param7, const QString &param8, const QString &param9, const QString &param10) const
{
    if (singular.isNull() || plural.isNull()) {
        qWarning() << "i18np() needs at least two arguments";
        return QString();
    }

    KLocalizedString trMessage = ki18np(singular.toUtf8().constData(), plural.toUtf8().constData());

    if (!param1.isNull()) {
        trMessage = trMessage.subs(param1);
    }
    if (!param2.isNull()) {
        trMessage = trMessage.subs(param2);
    }
    if (!param3.isNull()) {
        trMessage = trMessage.subs(param3);
    }
    if (!param4.isNull()) {
        trMessage = trMessage.subs(param4);
    }
    if (!param5.isNull()) {
        trMessage = trMessage.subs(param5);
    }
    if (!param6.isNull()) {
        trMessage = trMessage.subs(param6);
    }
    if (!param7.isNull()) {
        trMessage = trMessage.subs(param7);
    }
    if (!param8.isNull()) {
        trMessage = trMessage.subs(param8);
    }
    if (!param9.isNull()) {
        trMessage = trMessage.subs(param9);
    }
    if (!param10.isNull()) {
        trMessage = trMessage.subs(param10);
    }

    return trMessage.toString();
}

QString RootContext::i18ncp(const QString &context, const QString &singular, const QString &plural, const QString &param1, const QString &param2, const QString &param3, const QString &param4, const QString &param5, const QString &param6, const QString &param7, const QString &param8, const QString &param9, const QString &param10) const
{
    if (context.isNull() || singular.isNull() || plural.isNull()) {
        qWarning() << "i18ncp() needs at least three arguments";
        return QString();
    }

    KLocalizedString trMessage = ki18ncp(context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());

    if (!param1.isNull()) {
        trMessage = trMessage.subs(param1);
    }
    if (!param2.isNull()) {
        trMessage = trMessage.subs(param2);
    }
    if (!param3.isNull()) {
        trMessage = trMessage.subs(param3);
    }
    if (!param4.isNull()) {
        trMessage = trMessage.subs(param4);
    }
    if (!param5.isNull()) {
        trMessage = trMessage.subs(param5);
    }
    if (!param6.isNull()) {
        trMessage = trMessage.subs(param6);
    }
    if (!param7.isNull()) {
        trMessage = trMessage.subs(param7);
    }
    if (!param8.isNull()) {
        trMessage = trMessage.subs(param8);
    }
    if (!param9.isNull()) {
        trMessage = trMessage.subs(param9);
    }
    if (!param10.isNull()) {
        trMessage = trMessage.subs(param10);
    }

    return trMessage.toString();
}

#include <moc_rootcontext_p.cpp>
