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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kmacroexpander.h>

#include <qvaluestack.h>
#include <qregexp.h>

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
    uint pos;
    int len;
    QChar ec( escapechar );
    QStringList rst;
    QString rsts;

    for (pos = 0; pos < str.length(); ) {
        if (ec != (char)0) {
            if (str.unicode()[pos] != ec)
                goto nohit;
            if (!(len = expandEscapedMacro( str, pos, rst )))
                goto nohit;
        } else {
            if (!(len = expandPlainMacro( str, pos, rst )))
                goto nohit;
        }
            if (len < 0) {
                pos -= len;
                continue;
            }
            rsts = rst.join( " " );
            rst.clear();
            str.replace( pos, len, rsts );
            pos += rsts.length();
            continue;
      nohit:
        pos++;
    }
}


namespace KMacroExpander {

    /** @intern Quoting state of the expander code. Not available publicly. */
    enum Quoting { noquote, singlequote, doublequote, dollarquote, 
                   paren, subst, group, math };
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
    int len;
    uint pos2;
    QChar ec( escapechar );
    State state = { noquote, false };
    QValueStack<State> sstack;
    QValueStack<Save> ostack;
    QStringList rst;
    QString rsts;

    while (pos < str.length()) {
        QChar cc( str.unicode()[pos] );
        if (ec != (char)0) {
            if (cc != ec)
                goto nohit;
            if (!(len = expandEscapedMacro( str, pos, rst )))
                goto nohit;
        } else {
            if (!(len = expandPlainMacro( str, pos, rst )))
                goto nohit;
        }
            if (len < 0) {
                pos -= len;
                continue;
            }
            if (state.dquote) {
                rsts = rst.join( " " );
                rsts.replace( QRegExp("([$`\"\\\\])"), "\\\\1" );
            } else if (state.current == dollarquote) {
                rsts = rst.join( " " );
                rsts.replace( QRegExp("(['\\\\])"), "\\\\1" );
            } else if (state.current == singlequote) {
                rsts = rst.join( " " );
                rsts.replace( '\'', "'\\''");
            } else {
                if (rst.isEmpty()) {
                    str.remove( pos, len );
                    continue;
                } else {
                    rsts = "'";
#if 0 // this could pay off if join() would be cleverer and the strings were long
                    for (QStringList::Iterator it = rst.begin(); it != rst.end(); ++it)
                        (*it).replace( '\'', "'\\''" );
                    rsts += rst.join( "' '" );
#else
                    for (QStringList::ConstIterator it = rst.begin(); it != rst.end(); ++it) {
                        if (it != rst.begin())
                            rsts += "' '";
                        QString trsts( *it );
                        trsts.replace( '\'', "'\\''" );
                        rsts += trsts;
                    }
#endif
                    rsts += "'";
                }
            }
            rst.clear();
            str.replace( pos, len, rsts );
            pos += rsts.length();
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
                    state.current = paren;
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
            state.current = paren;
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
                    state.current = paren;
                    state.dquote = false;
                    sstack.push( state );
                }
                continue;
            } else if (state.current == paren)
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
            state.current = paren;
        } else if (cc == '{') {
            sstack.push( state );
            state.current = KMacroExpander::group;
        }
        pos++;
    }
    return sstack.empty();
}

bool KMacroExpanderBase::expandMacrosShellQuote( QString &str )
{
  uint pos = 0;
  return expandMacrosShellQuote( str, pos ) && pos == str.length();
}

int KMacroExpanderBase::expandPlainMacro( const QString &, uint, QStringList & )
{ qFatal( "KMacroExpanderBase::expandPlainMacro called!" ); return 0; }

int KMacroExpanderBase::expandEscapedMacro( const QString &, uint, QStringList & )
{ qFatal( "KMacroExpanderBase::expandEscapedMacro called!" ); return 0; }


//////////////////////////////////////////////////

