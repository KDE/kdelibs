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

QString KShell::quoteArg(const QString &arg)
{
    if (arg.isEmpty())
        return QString::fromLatin1("\"\"");

    QString ret(arg);
    ret.replace(QLatin1String("\\\""), QLatin1String("\\\\\""));
    ret.replace(QLatin1Char('\"'), QLatin1String("\\\""));

    for (int x = arg.length() - 1; x >= 0; --x) {
        if (isSpecialChar(arg[x].unicode())) {

            ret.prepend(QLatin1Char('\"'));

            if (ret.endsWith(QLatin1Char('\\'))) {
                ret.truncate(ret.length() - 1);
                ret.append(QLatin1String("\"\\"));
            }
            else {
                ret.append(QLatin1Char('\"'));
            }

            break;
        }
    }

    return ret;
}

