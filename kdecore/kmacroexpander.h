/*
    This file is part of the KDE libraries

    Copyright (c) 2002 Oswald Buddenhagen <ossi@kde.org>
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
#ifndef _KMACROEXPANDER_H
#define _KMACROEXPANDER_H

#include <qstring.h>
#include <qmap.h>

namespace KMacroExpander {
    /**
     * Perform safe macro expansion (substitution) on a string.
     * @param str The string to expand
     * @param map map with substitutions
     * @param c escape char indicating start of macro
     *
     * \code
     * // Code example
     * QMap<QChar,QString> map;
     * map.insert('u', "/tmp/myfile.txt");
     * map.insert('n', "My File");
     * QString s = "Title: %u:%n";
     * KMacroExpander::expandMacros(s, map);
     * // s is now "Title: /tmp/myfile.txt:My File";
     * \encdcode
     */
    void expandMacros( QString &str, const QMap<QChar,QString> &map, QChar c = '%' );

    /**
     * Perform safe macro expansion (substitution) on a string for use
     * in shell commands.
     *
     * @param str The string to expand
     * @param map map with substitutions
     * @param c escape char indicating start of macro
     *
     * \code
     * // Code example
     * QMap<QChar,QString> map;
     * map.insert('u', "/tmp/myfile.txt");
     * map.insert('n', "My File");
     * QString s = "kedit --caption %n %u";
     * KMacroExpander::expandMacrosShellQuote(s, map);
     * // s is now "kedit --caption 'My File' '/tmp/myfile.txt'";
     * system(QFile::encodeName(s));
     * \encdcode
     */
    bool expandMacrosShellQuote( QString &str, const QMap<QChar,QString> &map, QChar c = '%' );

    /**
     * Perform safe macro expansion (substitution) on a string.
     * @param str The string to expand
     * @param map map with substitutions
     * @param c escape char indicating start of macro
     *
     * \code
     * // Code example
     * QMap<QString,QString> map;
     * map.insert("url", "/tmp/myfile.txt");
     * map.insert("name", "My File");
     * QString s = "Title: %{url}-%name";
     * KMacroExpander::expandMacros(s, map);
     * // s is now "Title: /tmp/myfile.txt-My File";
     * \encdcode
     */
    void expandMacros( QString &str, const QMap<QString,QString> &map, QChar c = '%' );

    /**
     * Perform safe macro expansion (substitution) on a string for use
     * in shell commands.
     *
     * @param str The string to expand
     * @param map map with substitutions
     * @param c escape char indicating start of macro
     *
     * \code
     * // Code example
     * QMap<QString,QString> map;
     * map.insert("url", "/tmp/myfile.txt");
     * map.insert("name", "My File");
     * QString s = "kedit --caption %name %{url}";
     * KMacroExpander::expandMacrosShellQuote(s, map);
     * // s is now "kedit --caption 'My File' '/tmp/myfile.txt'";
     * system(QFile::encodeName(s));
     * \encdcode
     */
    bool expandMacrosShellQuote( QString &str, const QMap<QString,QString> &map, QChar c = '%' );
};

#endif /* _KMACROEXPANDER_H */
