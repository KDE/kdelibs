/*  This file is part of the KDE project
    Copyright (c) 1997,2001 Stephan Kulow <coolo@kde.org>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 1999-2002 Hans Petter Bieker <bieker@kde.org>
    Copyright (c) 2002 Lukas Tinkl <lukas@kde.org>
    Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>
    Copyright (C) 2009, 2010 John Layt <john@layt.net>

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

#ifndef KJOBTRACKERFORMATTERS_P_H
#define KJOBTRACKERFORMATTERS_P_H

#include <QString>

namespace KJobTrackerFormatters
{
    QString byteSize(double size);

    QString daysDuration(int n);
    QString hoursDuration(int n);
    QString minutesDuration(int n);
    QString secondsDuration(int n);
    QString duration(unsigned long mSec);
};

#endif
