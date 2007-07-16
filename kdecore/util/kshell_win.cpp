/*
    This file is part of the KDE libraries

    Copyright (c) 2007 Oswald Buddenhagen <ossi@kde.org>

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

#include "kshell.h"

#include <QString>
#include <QStringList>

QStringList KShell::splitArgs(const QString &args, Options flags, Errors *err)
{
    if (err)
        *err = NoError;

    return QStringList();
}

QString KShell::quoteArg(const QString &arg)
{
    if (arg.isEmpty())
        return QString::fromLatin1("\"\"");

    QString ret(arg);

    return ret;
}
