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

#undef QT3_SUPPORT
#include "kshell.h"

#include <qfile.h>
#include <qstring.h>
#include <qstringlist.h>

#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

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

QStringList KShell::splitArgs( const QString &args, int flags, int *err )
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
		(c >= QLatin1Char('a') && c <= QLatin1Char('z'))) {
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
    QLatin1Char q( '\'' );
    QString ret;
    for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it) {
        if (!ret.isEmpty())
            ret += QLatin1Char(' ');
        if (!(*it).length())
            ret.append( q ).append( q );
        else {
            for (int i = 0; i < (*it).length(); i++)
                if (isSpecial((*it).unicode()[i])) {
                    QString tmp(*it);
                    tmp.replace( q, QLatin1String("'\\''" ));
                    ret += q;
                    tmp += q;
                    ret += tmp;
                    goto ex;
                }
            ret += *it;
          ex: ;
        }
    }
    return ret;
}

QString KShell::joinArgs( const char * const *args, int nargs )
{
    if (!args)
        return QString(); // well, QString::empty, in fact. qt sucks ;)
    QLatin1Char q( '\'' );
    QString ret;
    for (const char * const *argp = args; nargs && *argp; argp++, nargs--) {
        if (!ret.isEmpty())
            ret += QLatin1Char(' ');
        if (!**argp)
            ret.append( q ).append( q );
        else {
            QString tmp( QFile::decodeName( *argp ) );
            for (int i = 0; i < tmp.length(); i++)
                if (isSpecial(tmp.unicode()[i])) {
                    tmp.replace( q, QLatin1String("'\\''" ));
                    ret += q;
                    tmp += q;
                    ret += tmp;
                    goto ex;
                }
            ret += tmp;
          ex: ;
       }
    }
    return ret;
}

QString KShell::joinArgsDQ( const QStringList &args )
{
    QLatin1Char q( '\'' ), sp( ' ' ), bs( '\\' );
    QString ret;
    for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it) {
        if (!ret.isEmpty())
            ret += sp;
        if (!(*it).length())
            ret.append( q ).append( q );
        else {
            for (int i = 0; i < (*it).length(); i++)
                if (isSpecial((*it).unicode()[i])) {
                    ret.append( QChar::fromAscii('$') ).append( q );
                    for (int pos = 0; pos < (*it).length(); pos++) {
                        int c = (*it).unicode()[pos].unicode();
                        if (c < 32) {
                            ret += bs;
                            switch (c) {
                            case '\a': ret += QLatin1Char('a'); break;
                            case '\b': ret += QLatin1Char('b'); break;
                            case '\033': ret += QLatin1Char('e'); break;
                            case '\f': ret += QLatin1Char('f'); break;
                            case '\n': ret += QLatin1Char('n'); break;
                            case '\r': ret += QLatin1Char('r'); break;
                            case '\t': ret += QLatin1Char('t'); break;
                            case '\034': ret += QLatin1Char('c'); ret += QLatin1Char('|'); break;
                            default: ret += QLatin1Char('c'); ret += c + '@'; break;
                            }
                        } else {
                            if (c == '\'' || c == '\\')
                                ret += bs;
                            ret += c;
                        }
                    }
                    ret.append( q );
                    goto ex;
                }
            ret += *it;
          ex: ;
        }
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

QString KShell::homeDir( const QString &user )
{
    if (user.isEmpty())
        return QFile::decodeName( getenv( "HOME" ) );
    struct passwd *pw = getpwnam( QFile::encodeName( user ).data() );
    if (!pw)
        return QString();
    return QFile::decodeName( pw->pw_dir );
}
