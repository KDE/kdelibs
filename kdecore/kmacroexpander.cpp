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

#include "kmacroexpander.h"
#include "kdebug.h"

#include <qstack.h>
#include <qregexp.h>
#include <qhash.h>
#include <qstring.h>
#include <qstringlist.h>

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
    int pos;
    int len;
    QChar ec( escapechar );
    QStringList rst;
    QString rsts;

    for (pos = 0; pos < str.length(); ) {
        if (ec != QLatin1Char(0) ) {
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
            rsts = rst.join( QLatin1String(" ") );
            rst.clear();
            str.replace( pos, len, rsts );
            pos += rsts.length();
            continue;
      nohit:
        pos++;
    }
}


namespace KMacroExpander {

    enum Quoting { noquote, singlequote, doublequote, dollarquote, 
                   paren, subst, group, math };
    typedef struct {
        Quoting current;
        bool dquote;
    } State;
    typedef struct {
        QString str;
        int pos;
    } Save;

}

using namespace KMacroExpander;

bool KMacroExpanderBase::expandMacrosShellQuote( QString &str, int &pos )
{
    int len;
    int pos2;
    QChar ec( escapechar );
    State state = { noquote, false };
    QStack<State> sstack;
    QStack<Save> ostack;
    QStringList rst;
    QString rsts;

    while (pos < str.length()) {
        QChar cc( str.unicode()[pos] );
        if (ec != QLatin1Char(0)) {
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
                rsts = rst.join( QLatin1String(" ") );
                rsts.replace( QRegExp(QLatin1String("([$`\"\\\\])")), QLatin1String("\\\\1") );
            } else if (state.current == dollarquote) {
                rsts = rst.join( QLatin1String(" ") );
                rsts.replace( QRegExp(QLatin1String("(['\\\\])")), QLatin1String("\\\\1") );
            } else if (state.current == singlequote) {
                rsts = rst.join( QLatin1String(" ") );
                rsts.replace( QLatin1Char('\''), QLatin1String("'\\''") );
            } else {
                if (rst.isEmpty()) {
                    str.remove( pos, len );
                    continue;
                } else {
                    rsts = QLatin1Char('\'');
#if 0 // this could pay off if join() would be cleverer and the strings were long
                    for (QStringList::Iterator it = rst.begin(); it != rst.end(); ++it)
                        (*it).replace( '\'', "'\\''" );
                    rsts += rst.join( "' '" );
#else
                    for (QStringList::ConstIterator it = rst.begin(); it != rst.end(); ++it) {
                        if (it != rst.begin())
                            rsts += QLatin1String("' '");
                        QString trsts( *it );
                        trsts.replace( QLatin1Char('\''), QLatin1String("'\\''" ) );
                        rsts += trsts;
                    }
#endif
                    rsts += QLatin1Char('\'');
                }
            }
            rst.clear();
            str.replace( pos, len, rsts );
            pos += rsts.length();
            continue;
      nohit:
        if (state.current == singlequote) {
            if (cc == QLatin1Char('\''))
                state = sstack.pop();
        } else if (cc == QLatin1Char('\\')) {
            // always swallow the char -> prevent anomalies due to expansion
            pos += 2;
            continue;
        } else if (state.current == dollarquote) {
            if (cc == QLatin1Char('\''))
                state = sstack.pop();
        } else if (cc == QLatin1Char('$')) {
            cc = str[++pos];
            if (cc == QLatin1Char('(')) {
                sstack.push( state );
                if (str[pos + 1] == QLatin1Char('(')) {
                    Save sav = { str, pos + 2 };
                    ostack.push( sav );
                    state.current = math;
                    pos += 2;
                    continue;
                } else {
                    state.current = paren;
                    state.dquote = false;
                }
            } else if (cc == QLatin1Char('{')) {
                sstack.push( state );
                state.current = subst;
            } else if (!state.dquote) {
                if (cc == QLatin1Char('\'')) {
                    sstack.push( state );
                    state.current = dollarquote;
                } else if (cc == QLatin1Char('"')) {
                    sstack.push( state );
                    state.current = doublequote;
                    state.dquote = true;
                }
            }
            // always swallow the char -> prevent anomalies due to expansion
        } else if (cc == QLatin1Char('`')) {
            str.replace( pos, 1, QLatin1String("$( " )); // add space -> avoid creating $((
            pos2 = pos += 3;
            for (;;) {
                if (pos2 >= str.length()) {
                    pos = pos2;
                    return false;
                }
                cc = str.unicode()[pos2];
                if (cc == QLatin1Char('`'))
                    break;
                if (cc == QLatin1Char('\\')) {
                    cc = str[++pos2];
                    if (cc == QLatin1Char('$') || cc == QLatin1Char('`') || cc == QLatin1Char('\\') ||
                        (cc == QLatin1Char('"') && state.dquote))
                    {
                        str.remove( pos2 - 1, 1 );
                        continue;
                    }
                }
                pos2++;
            }
            str[pos2] = QLatin1Char(')');
            sstack.push( state );
            state.current = paren;
            state.dquote = false;
            continue;
        } else if (state.current == doublequote) {
            if (cc == QLatin1Char('"'))
                state = sstack.pop();
        } else if (cc == QLatin1Char('\'')) {
            if (!state.dquote) {
                sstack.push( state );
                state.current = singlequote;
            }
        } else if (cc == QLatin1Char('"')) {
            if (!state.dquote) {
                sstack.push( state );
                state.current = doublequote;
                state.dquote = true;
            }
        } else if (state.current == subst) {
            if (cc == QLatin1Char('}'))
                state = sstack.pop();
        } else if (cc == QLatin1Char(')')) {
            if (state.current == math) {
                if (str[pos + 1] == QLatin1Char(')')) {
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
        } else if (cc == QLatin1Char('}')) {
            if (state.current == KMacroExpander::group)
                state = sstack.pop();
            else
                break;
        } else if (cc == QLatin1Char('(')) {
            sstack.push( state );
            state.current = paren;
        } else if (cc == QLatin1Char('{')) {
            sstack.push( state );
            state.current = KMacroExpander::group;
        }
        pos++;
    }
    return sstack.empty();
}

bool KMacroExpanderBase::expandMacrosShellQuote( QString &str )
{
  int pos = 0;
  return expandMacrosShellQuote( str, pos ) && pos == str.length();
}

int KMacroExpanderBase::expandPlainMacro( const QString &, int, QStringList & )
{ qFatal( "KMacroExpanderBase::expandPlainMacro called!" ); return 0; }

int KMacroExpanderBase::expandEscapedMacro( const QString &, int, QStringList & )
{ qFatal( "KMacroExpanderBase::expandEscapedMacro called!" ); return 0; }


//////////////////////////////////////////////////

template <typename KT, typename VT>
class KMacroMapExpander : public KMacroExpanderBase {

public:
    KMacroMapExpander( const QHash<KT,VT> &map, QChar c = QLatin1Char('%') ) :
        KMacroExpanderBase( c ), macromap( map ) {}

protected:
    virtual int expandPlainMacro( const QString &str, int pos, QStringList &ret );
    virtual int expandEscapedMacro( const QString &str, int pos, QStringList &ret );

private:
    QHash<KT,VT> macromap;
};

static QStringList &operator+=( QStringList &s, const QString &n) { s << n; return s; }

////////

static bool
isIdentifier( QChar c )
{
    return c == QLatin1Char('_') || 
          (c >= QLatin1Char('A') && c <= QLatin1Char('Z')) || 
          (c >= QLatin1Char('a') && c <= QLatin1Char('z')) || 
          (c >= QLatin1Char('0') && c <= QLatin1Char('9'));
}

////////

template <typename VT>
class KMacroMapExpander<QChar,VT> : public KMacroExpanderBase {

public:
    KMacroMapExpander( const QHash<QChar,VT> &map, QChar c = QLatin1Char('%') ) :
        KMacroExpanderBase( c ), macromap( map ) {}

protected:
    virtual int expandPlainMacro( const QString &str, int pos, QStringList &ret );
    virtual int expandEscapedMacro( const QString &str, int pos, QStringList &ret );

private:
    QHash<QChar,VT> macromap;
};

template <typename VT>
int
KMacroMapExpander<QChar,VT>::expandPlainMacro( const QString &str, int pos, QStringList &ret )
{
    const KMacroMapExpander<QChar,VT> *const_this = this;
    typename QHash<QChar,VT>::const_iterator it = const_this->macromap.find(str[pos]);
    if (it != macromap.end()) {
       ret += it.value();
       return 1;
    }
    return 0;
}

template <typename VT>
int
KMacroMapExpander<QChar,VT>::expandEscapedMacro( const QString &str, int pos, QStringList &ret )
{
    if (str[pos + 1] == escapeChar()) {
        ret += QString( escapeChar() );
        return 2;
    }
    const KMacroMapExpander<QChar,VT> *const_this = this;
    typename QHash<QChar,VT>::const_iterator it = const_this->macromap.find(str[pos+1]);
    if (it != const_this->macromap.end()) {
       ret += it.value();
       return 2;
    }

    return 0;
}

template <typename VT>
class KMacroMapExpander<QString,VT> : public KMacroExpanderBase {

public:
    KMacroMapExpander( const QHash<QString,VT> &map, QChar c = QLatin1Char('%') ) :
        KMacroExpanderBase( c ), macromap( map ) {}

protected:
    virtual int expandPlainMacro( const QString &str, int pos, QStringList &ret );
    virtual int expandEscapedMacro( const QString &str, int pos, QStringList &ret );

private:
    QHash<QString,VT> macromap;
};

template <typename VT>
int
KMacroMapExpander<QString,VT>::expandPlainMacro( const QString &str, int pos, QStringList &ret )
{
    if (isIdentifier( str[pos - 1] ))
        return 0;
    int sl;
    for (sl = 0; isIdentifier( str[pos + sl] ); sl++);
    if (!sl)
        return 0;
    const KMacroMapExpander<QString,VT> *const_this = this;
    typename QHash<QString,VT>::const_iterator it = 
        const_this->macromap.find( str.mid( pos, sl ) );
    if (it != macromap.end()) {
        ret += it.value();
        return sl;
    }
    return 0;
}

template <typename VT>
int
KMacroMapExpander<QString,VT>::expandEscapedMacro( const QString &str, int pos, QStringList &ret )
{
    if (str.length() <= pos + 1)
      return 0;

    if (str[pos + 1] == escapeChar()) {
        ret += QString( escapeChar() );
        return 2;
    }
    int sl, rsl, rpos;
    if (str[pos + 1] == QLatin1Char('{')) {
        rpos = pos + 2;
        sl = str.indexOf(QLatin1Char('}'), rpos);
        if (sl == -1)
            return 0;
        else
          sl -= rpos;
        rsl = sl + 3;
    } else {
        rpos = pos + 1;
        for (sl = 0; (rpos + sl < str.length()) && isIdentifier( str[rpos + sl] ); sl++);
        rsl = sl + 1;
    }
    if (!sl)
        return 0;
    const KMacroMapExpander<QString,VT> *const_this = this;
    typename QHash<QString,VT>::const_iterator it =
        const_this->macromap.find( str.mid( rpos, sl ) );
    if (it != macromap.end()) {
        ret += it.value();
        return rsl;
    }
    return 0;
}

////////////

int
KCharMacroExpander::expandPlainMacro( const QString &str, int pos, QStringList &ret )
{
    if (expandMacro( str[pos], ret ))
        return 1;
    return 0;
}

int
KCharMacroExpander::expandEscapedMacro( const QString &str, int pos, QStringList &ret )
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
KWordMacroExpander::expandPlainMacro( const QString &str, int pos, QStringList &ret )
{
    if (isIdentifier( str[pos - 1] ))
        return 0;
    int sl;
    for (sl = 0; isIdentifier( str[pos + sl] ); sl++);
    if (!sl)
        return 0;
    if (expandMacro( str.mid( pos, sl ), ret ))
        return sl;
    return 0;
}

int
KWordMacroExpander::expandEscapedMacro( const QString &str, int pos, QStringList &ret )
{
    if (str[pos + 1] == escapeChar()) {
        ret += QString( escapeChar() );
        return 2;
    }
    int sl, rsl, rpos;
    if (str[pos + 1] == QLatin1Char('{')) {
        rpos = pos + 2;
        for (sl = 0; str[rpos + sl] != QLatin1Char('}'); sl++)
            if (rpos + sl >= str.length())
                return 0;
        rsl = sl + 3;
    } else {
        rpos = pos + 1;
        for (sl = 0; isIdentifier( str[rpos + sl] ); sl++);
        rsl = sl + 1;
    }
    if (!sl)
        return 0;
    if (expandMacro( str.mid( rpos, sl ), ret ))
        return rsl;
    return 0;
}

////////////

template <typename KT, typename VT>
inline QString
TexpandMacros( const QString &ostr, const QHash<KT,VT> &map, QChar c )
{
    QString str( ostr );
    KMacroMapExpander<KT,VT> kmx( map, c );
    kmx.expandMacros( str );
    return str;
}

template <typename KT, typename VT>
inline QString
TexpandMacrosShellQuote( const QString &ostr, const QHash<KT,VT> &map, QChar c )
{
    QString str( ostr );
    KMacroMapExpander<KT,VT> kmx( map, c );
    if (!kmx.expandMacrosShellQuote( str ))
        return QString::null;
    return str;
}

// public API
namespace KMacroExpander {

  QString expandMacros( const QString &ostr, const QHash<QChar,QString> &map, QChar c )
    { return TexpandMacros( ostr, map, c ); }
  QString expandMacrosShellQuote( const QString &ostr, const QHash<QChar,QString> &map, QChar c )
    { return TexpandMacrosShellQuote( ostr, map, c ); }
  QString expandMacros( const QString &ostr, const QHash<QString,QString> &map, QChar c )
    { return TexpandMacros( ostr, map, c ); }
  QString expandMacrosShellQuote( const QString &ostr, const QHash<QString,QString> &map, QChar c )
    { return TexpandMacrosShellQuote( ostr, map, c ); }
  QString expandMacros( const QString &ostr, const QHash<QChar,QStringList> &map, QChar c )
    { return TexpandMacros( ostr, map, c ); }
  QString expandMacrosShellQuote( const QString &ostr, const QHash<QChar,QStringList> &map, QChar c )
    { return TexpandMacrosShellQuote( ostr, map, c ); }
  QString expandMacros( const QString &ostr, const QHash<QString,QStringList> &map, QChar c )
    { return TexpandMacros( ostr, map, c ); }
  QString expandMacrosShellQuote( const QString &ostr, const QHash<QString,QStringList> &map, QChar c )
    { return TexpandMacrosShellQuote( ostr, map, c ); }

} // namespace
