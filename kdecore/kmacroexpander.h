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
#ifndef _KMACROEXPANDER_H
#define _KMACROEXPANDER_H

#include <qstring.h>
#include <qmap.h>

/**
 * Abstract base class for the worker classes behind the @ref KMacroExpander namespace.
 * @internal
 *
 * @since 3.2
 * @author Oswald Buddenhagen <ossi@kde.org>
 */
class KMacroExpanderBase {

public:
    /**
     * Constructor.
     * @param c escape char indicating start of macros, or QChar::null for none
     */
    KMacroExpanderBase( QChar c = '%' );

    /**
     * Destructor.
     */
    virtual ~KMacroExpanderBase();

    /**
     * Perform safe macro expansion (substitution) on a string.
     *
     * @param str the string in which macros are expanded in-place
     */
    void expandMacros( QString &str );

    /*
     * Perform safe macro expansion (substitution) on a string for use
     * in shell commands.
     *
     * Explicitly supported shell constructs:
     *   \ '' "" $'' $"" {} () $(()) ${} $() ``
     *
     * Implicitly supported shell constructs:
     *   (())
     *
     * Unsupported shell constructs that will cause problems:
     *  @li Shortened "case $v in pat)" syntax. Use "case $v in (pat)" instead.
     *
     * The rest of the shell (incl. bash) syntax is simply ignored,
     * as it is not expected to cause problems.
     *
     * Note that bash contains a bug which makes macro expansion within 
     * double quoted substitutions ("${VAR:-%macro}") inherently insecure.
     *
     * @param str the string in which macros are expanded in-place
     * @param pos the position inside the string at which parsing/substitution
     *  should start, and upon exit where processing stopped
     * @return false if the string could not be parsed and therefore no safe
     *  substitution was possible. Note that macros will have been processed
     *  up to the point where the error occured. An unmatched closing parent
     *  or brace outside any shell construct is @em not an error (unlike in
     *  the end-user functions in the KMacroExpander namespace), but still
     *  prematurely terminates processing.
     */
    bool expandMacrosShellQuote( QString &str, uint &pos );

    /**
     * Set the macro escape character.
     * @param c escape char indicating start of macros, or QChar::null if none
     */
    void setEscapeChar( QChar c );

    /**
     * Obtain the macro escape character.
     * @return escape char indicating start of macros, or QChar::null if none
     */
    QChar escapeChar() const;

protected:
    /**
     * This function is called for every single char within the string if
     * the escape char is QChar::null. It should determine whether the
     * string starting at @p pos witin @p str is a valid macro and return
     * the substitution value for it if so.
     * @param str the input string
     * @param pos the offset within @p str
     * @param len return value: the number of chars to substitute with @p ret,
     *  i.e., the determined macro length
     * @param ret return value: the string to substitute for the macro
     * @return true if a valid macro was found and substitution should be performed
     */
    virtual bool expandPlainMacro( const QString &str, uint pos, uint &len, QString &ret ) = 0;

    /**
     * This function is called every time the escape char is found if it is
     * not QChar::null. It should determine whether the
     * string starting at @p pos witin @p str is a valid macro and return
     * the substitution value for it if so.
     * @param str the input string
     * @param pos the offset within @p str. Note that this is the position of
     *  the occurence of the escape char
     * @param len return value: the number of chars to substitute with @p ret,
     *  i.e., the determined total macro length
     * @param ret return value: the string to substitute for the macro
     * @return true if a valid macro was found and substitution should be performed
     */
    virtual bool expandEscapedMacro( const QString &str, uint pos, uint &len, QString &ret ) = 0;

private:
    QChar escapechar;
};


namespace KMacroExpander {
    /**
     * Perform safe macro expansion (substitution) on a string.
     * The escape char must be quoted with itself to obtain its literal
     * representation in the resulting string.
     *
     * @param str The string to expand
     * @param map map with substitutions
     * @param c escape char indicating start of macro, or QChar::null if none
     * @return the string with all valid macros expanded
     * @since 3.2
     *
     * \code
     * // Code example
     * QMap<QChar,QString> map;
     * map.insert('u', "/tmp/myfile.txt");
     * map.insert('n', "My File");
     * QString s = "%% Title: %u:%n";
     * KMacroExpander::expandMacros(s, map);
     * // s is now "% Title: /tmp/myfile.txt:My File";
     * \endcode
     */
    QString expandMacros( const QString &str, const QMap<QChar,QString> &map, QChar c = '%' );

    /**
     * Perform safe macro expansion (substitution) on a string for use
     * in shell commands.
     * The escape char must be quoted with itself to obtain its literal
     * representation in the resulting string.
     *
     * @param str The string to expand
     * @param map map with substitutions
     * @param c escape char indicating start of macro, or QChar::null if none
     * @return the string with all valid macros expanded, or a null string
     *  if a shell syntax error was detected in the command
     * @since 3.2
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
     * \endcode
     */
    QString expandMacrosShellQuote( const QString &str, const QMap<QChar,QString> &map, QChar c = '%' );

    /**
     * Perform safe macro expansion (substitution) on a string.
     * The escape char must be quoted with itself to obtain its literal
     * representation in the resulting string.
     * Macro names can consist of chars in the range [A-Za-z0-9_];
     * use braces to delimit macros from following words starting
     * with these chars, or to use other chars for macro names.
     *
     * @param str The string to expand
     * @param map map with substitutions
     * @param c escape char indicating start of macro, or QChar::null if none
     * @return the string with all valid macros expanded
     * @since 3.2
     *
     * \code
     * // Code example
     * QMap<QString,QString> map;
     * map.insert("url", "/tmp/myfile.txt");
     * map.insert("name", "My File");
     * QString s = "Title: %{url}-%name";
     * KMacroExpander::expandMacros(s, map);
     * // s is now "Title: /tmp/myfile.txt-My File";
     * \endcode
     */
    QString expandMacros( const QString &str, const QMap<QString,QString> &map, QChar c = '%' );

    /**
     * Perform safe macro expansion (substitution) on a string for use
     * in shell commands.
     * The escape char must be quoted with itself to obtain its literal
     * representation in the resulting string.
     * Macro names can consist of chars in the range [A-Za-z0-9_];
     * use braces to delimit macros from following words starting
     * with these chars, or to use other chars for macro names.
     *
     * @param str The string to expand
     * @param map map with substitutions
     * @param c escape char indicating start of macro, or QChar::null if none
     * @return the string with all valid macros expanded, or a null string
     *  if a shell syntax error was detected in the command
     * @since 3.2
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
     * \endcode
     */
    QString expandMacrosShellQuote( const QString &str, const QMap<QString,QString> &map, QChar c = '%' );
};

#endif /* _KMACROEXPANDER_H */
