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
#include <QtCore/QFile>

static QString homeDir( const QString &user )
{
    if (user.isEmpty())
        return QDir::homePath();
    return KUser(user).homeDir();
}

static int fromHex( QChar cUnicode )
{
    char c = cUnicode.toAscii ();

    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

inline static bool isQuoteMeta( QChar cUnicode )
{
#if 0 // it's not worth it, especially after seeing gcc's asm output ...
    static const uchar iqm[] = {
        0x00, 0x00, 0x00, 0x00, 0x94, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00
    }; // \'"$

    return (c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7)));
#else
    char c = cUnicode.toAscii();
    return c == '\\' || c == '\'' || c == '"' || c == '$';
#endif
}

inline static bool isMeta( QChar cUnicode )
{
    static const uchar iqm[] = {
        0x00, 0x00, 0x00, 0x00, 0xdc, 0x07, 0x00, 0xd8,
        0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x38
    }; // \'"$`<>|;&(){}*?#

    uint c = cUnicode.unicode();

    return (c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7)));
}

QStringList KShell::splitArgs( const QString &args, Options flags, Errors *err )
{
    QStringList ret;
    bool firstword = flags & AbortOnMeta;

    for (int pos = 0; ; ) {
        QChar c;
        do {
            if (pos >= args.length())
                goto okret;
            c = args.unicode()[pos++];
        } while (c.isSpace());
        QString cret;
        if ((flags & TildeExpand) && c == QLatin1Char('~')) {
            int opos = pos;
            for (; ; pos++) {
                if (pos >= args.length())
                    break;
                c = args.unicode()[pos];
                if (c == QLatin1Char('/') || c.isSpace())
                    break;
                if (isQuoteMeta( c )) {
                    pos = opos;
                    c = QLatin1Char('~');
                    goto notilde;
                }
                if ((flags & AbortOnMeta) && isMeta( c ))
                    goto metaerr;
            }
            QString ccret = homeDir( args.mid(opos, pos-opos) );
            if (ccret.isEmpty()) {
                pos = opos;
                c = QLatin1Char('~');
                goto notilde;
            }
            if (pos >= args.length()) {
                ret += ccret;
                goto okret;
            }
            pos++;
            if (c.isSpace()) {
                ret += ccret;
                firstword = false;
                continue;
            }
            cret = ccret;
        }
        // before the notilde label, as a tilde does not match anyway
        if (firstword) {
            if (c == QLatin1Char('_') ||
                (c >= QLatin1Char('A') && c <= QLatin1Char('Z')) ||
                (c >= QLatin1Char('a') && c <= QLatin1Char('z')))
            {
                int pos2 = pos;
                QChar cc;
                do
                    cc = args[pos2++];
                while (cc == QLatin1Char('_') ||
                       (cc >= QLatin1Char('A') && cc <= QLatin1Char('Z')) ||
                       (cc >= QLatin1Char('a') && cc <= QLatin1Char('z')) ||
                       (cc >= QLatin1Char('0') && cc <= QLatin1Char('9')));
                if (cc == QLatin1Char('='))
                    goto metaerr;
            }
        }
      notilde:
        do {
            if (c == QLatin1Char('\'')) {
                int spos = pos;
                do {
                    if (pos >= args.length())
                        goto quoteerr;
                    c = args.unicode()[pos++];
                } while (c != QLatin1Char('\''));
                cret += args.mid(spos, pos-spos-1);
            } else if (c == QLatin1Char('"')) {
                for (;;) {
                    if (pos >= args.length())
                        goto quoteerr;
                    c = args.unicode()[pos++];
                    if (c == QLatin1Char('"'))
                        break;
                    if (c == QLatin1Char('\\')) {
                        if (pos >= args.length())
                            goto quoteerr;
                        c = args.unicode()[pos++];
                        if (c != QLatin1Char('"') &&
                            c != QLatin1Char('\\') &&
                            !((flags & AbortOnMeta) &&
                              (c == QLatin1Char('$') ||
                               c == QLatin1Char('`'))))
                            cret += QLatin1Char('\\');
                    } else if ((flags & AbortOnMeta) &&
                                (c == QLatin1Char('$') ||
                                 c == QLatin1Char('`')))
                        goto metaerr;
                    cret += c;
                }
            } else if (c == QLatin1Char('$') && args[pos] == QLatin1Char('\'')) {
                pos++;
                for (;;) {
                    if (pos >= args.length())
                        goto quoteerr;
                    c = args.unicode()[pos++];
                    if (c == QLatin1Char('\''))
                        break;
                    if (c == QLatin1Char('\\')) {
                        if (pos >= args.length())
                            goto quoteerr;
                        c = args.unicode()[pos++];
                        switch (c.toAscii()) {
                        case 'a': cret += QLatin1Char('\a'); break;
                        case 'b': cret += QLatin1Char('\b'); break;
                        case 'e': cret += QLatin1Char('\033'); break;
                        case 'f': cret += QLatin1Char('\f'); break;
                        case 'n': cret += QLatin1Char('\n'); break;
                        case 'r': cret += QLatin1Char('\r'); break;
                        case 't': cret += QLatin1Char('\t'); break;
                        case '\\': cret += QLatin1Char('\\'); break;
                        case '\'': cret += QLatin1Char('\''); break;
                        case 'c': cret += args[pos++].toAscii() & 31; break;
                        case 'x':
                          {
                            int hv = fromHex( args[pos] );
                            if (hv < 0) {
                                cret += QLatin1String("\\x");
                            } else {
                                int hhv = fromHex( args[++pos] );
                                if (hhv > 0) {
                                    hv = hv * 16 + hhv;
                                    pos++;
                                }
                                cret += QChar( hv );
                            }
                            break;
                          }
                        default:
                            if (c.toAscii() >= '0' && c.toAscii() <= '7') {
                                char cAscii = c.toAscii();
                                int hv = cAscii - '0';
                                for (int i = 0; i < 2; i++) {
                                    c = args[pos];
                                    if (c.toAscii() < '0' || c.toAscii() > '7')
                                        break;
                                    hv = hv * 8 + (c.toAscii() - '0');
                                    pos++;
                                }
                                cret += QChar( hv );
                            } else {
                                cret += QLatin1Char('\\');
                                cret += c;
                            }
                            break;
                        }
                    } else
                        cret += c;
                }
            } else {
                if (c == QLatin1Char('\\')) {
                    if (pos >= args.length())
                        goto quoteerr;
                    c = args.unicode()[pos++];
                    if (!c.isSpace() &&
                        !((flags & AbortOnMeta) ? isMeta( c ) : isQuoteMeta( c )))
                        cret += QLatin1Char('\\');
                } else if ((flags & AbortOnMeta) && isMeta( c ))
                    goto metaerr;
                cret += c;
            }
            if (pos >= args.length())
                break;
            c = args.unicode()[pos++];
        } while (!c.isSpace());
        ret += cret;
        firstword = false;
    }

  okret:
    if (err)
        *err = NoError;
    return ret;

  quoteerr:
    if (err)
        *err = BadQuoting;
    return QStringList();

  metaerr:
    if (err)
        *err = FoundMeta;
    return QStringList();
}

inline static bool isSpecial( QChar cUnicode )
{
    static const uchar iqm[] = {
        0xff, 0xff, 0xff, 0xff, 0xdd, 0x07, 0x00, 0xd8,
        0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x38
    }; // 0-32 \'"$`<>|;&(){}*?#

    uint c = cUnicode.unicode ();
    return (c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7)));
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

QString KShell::quoteArg( const QString &arg )
{
    if (!arg.length())
        return QString::fromLatin1("''");
    for (int i = 0; i < arg.length(); i++)
        if (isSpecial( arg.unicode()[i] )) {
            QChar q( QLatin1Char('\'') );
            return QString( arg ).replace( q, QLatin1String("'\\''") ).prepend( q ).append( q );
        }
    return arg;
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

