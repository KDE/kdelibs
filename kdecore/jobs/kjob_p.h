/*  This file is part of the KDE project
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef KJOB_P_H
#define KJOB_P_H

#include "kjob.h"
#include <QMap>
#include <QMetaType>

class KJobUiDelegate;
class QTimer;

class KJobPrivate
{
public:
    KJobPrivate()
        : q_ptr(0), uiDelegate(0), error(KJob::NoError),
          progressUnit(KJob::Bytes), percentage(0),
          suspended(false), capabilities(KJob::NoCapabilities),
          speedTimer(0)
    {
        if (!_k_kjobUnitEnumRegistered) {
            _k_kjobUnitEnumRegistered = qRegisterMetaType<KJob::Unit>("KJob::Unit");
        }
    }

    KJob *q_ptr;

    KJobUiDelegate *uiDelegate;
    int error;
    QString errorText;
    KJob::Unit progressUnit;
    QMap<KJob::Unit, qulonglong> processedAmount;
    QMap<KJob::Unit, qulonglong> totalAmount;
    unsigned long percentage;
    bool suspended;
    KJob::Capabilities capabilities;
    QTimer *speedTimer;

    void _k_speedTimeout();

    static bool _k_kjobUnitEnumRegistered;

    Q_DECLARE_PUBLIC(KJob)
};

#endif
