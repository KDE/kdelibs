/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "phononnamespace.h"
#include "phononnamespace_p.h"

#include "factory.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QList>

namespace Phonon
{
    QString categoryToString(Category c)
    {
        switch(c)
        {
        case Phonon::NotificationCategory:
            return QCoreApplication::translate("Phonon::", "Notifications");
        case Phonon::MusicCategory:
            return QCoreApplication::translate("Phonon::", "Music");
        case Phonon::VideoCategory:
            return QCoreApplication::translate("Phonon::", "Video");
        case Phonon::CommunicationCategory:
            return QCoreApplication::translate("Phonon::", "Communication");
        case Phonon::GameCategory:
            return QCoreApplication::translate("Phonon::", "Games");
        case Phonon::AccessibilityCategory:
            return QCoreApplication::translate("Phonon::", "Accessibility");
        }
        return QString();
    }
}

Q_DECLARE_METATYPE(QList<int>)
static int registerPhononMetaTypes()
{
    qRegisterMetaType<Phonon::State>();
    qRegisterMetaType<Phonon::ErrorType>();
    qRegisterMetaType<Phonon::Category>();

    // need those for QSettings
    qRegisterMetaType<QList<int> >();
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");

    return 0; // something
}

#ifdef Q_CONSTRUCTOR_FUNCTION
Q_CONSTRUCTOR_FUNCTION(registerPhononMetaTypes)
#else
static const int _Phonon_registerMetaTypes = registerPhononMetaTypes();
#endif

Phonon::GlobalData::GlobalData()
{
    showDebugOutput = !qgetenv("PHONON_DEBUG").isEmpty();
}

Q_GLOBAL_STATIC(Phonon::GlobalData, globalInstance)

Phonon::GlobalData *Phonon::GlobalData::instance()
{
    return globalInstance();
}

// vim: sw=4 ts=4
