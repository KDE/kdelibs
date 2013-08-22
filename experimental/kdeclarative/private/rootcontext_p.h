/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
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

#ifndef ROOTCONTEXT_H
#define ROOTCONTEXT_H

#include <QObject>

class RootContext : public QObject
{
    Q_OBJECT

public:
    RootContext(QObject *parent = 0);
    ~RootContext();

    Q_INVOKABLE QString i18n(const QString &message, const QString &param1 = QString(), const QString &param2 = QString(), const QString &param3 = QString(), const QString &param4 = QString(), const QString &param5 = QString(), const QString &param6 = QString(), const QString &param7 = QString(), const QString &param8 = QString(), const QString &param9 = QString(), const QString &param10 = QString()) const;
};

#endif
