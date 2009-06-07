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

#include "kshell.h"

#include <QtCore/QStringList>
#include <QtCore/QStack>
#include <QtCore/QRegExp>

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
    ushort ec = d->escapechar.unicode();
    State state = { noquote, false };
    QStack<State> sstack;
    QStack<Save> ostack;
    QStringList rst;
    QString rsts;

    while (pos < str.length()) {
        ushort cc = str.unicode()[pos].unicode();
        if (ec != 0) {
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
                    rsts = KShell::joinArgs( rst );
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
            cc = str.unicode()[++pos].unicode();
            if (cc == '(') {
                sstack.push( state );
                if (str.unicode()[pos + 1].unicode() == '(') {
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
            str.replace( pos, 1, QLatin1String("$( " )); // add space -> avoid creating $((
            pos2 = pos += 3;
            for (;;) {
                if (pos2 >= str.length()) {
                    pos = pos2;
                    return false;
                }
                cc = str.unicode()[pos2].unicode();
                if (cc == '`')
                    break;
                if (cc == '\\') {
                    cc = str.unicode()[++pos2].unicode();
                    if (cc == '$' || cc == '`' || cc == '\\' ||
                        (cc == '"' && state.dquote))
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
                if (str.unicode()[pos + 1].unicode() == ')') {
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
