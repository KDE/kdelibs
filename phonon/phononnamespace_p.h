/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONONNAMESPACE_P_H
#define PHONONNAMESPACE_P_H

#include <QtCore/QDebug>
#include "phonon_export.h"

namespace Phonon
{
    struct PHONON_EXPORT GlobalData
    {
        GlobalData();

        bool showDebugOutput;

        static GlobalData *instance();
    };
}

#if defined(QT_NO_DEBUG)
#define pDebug qDebug
#define pError() qDebug()
#else
#define pDebug if (!Phonon::GlobalData::instance()->showDebugOutput) {} else qDebug
#define pError() if (!Phonon::GlobalData::instance()->showDebugOutput) {} else qDebug() << "ERROR:"
#endif

// vim: sw=4 ts=4 tw=80
#endif // PHONONNAMESPACE_P_H
