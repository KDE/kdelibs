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

#include <kshell.h>

#include <qfile.h>

#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

static int fromHex( QChar c )
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

inline static bool isQuoteMeta( uint c )
{
#if 0 // it's not worth it, especially after seeing gcc's asm output ...
    static const uchar iqm[] = {
        0x00, 0x00, 0x00, 0x00, 0x94, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00
    }; // \'"$
    
    return (c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7)));
#else
    return c == '\\' || c == '\'' || c == '"' || c == '$';
#endif
}

inline static bool isMeta( uint c )
{
    static const uchar iqm[] = {
        0x00, 0x00, 0x00, 0x00, 0xdc, 0x07, 0x00, 0xd8,
        0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x38
    }; // \'"$`<>|;&(){}*?#
    
    return (c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7)));
}

QStringList KShell::splitArgs( const QString &args, int flags, int *err )
{
    QStringList ret;
    bool firstword = flags & AbortOnMeta;

    for (uint pos = 0; ; ) {
        QChar c;
        do {
            if (pos >= args.length())
                goto okret;
            c = args.unicode()[pos++];
        } while (c.isSpace());
        QString cret;
        if ((flags & TildeExpand) && c == '~') {
            uint opos = pos;
            for (; ; pos++) {
                if (pos >= args.length())
                    break;
                c = args.unicode()[pos];
                if (c == '/' || c.isSpace())
                    break;
                if (isQuoteMeta( c )) {
                    pos = opos;
                    c = '~';
                    goto notilde;
                }
                if ((flags & AbortOnMeta) && isMeta( c ))
                    goto metaerr;
            }
            QString ccret = homeDir( QConstString( args.unicode() + opos, pos - opos ).string() );
            if (ccret.isEmpty()) {
                pos = opos;
                c = '~';
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
            if (c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
                uint pos2 = pos;
                QChar cc;
                do
                  cc = args[pos2++];
                while (cc == '_' || (cc >= 'A' && cc <= 'Z') ||
                       (cc >= 'a' && cc <= 'z') || (cc >= '0' && cc <= '9'));
                if (cc == '=')
                    goto metaerr;
            }
        }
      notilde:
        do {
            if (c == '\'') {
                uint spos = pos;
                do {
                    if (pos >= args.length())
                        goto quoteerr;
                    c = args.unicode()[pos++];
                } while (c != '\'');
                cret += QConstString( args.unicode() + spos, pos - spos - 1 ).string();
            } else if (c == '"') {
                for (;;) {
                    if (pos >= args.length())
                        goto quoteerr;
                    c = args.unicode()[pos++];
                    if (c == '"')
                        break;
                    if (c == '\\') {
                        if (pos >= args.length())
                            goto quoteerr;
                        c = args.unicode()[pos++];
                        if (c != '"' && c != '\\' &&
                            !((flags & AbortOnMeta) && (c == '$' || c == '`')))
                            cret += '\\';
                    } else if ((flags & AbortOnMeta) && (c == '$' || c == '`'))
                        goto metaerr;
                    cret += c;
                }
            } else if (c == '$' && args[pos] == '\'') {
                pos++;
                for (;;) {
                    if (pos >= args.length())
                        goto quoteerr;
                    c = args.unicode()[pos++];
                    if (c == '\'')
                        break;
                    if (c == '\\') {
                        if (pos >= args.length())
                            goto quoteerr;
                        c = args.unicode()[pos++];
                        switch (c) {
                        case 'a': cret += '\a'; break;
                        case 'b': cret += '\b'; break;
                        case 'e': cret += '\033'; break;
                        case 'f': cret += '\f'; break;
                        case 'n': cret += '\n'; break;
                        case 'r': cret += '\r'; break;
                        case 't': cret += '\t'; break;
                        case '\\': cret += '\\'; break;
                        case '\'': cret += '\''; break;
                        case 'c': cret += args[pos++] & 31; break;
                        case 'x':
                          {
                            int hv = fromHex( args[pos] );
                            if (hv < 0) {
                                cret += "\\x";
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
                            if (c >= '0' && c <= '7') {
                                int hv = c - '0';
                                for (int i = 0; i < 2; i++) {
                                    c = args[pos];
                                    if (c < '0' || c > '7')
                                        break;
                                    hv = hv * 8 + (c - '0');
                                    pos++;
                                }
                                cret += QChar( hv );
                            } else {
                                cret += '\\';
                                cret += c;
                            }
                            break;
                        }
                    } else
                        cret += c;
                }
            } else {
                if (c == '\\') {
                    if (pos >= args.length())
                        goto quoteerr;
                    c = args.unicode()[pos++];
                    if (!c.isSpace() &&
                        !((flags & AbortOnMeta) ? isMeta( c ) : isQuoteMeta( c )))
                        cret += '\\';
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

inline static bool isSpecial( uint c )
{
    static const uchar iqm[] = {
        0xff, 0xff, 0xff, 0xff, 0xdd, 0x07, 0x00, 0xd8,
        0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x38
    }; // 0-32 \'"$`<>|;&(){}*?#
    
    return (c < sizeof(iqm) * 8) && (iqm[c / 8] & (1 << (c & 7)));
}

QString KShell::joinArgs( const QStringList &args )
{
    QChar q( '\'' );
    QString ret;
    for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it) {
        if (!ret.isEmpty())
            ret += ' ';
        if (!(*it).length())
            ret.append( q ).append( q );
        else {
            for (uint i = 0; i < (*it).length(); i++)
                if (isSpecial((*it).unicode()[i])) {
                    QString tmp(*it);
                    tmp.replace( q, "'\\''" );
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
        return QString::null; // well, QString::empty, in fact. qt sucks ;)
    QChar q( '\'' );
    QString ret;
    for (const char * const *argp = args; nargs && *argp; argp++, nargs--) {
        if (!ret.isEmpty())
            ret += ' ';
        if (!**argp)
            ret.append( q ).append( q );
        else {
            QString tmp( QFile::decodeName( *argp ) );
            for (uint i = 0; i < tmp.length(); i++)
                if (isSpecial(tmp.unicode()[i])) {
                    tmp.replace( q, "'\\''" );
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
    QChar q( '\'' ), sp( ' ' ), bs( '\\' );
    QString ret;
    for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it) {
        if (!ret.isEmpty())
            ret += sp;
        if (!(*it).length())
            ret.append( q ).append( q );
        else {
            for (uint i = 0; i < (*it).length(); i++)
                if (isSpecial((*it).unicode()[i])) {
                    ret.append( '$' ).append( q );
                    for (uint pos = 0; pos < (*it).length(); pos++) {
                        int c = (*it).unicode()[pos];
                        if (c < 32) {
                            ret += bs;
                            switch (c) {
                            case '\a': ret += 'a'; break;
                            case '\b': ret += 'b'; break;
                            case '\033': ret += 'e'; break;
                            case '\f': ret += 'f'; break;
                            case '\n': ret += 'n'; break;
                            case '\r': ret += 'r'; break;
                            case '\t': ret += 't'; break;
                            case '\034': ret += 'c'; ret += '|'; break;
                            default: ret += 'c'; ret += c + '@'; break;
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
    if (fname[0] == '~') {
        int pos = fname.find( '/' );
        if (pos < 0)
            return homeDir( QConstString( fname.unicode() + 1, fname.length() - 1 ).string() );
        QString ret = homeDir( QConstString( fname.unicode() + 1, pos - 1 ).string() );
        if (!ret.isNull())
            ret += QConstString( fname.unicode() + pos, fname.length() - pos ).string();
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
        return QString::null;
    return QFile::decodeName( pw->pw_dir );
}
