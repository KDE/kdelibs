/*
    This file is part of the KDE libraries

    Copyright (c) 2002-2003 Oswald Buddenhagen <ossi@kde.org>
    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kmacroexpander.h>

#include <qvaluestack.h>
#include <qregexp.h>

class KSelfDelimitingMacroMapExpander : public KMacroExpanderBase {

public:
    KSelfDelimitingMacroMapExpander( const QMap<QChar,QString> &map, QChar c = '%' );

protected:
    virtual bool expandPlainMacro( const QString &str, uint pos, uint &len, QString &ret );
    virtual bool expandEscapedMacro( const QString &str, uint pos, uint &len, QString &ret );

    QMap<QChar,QString> macromap;
};

class KHandDelimitedMacroMapExpander : public KMacroExpanderBase {

public:
    KHandDelimitedMacroMapExpander( const QMap<QString,QString> &map, QChar c = '%' );

protected:
    virtual bool expandPlainMacro( const QString &str, uint pos, uint &len, QString &ret );
    virtual bool expandEscapedMacro( const QString &str, uint pos, uint &len, QString &ret );

private:
    bool isIdentifier(uint c) { return c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'); }

    QMap<QString,QString> macromap;
};


KMacroExpanderBase::KMacroExpanderBase( QChar c )
{
    escapechar = c;
}

KMacroExpanderBase::~KMacroExpanderBase()
{
}

void
KMacroExpanderBase::setEscapeChar( QChar c )
{
    escapechar = c;
}

QChar
KMacroExpanderBase::escapeChar() const
{
    return escapechar;
}

void KMacroExpanderBase::expandMacros( QString &str )
{
    uint pos, len;
    QChar ec( escapechar );
    QString rst;

    for (pos = 0; pos < str.length(); ) {
        if (ec != (char)0) {
            if (str.unicode()[pos] != ec)
                goto nohit;
            if (!expandEscapedMacro( str, pos, len, rst ))
                goto nohit;
        } else {
            if (!expandPlainMacro( str, pos, len, rst ))
                goto nohit;
        }
            str.replace( pos, len, rst );
            pos += rst.length();
            continue;
      nohit:
        pos++;
    }
}


namespace KMacroExpander {

    enum Quoting { noquote, singlequote, doublequote, dollarquote, 
                   parent, subst, group, math };
    typedef struct {
        Quoting current;
        bool dquote;
    } State;
    typedef struct {
        QString str;
        uint pos;
    } Save;

}

using namespace KMacroExpander;

bool KMacroExpanderBase::expandMacrosShellQuote( QString &str, uint &pos )
{
    uint len, pos2;
    QChar ec( escapechar );
    State state = { noquote, false };
    QValueStack<State> sstack;
    QValueStack<Save> ostack;
    QString rst;

    while (pos < str.length()) {
        QChar cc( str.unicode()[pos] );
        if (ec != (char)0) {
            if (cc != ec)
                goto nohit;
            if (!expandEscapedMacro( str, pos, len, rst ))
                goto nohit;
        } else {
            if (!expandPlainMacro( str, pos, len, rst ))
                goto nohit;
        }
            if (state.dquote) {
                rst.replace( QRegExp("([$`\"\\\\])"), "\\\\1" );
                str.replace( pos, len, rst);
                pos += rst.length();
            } else if (state.current == dollarquote) {
                rst.replace( QRegExp("(['\\\\])"), "\\\\1" );
                str.replace( pos, len, rst );
                pos += rst.length();
            } else {
                rst.replace( "'", "'\\''");
		if (state.current != singlequote) {
                    rst.prepend( "'" );
                    rst.append( "'" );
                }
                str.replace( pos, len, rst);
                pos += rst.length();
            }
            continue;
      nohit:
        if (state.current == singlequote) {
            if (cc == '\'')
                state = sstack.pop();
        } else if (cc == '\\') {
            // always swallow the char -> prevent anomalies due to expansion
            pos += 2;
            continue;
        } else if (state.current == dollarquote) {
            if (cc == '\'')
                state = sstack.pop();
        } else if (cc == '$') {
            cc = str[++pos];
            if (cc == '(') {
                sstack.push( state );
                if (str[pos + 1] == '(') {
                    Save sav = { str, pos + 2 };
                    ostack.push( sav );
                    state.current = math;
                    pos += 2;
                    continue;
                } else {
                    state.current = parent;
                    state.dquote = false;
                }
            } else if (cc == '{') {
                sstack.push( state );
                state.current = subst;
            } else if (!state.dquote) {
                if (cc == '\'') {
                    sstack.push( state );
                    state.current = dollarquote;
                } else if (cc == '"') {
                    sstack.push( state );
                    state.current = doublequote;
                    state.dquote = true;
                }
            }
            // always swallow the char -> prevent anomalies due to expansion
        } else if (cc == '`') {
            str.replace( pos, 1, "$( " ); // add space -> avoid creating $((
            pos2 = pos += 3;
            for (;;) {
                if (pos2 >= str.length()) {
                    pos = pos2;
                    return false;
                }
                cc = str.unicode()[pos2];
                if (cc == '`')
                    break;
                if (cc == '\\') {
                    cc = str[++pos2];
                    if (cc == '$' || cc == '`' || cc == '\\' ||
                        (cc == '"' && state.dquote))
                    {
                        str.remove( pos2 - 1, 1 );
                        continue;
                    }
                }
                pos2++;
            }
            str[pos2] = ')';
            sstack.push( state );
            state.current = parent;
            state.dquote = false;
            continue;
        } else if (state.current == doublequote) {
            if (cc == '"')
                state = sstack.pop();
        } else if (cc == '\'') {
            if (!state.dquote) {
                sstack.push( state );
                state.current = singlequote;
            }
        } else if (cc == '"') {
            if (!state.dquote) {
                sstack.push( state );
                state.current = doublequote;
                state.dquote = true;
            }
        } else if (state.current == subst) {
            if (cc == '}')
                state = sstack.pop();
        } else if (cc == ')') {
            if (state.current == math) {
                if (str[pos + 1] == ')') {
                    state = sstack.pop();
                    pos += 2;
                } else {
                    // false hit: the $(( was a $( ( in fact
                    // ash does not care, but bash does
                    pos = ostack.top().pos;
                    str = ostack.top().str;
                    ostack.pop();
                    state.current = parent;
                    state.dquote = false;
                    sstack.push( state );
                }
                continue;
            } else if (state.current == parent)
                state = sstack.pop();
            else
                break;
        } else if (cc == '}') {
            if (state.current == KMacroExpander::group)
                state = sstack.pop();
            else
                break;
        } else if (cc == '(') {
            sstack.push( state );
            state.current = parent;
        } else if (cc == '{') {
            sstack.push( state );
            state.current = KMacroExpander::group;
        }
        pos++;
    }
    return sstack.empty();
}


//////////////////////////////////////////////////

KSelfDelimitingMacroMapExpander::KSelfDelimitingMacroMapExpander( const QMap<QChar,QString> &map, QChar c )
    : KMacroExpanderBase( c ), macromap(map)
{
}

bool
KSelfDelimitingMacroMapExpander::expandPlainMacro( const QString &str, uint pos, uint &len, QString &ret )
{
    QMapConstIterator<QChar,QString> it = macromap.find(str[pos]);
    if (it != macromap.end()) {
       len = 1;
       ret = it.data();
       return true;
    }
    return false;
}

bool
KSelfDelimitingMacroMapExpander::expandEscapedMacro( const QString &str, uint pos, uint &len, QString &ret )
{
    if (str[pos + 1] == escapeChar()) {
        len = 2;
        ret = escapeChar();
        return true;
    }

    QMapConstIterator<QChar,QString> it = macromap.find(str[pos+1]);
    if (it != macromap.end()) {
       len = 2;
       ret = it.data();
       return true;
    }
    return false;
}


//////

KHandDelimitedMacroMapExpander::KHandDelimitedMacroMapExpander( const QMap<QString,QString> &map, QChar c )
    : KMacroExpanderBase( c ), macromap(map)
{
}

bool
KHandDelimitedMacroMapExpander::expandPlainMacro( const QString &str, uint pos, uint &len, QString &ret )
{
    if (isIdentifier( str[pos - 1].unicode() ))
        return false;
    uint sl;
    for (sl = 0; isIdentifier( str[pos + sl].unicode() ); sl++);
    if (!sl)
        return false;
    QMapConstIterator<QString,QString> it =
        macromap.find( QConstString( str.unicode() + pos, sl ).string() );
    if (it != macromap.end()) {
        len = sl;
        ret = it.data();
        return true;
    }
    return false;
}

bool
KHandDelimitedMacroMapExpander::expandEscapedMacro( const QString &str, uint pos, uint &len, QString &ret )
{
    if (str[pos + 1] == escapeChar()) {
        len = 2;
        ret = escapeChar();
        return true;
    }
    uint sl, rsl, rpos;
    if (str[pos + 1] == '{') {
        rpos = pos + 2;
        for (sl = 0; str[rpos + sl] != '}'; sl++);
        rsl = sl + 3;
    } else {
        rpos = pos + 1;
        for (sl = 0; isIdentifier( str[rpos + sl].unicode() ); sl++);
        rsl = sl + 1;
    }
    if (!sl)
        return false;
    QMapConstIterator<QString,QString> it =
        macromap.find( QConstString( str.unicode() + rpos, sl ).string() );
    if (it != macromap.end()) {
        len = rsl;
        ret = it.data();
        return true;
    }
    return false;
}

/////// KMacroExpander

QString
KMacroExpander::expandMacros( const QString &ostr, const QMap<QChar,QString> &map, QChar c )
{
    QString str( ostr );
    KSelfDelimitingMacroMapExpander kmx( map, c );
    kmx.expandMacros( str );
    return str;
}

QString
KMacroExpander::expandMacrosShellQuote( const QString &ostr, const QMap<QChar,QString> &map, QChar c )
{
    QString str( ostr );
    KSelfDelimitingMacroMapExpander kmx( map, c );
    uint pos = 0;
    if (kmx.expandMacrosShellQuote( str, pos ) && pos == str.length())
        return str;
    else
        return QString::null;
}

QString
KMacroExpander::expandMacros( const QString &ostr, const QMap<QString,QString> &map, QChar c )
{
    QString str( ostr );
    KHandDelimitedMacroMapExpander kmx( map, c );
    kmx.expandMacros( str );
    return str;
}

QString
KMacroExpander::expandMacrosShellQuote( const QString &ostr, const QMap<QString,QString> &map, QChar c )
{
    QString str( ostr );
    KHandDelimitedMacroMapExpander kmx( map, c );
    uint pos = 0;
    if (kmx.expandMacrosShellQuote( str, pos ) && pos == str.length())
        return str;
    else
        return QString::null;
}

