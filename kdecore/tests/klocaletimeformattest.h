/*
 * This file is part of the KDE libraries.
 * Copyright 2009  Michael Leupold <lemma@confuego.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KLOCALETIMEFORMATTEST_H
#define KLOCALETIMEFORMATTEST_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTime>
#include <QtCore/QMap>
#include <klocale.h>

struct TimeFormatTestValue
{
    QString timeFormat; // time format string
    // expected values for the various format options
    QMap<KLocale::TimeFormatOptions, QString> exp;
};

class KLocaleTimeFormatTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void formatTimeTest();
    void readTimeTest();

    void formatAndReadAllCountriesTest();

private:
    QMap<QString, TimeFormatTestValue> m_timeFormats;
    QMap<QString, QString> m_allFormats;
};

#endif