template<class KT,class VT>
class KMacroMapExpander : public KMacroExpanderBase {

public:
    KMacroMapExpander( const QMap<KT,VT> &map, QChar c = '%' ) :
        KMacroExpanderBase( c ), macromap( map ) {}

protected:
    virtual int expandPlainMacro( const QString &str, uint pos, QStringList &ret );
    virtual int expandEscapedMacro( const QString &str, uint pos, QStringList &ret );

private:
    QMap<KT,VT> macromap;
};

static QStringList &operator+=( QStringList &s, const QString &n) { s << n; return s; }

////////

static bool
isIdentifier( uint c )
{
    return c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

////////

template<class VT>
class KMacroMapExpander<QChar,VT> : public KMacroExpanderBase {

public:
    KMacroMapExpander( const QMap<QChar,VT> &map, QChar c = '%' ) :
        KMacroExpanderBase( c ), macromap( map ) {}

protected:
    virtual int expandPlainMacro( const QString &str, uint pos, QStringList &ret );
    virtual int expandEscapedMacro( const QString &str, uint pos, QStringList &ret );

private:
    QMap<QChar,VT> macromap;
};

template<class VT>
int
KMacroMapExpander<QChar,VT>::expandPlainMacro( const QString &str, uint pos, QStringList &ret )
{
    QMapConstIterator<QChar,VT> it = macromap.find(str[pos]);
    if (it != macromap.end()) {
       ret += it.data();
       return 1;
    }
    return 0;
}

template<class VT>
int
KMacroMapExpander<QChar,VT>::expandEscapedMacro( const QString &str, uint pos, QStringList &ret )
{
    if (str[pos + 1] == escapeChar()) {
        ret += QString( escapeChar() );
        return 2;
    }
    QMapConstIterator<QChar,VT> it = macromap.find(str[pos+1]);
    if (it != macromap.end()) {
       ret += it.data();
       return 2;
    }

    return 0;
}

template<class VT>
class KMacroMapExpander<QString,VT> : public KMacroExpanderBase {

public:
    KMacroMapExpander( const QMap<QString,VT> &map, QChar c = '%' ) :
        KMacroExpanderBase( c ), macromap( map ) {}

protected:
    virtual int expandPlainMacro( const QString &str, uint pos, QStringList &ret );
    virtual int expandEscapedMacro( const QString &str, uint pos, QStringList &ret );

private:
    QMap<QString,VT> macromap;
};

template<class VT>
int
KMacroMapExpander<QString,VT>::expandPlainMacro( const QString &str, uint pos, QStringList &ret )
{
    if (isIdentifier( str[pos - 1].unicode() ))
        return 0;
    uint sl;
    for (sl = 0; isIdentifier( str[pos + sl].unicode() ); sl++);
    if (!sl)
        return 0;
    QMapConstIterator<QString,VT> it =
        macromap.find( QConstString( str.unicode() + pos, sl ).string() );
    if (it != macromap.end()) {
        ret += it.data();
        return sl;
    }
    return 0;
}

template<class VT>
int
KMacroMapExpander<QString,VT>::expandEscapedMacro( const QString &str, uint pos, QStringList &ret )
{
    if (str[pos + 1] == escapeChar()) {
        ret += QString( escapeChar() );
        return 2;
    }
    uint sl, rsl, rpos;
    if (str[pos + 1] == '{') {
        rpos = pos + 2;
        for (sl = 0; str[rpos + sl] != '}'; sl++)
            if (rpos + sl >= str.length())
                return 0;
        rsl = sl + 3;
    } else {
        rpos = pos + 1;
        for (sl = 0; isIdentifier( str[rpos + sl].unicode() ); sl++);
        rsl = sl + 1;
    }
    if (!sl)
        return 0;
    QMapConstIterator<QString,VT> it =
        macromap.find( QConstString( str.unicode() + rpos, sl ).string() );
    if (it != macromap.end()) {
        ret += it.data();
        return rsl;
    }
    return 0;
}

////////////

int
KCharMacroExpander::expandPlainMacro( const QString &str, uint pos, QStringList &ret )
{
    if (expandMacro( str[pos], ret ))
        return 1;
    return 0;
}

int
KCharMacroExpander::expandEscapedMacro( const QString &str, uint pos, QStringList &ret )
{
    if (str[pos + 1] == escapeChar()) {
        ret += QString( escapeChar() );
        return 2;
    }
    if (expandMacro( str[pos+1], ret ))
        return 2;
    return 0;
}

int
KWordMacroExpander::expandPlainMacro( const QString &str, uint pos, QStringList &ret )
{
    if (isIdentifier( str[pos - 1].unicode() ))
        return 0;
    uint sl;
    for (sl = 0; isIdentifier( str[pos + sl].unicode() ); sl++);
    if (!sl)
        return 0;
    if (expandMacro( QConstString( str.unicode() + pos, sl ).string(), ret ))
        return sl;
    return 0;
}

int
KWordMacroExpander::expandEscapedMacro( const QString &str, uint pos, QStringList &ret )
{
    if (str[pos + 1] == escapeChar()) {
        ret += QString( escapeChar() );
        return 2;
    }
    uint sl, rsl, rpos;
    if (str[pos + 1] == '{') {
        rpos = pos + 2;
        for (sl = 0; str[rpos + sl] != '}'; sl++)
            if (rpos + sl >= str.length())
                return 0;
        rsl = sl + 3;
    } else {
        rpos = pos + 1;
        for (sl = 0; isIdentifier( str[rpos + sl].unicode() ); sl++);
        rsl = sl + 1;
    }
    if (!sl)
        return 0;
    if (expandMacro( QConstString( str.unicode() + rpos, sl ).string(), ret ))
        return rsl;
    return 0;
}

////////////

template<class KT,class VT>
inline QString
TexpandMacros( const QString &ostr, const QMap<KT,VT> &map, QChar c )
{
    QString str( ostr );
    KMacroMapExpander<KT,VT> kmx( map, c );
    kmx.expandMacros( str );
    return str;
}

template<class KT,class VT>
inline QString
TexpandMacrosShellQuote( const QString &ostr, const QMap<KT,VT> &map, QChar c )
{
    QString str( ostr );
    KMacroMapExpander<KT,VT> kmx( map, c );
    if (!kmx.expandMacrosShellQuote( str ))
        return QString::null;
    return str;
}

// public API
namespace KMacroExpander {

  QString expandMacros( const QString &ostr, const QMap<QChar,QString> &map, QChar c ) { return TexpandMacros( ostr, map, c ); }
  QString expandMacrosShellQuote( const QString &ostr, const QMap<QChar,QString> &map, QChar c ) { return TexpandMacrosShellQuote( ostr, map, c ); }
  QString expandMacros( const QString &ostr, const QMap<QString,QString> &map, QChar c ) { return TexpandMacros( ostr, map, c ); }
  QString expandMacrosShellQuote( const QString &ostr, const QMap<QString,QString> &map, QChar c ) { return TexpandMacrosShellQuote( ostr, map, c ); }
  QString expandMacros( const QString &ostr, const QMap<QChar,QStringList> &map, QChar c ) { return TexpandMacros( ostr, map, c ); }
  QString expandMacrosShellQuote( const QString &ostr, const QMap<QChar,QStringList> &map, QChar c ) { return TexpandMacrosShellQuote( ostr, map, c ); }
  QString expandMacros( const QString &ostr, const QMap<QString,QStringList> &map, QChar c ) { return TexpandMacros( ostr, map, c ); }
  QString expandMacrosShellQuote( const QString &ostr, const QMap<QString,QStringList> &map, QChar c ) { return TexpandMacrosShellQuote( ostr, map, c ); }

} // namespace
