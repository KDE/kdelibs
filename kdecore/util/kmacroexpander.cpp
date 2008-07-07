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

#include "kmacroexpander_p.h"
#include "kdebug.h"

#include <QtCore/QHash>
#include <QtCore/QStringList>

KMacroExpanderBase::KMacroExpanderBase( QChar c ) : d(new KMacroExpanderBasePrivate(c))
{
}

KMacroExpanderBase::~KMacroExpanderBase()
{
    delete d;
}

void
KMacroExpanderBase::setEscapeChar( QChar c )
{
    d->escapechar = c;
}

QChar
KMacroExpanderBase::escapeChar() const
{
    return d->escapechar;
}

void KMacroExpanderBase::expandMacros( QString &str )
{
    int pos;
    int len;
    QChar ec( d->escapechar );
    QStringList rst;
    QString rsts;

    for (pos = 0; pos < str.length(); ) {
        if (ec != QLatin1Char(0) ) {
            if (str.unicode()[pos] != ec)
                goto nohit;
            kDebug(9010) << "trying"<< str << "from" << pos << ":" << str.mid(pos);
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
    if (it != const_this->macromap.end()) {
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
    for (sl = 0; isIdentifier( str[pos + sl] ); sl++)
        ;
    if (!sl)
        return 0;
    const KMacroMapExpander<QString,VT> *const_this = this;
    typename QHash<QString,VT>::const_iterator it = 
        const_this->macromap.find( str.mid( pos, sl ) );
    if (it != const_this->macromap.end()) {
        ret += it.value();
        return sl;
    }
    return 0;
}

template <typename VT>
int
KMacroMapExpander<QString,VT>::expandEscapedMacro( const QString &str, int pos, QStringList &ret )
{
    kDebug() << "Expanding qstring vt macro:" << str << pos;
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
        for (sl = 0; (rpos + sl < str.length()) && isIdentifier( str[rpos + sl] ); sl++)
            ;
        rsl = sl + 1;
    }
    if (!sl)
        return 0;
    const KMacroMapExpander<QString,VT> *const_this = this;
    typename QHash<QString,VT>::const_iterator it =
        const_this->macromap.find( str.mid( rpos, sl ) );
    if (it != const_this->macromap.end()) {
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
    for (sl = 0; isIdentifier( str[pos + sl] ); sl++)
        ;
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
        for (sl = 0; isIdentifier( str[rpos + sl] ); sl++)
            ;
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
        return QString();
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
