/*
    This file is part of the KDE libraries

    Copyright (c) 2007 Bernhard Loos <nhuh.put@web.de>
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

#include <kkernel_win.h>

#include <QString>
#include <QStringList>
#include <QtCore/QDir>

inline static bool isMetaChar(ushort c)
{
    static const uchar iqm[] = {
        0x00, 0x00, 0x00, 0x00, 0x40, 0x03, 0x00, 0x50,
        0x01, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x10
    }; // &<>()@^|

    return (c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7)));
}

inline static bool isSpecialChar(ushort c)
{
    static const uchar iqm[] = {
        0xff, 0xff, 0xff, 0xff, 0xc3, 0x1b, 0x00, 0x78, 
        0x01, 0x00, 0x00, 0x68, 0x01, 0x00, 0x00, 0x78
    }; // 0-32 &()[]{}^=;!'+,`~<>@|

    return (c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7)));
}

inline static bool isWhiteSpace(ushort c)
{
    return c == ' ' || c == '\t';
}

QStringList KShell::splitArgs(const QString &args, Options flags, Errors *err)
{
    QStringList ret;

    if (err)
        *err = NoError;

    const QLatin1Char bs('\\'), dq('\"');

    int p = 0;
    const int length = args.length();
    forever {
        while (p < length && isWhiteSpace(args[p].unicode()))
            ++p;
        if (p == length)
            return ret;

        QString arg;
        bool inquote = false;
        forever {
            bool copy = true;
            int bslashes = 0;
            while (p < length && args[p] == bs) {
                ++p;
                ++bslashes;
            }
            if (p < length && args[p] == dq) {
                if (bslashes % 2 == 0) {
                    if (inquote) {
                        if(p + 1 < length && args[p + 1] == dq)
                            ++p;
                        else
                            copy = false;
                    } else {
                        copy = false;
                    }
                    inquote = !inquote;
                }
                bslashes /= 2;
            }

            while (--bslashes >= 0)
                arg.append(bs);

            if (p == length || (!inquote && isWhiteSpace(args[p].unicode()))) {
                ret.append(arg);
                if (inquote) {
                    if (err)
                        *err = BadQuoting;
                    return QStringList();
                }
                break;
            }

            if (copy) {
                if (!inquote && (flags & AbortOnMeta) && isMetaChar(args[p].unicode())) {
                    if (err)
                        *err = FoundMeta;
                    return QStringList();
                }
                arg.append(args[p]);
            }
            ++p;
        }
    }
    //not reached
}

static QString quoteArg(const QString &arg)
{
    // Escape quotes, preceding backslashes are doubled. Surround with quotes.
    // Note that cmd does not understand quote escapes in quoted strings,
    // so the quoting needs to be "suspended".
    const QLatin1Char bs('\\'), dq('\"');
    QString ret;
    bool inquote = false;
    int bslashes = 0;
    for (int p = 0; p < arg.length(); p++) {
        if (arg[p] == bs) {
            bslashes++;
        } else if (arg[p] == dq) {
            if (inquote) {
                ret.append(dq);
                inquote = false;
            }
            for (; bslashes; bslashes--)
                ret.append(QLatin1String("\\\\"));
            ret.append(QLatin1String("\\^\""));
        } else {
            if (!inquote) {
                ret.append(dq);
                inquote = true;
            }
            for (; bslashes; bslashes--)
                ret.append(bs);
            ret.append(arg[p]);
        }
    }
    if (bslashes) {
        // Ensure that we don't have directly trailing backslashes,
        // so concatenating with another string won't cause surprises.
        if (!inquote) {
            ret.append(dq);
            inquote = true;
        }
        for (; bslashes; bslashes--)
            ret.append(QLatin1String("\\\\"));
    }
    if (inquote)
        ret.append(dq);
    return ret;
}

QString KShell::quoteArg(const QString &arg)
{
    if (arg.isEmpty())
        return QString::fromLatin1("\"\"");

    // Ensure that we don't have directly trailing backslashes,
    // so concatenating with another string won't cause surprises.
    if (arg.endsWith(QLatin1Char('\\')))
        return ::quoteArg(arg);

    for (int x = arg.length() - 1; x >= 0; --x)
        if (isSpecialChar(arg[x].unicode()))
            return ::quoteArg(arg);

    // Escape quotes. Preceding backslashes are doubled.
    QString ret(arg);
    ret.replace(QRegExp(QLatin1String("(\\\\*)\"")), QLatin1String("\\1\\1\\^\""));
    return ret;
}

