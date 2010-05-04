/* This file is part of the KDE libraries
   Copyright 2010 Canonical Ltd
   Author: Aurélien Gâteau <aurelien.gateau@canonical.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "kconfigutils.h"

// KDE
#include <kconfig.h>
#include <kconfiggroup.h>

namespace KConfigUtils
{

bool hasGroup(KConfig *config, const QStringList &lst)
{
    KConfigGroup group = openGroup(config, lst);
    return group.exists();
}

KConfigGroup openGroup(KConfig *config, const QStringList &_lst)
{
    if (_lst.isEmpty()) {
        return KConfigGroup(config, QString());
    }

    QStringList lst = _lst;

    KConfigGroup cg;
    for (cg = KConfigGroup(config, lst.takeFirst()); !lst.isEmpty(); cg = KConfigGroup(&cg, lst.takeFirst())) {}
    return cg;
}

QStringList parseGroupString(const QString &_str, bool *ok, QString *error)
{
    QString str = unescapeString(_str.trimmed(), ok, error);
    if (!ok) {
        return QStringList();
    }

    *ok = true;
    if (str[0] != '[') {
        // Simplified notation, no '['
        return QStringList() << str;
    }

    if (!str.endsWith(']')) {
        *ok = false;
        *error = QString("Missing closing ']' in %1").arg(_str);
        return QStringList();
    }
    // trim outer brackets
    str.chop(1);
    str.remove(0, 1);

    return str.split("][");
}

QString unescapeString(const QString &src, bool *ok, QString *error)
{
    QString dst;
    int length = src.length();
    for (int pos = 0; pos < length; ++pos) {
        QChar ch = src.at(pos);
        if (ch != '\\') {
            dst += ch;
        } else {
            ++pos;
            if (pos == length) {
                *ok = false;
                *error = QString("Unfinished escape sequence in %1").arg(src);
                return QString();
            }
            ch = src.at(pos);
            if (ch == 's') {
                dst += ' ';
            } else if (ch == 't') {
                dst += '\t';
            } else if (ch == 'n') {
                dst += '\n';
            } else if (ch == 'r') {
                dst += '\r';
            } else if (ch == '\\') {
                dst += '\\';
            } else if (ch == 'x') {
                if (pos + 2 < length) {
                    char value = src.mid(pos + 1, 2).toInt(ok, 16);
                    if (*ok) {
                        dst += QChar::fromAscii(value);
                        pos += 2;
                    } else {
                        *error = QString("Invalid hex escape sequence at column %1 in %2").arg(pos).arg(src);
                        return QString();
                    }
                } else {
                    *ok = false;
                    *error = QString("Unfinished hex escape sequence at column %1 in %2").arg(pos).arg(src);
                    return QString();
                }
            } else {
                *ok = false;
                *error = QString("Invalid escape sequence at column %1 in %2").arg(pos).arg(src);
                return QString();
            }
        }
    }

    *ok = true;
    return dst;
}

} // namespace
