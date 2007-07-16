/*
    This file is part of the KDE libraries

    Copyright (c) 2003 Oswald Buddenhagen <ossi@kde.org>

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
#include "kuser.h"

#include <QtCore/QDir>

static QString homeDir( const QString &user )
{
    if (user.isEmpty())
        return QDir::homePath();
    return KUser(user).homeDir();
}

QString KShell::joinArgs( const QStringList &args )
{
    QString ret;
    for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it) {
        if (!ret.isEmpty())
            ret.append(QLatin1Char(' '));
        ret.append(quoteArg(*it));
    }
    return ret;
}

QString KShell::tildeExpand( const QString &fname )
{
    if (fname.length() && fname[0] == QLatin1Char('~')) {
        int pos = fname.indexOf( QLatin1Char('/') );
        if (pos < 0)
            return homeDir( fname.mid(1) );
        QString ret = homeDir( fname.mid(1, pos-1) );
        if (!ret.isNull())
            ret += fname.mid(pos);
        return ret;
    }
    return fname;
}

bool KShell::matchFileName( const QString &filename, const QString &pattern )
{
    int len = filename.length();
    int pattern_len = pattern.length();

    if (!pattern_len)
        return false;

    // Patterns like "Makefile*"
    if (pattern[pattern_len - 1] == '*' && len + 1 >= pattern_len) {
        if (pattern[0] == '*')
            return filename.indexOf(pattern.mid(1, pattern_len - 2)) != -1;

        const QChar *c1 = pattern.unicode();
        const QChar *c2 = filename.unicode();
        int cnt = 1;
        while (cnt < pattern_len && *c1++ == *c2++)
           ++cnt;
        return cnt == pattern_len;
    }

    // Patterns like "*~", "*.extension"
    if (pattern[0] == '*' && len + 1 >= pattern_len)
    {
        const QChar *c1 = pattern.unicode() + pattern_len - 1;
        const QChar *c2 = filename.unicode() + len - 1;
        int cnt = 1;
        while (cnt < pattern_len && *c1-- == *c2--)
            ++cnt;
        return cnt == pattern_len;
    }

    // Patterns like "Makefile"
    return filename == pattern;
}
