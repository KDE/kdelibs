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
#ifndef _KSHELL_H
#define _KSHELL_H

#include <qstring.h>
#include <qstringlist.h>
#include "kdelibs_export.h"

/**
 * \namespace KShell
 * Provides some basic POSIX shell and bash functionality.
 * @see KStringHandler
 *
 * @since 3.2
 */
namespace KShell {

    /**
     * Flags for splitArgs().
     */
    enum Options {
        NoOptions = 0,

        /**
         * Perform tilde expansion.
         */
        TildeExpand = 1,

        /**
         * Bail out if a non-quoting and not quoted shell meta character is encoutered.
         * Meta characters are the command separators @p semicolon and @p ampersand,
         * the redirection symbols @p less-than, @p greater-than and the @p pipe @p symbol,
         * the grouping symbols opening and closing @p parens and @p braces, the command
         * substitution symbol @p backquote, the generic substitution symbol @p dollar
         * (if not followed by an apostrophe), the wildcards @p asterisk and
         * @p question @p mark, and the comment symbol @p hash @p mark. Additionally,
         * a variable assignment in the first word is recognized.
         */
        AbortOnMeta = 2
    };

    /**
     * Status codes from splitArgs()
     */
    enum Errors {
        /**
         * Success.
         */
        NoError = 0,

        /**
         * Indicates a parsing error, like an unterminated quoted string.
         */
        BadQuoting,

        /**
         * The AbortOnMeta flag was set and a shell meta character
         * was encoutered.
         */
        FoundMeta
    };

    /**
     * Splits @p cmd according to POSIX shell word splitting and quoting rules.
     * Can optionally perform tilde expansion and/or abort if it finds shell
     * meta characters it cannot process.
     *
     * @param cmd the command to split
     * @param flags operation flags, see Options
     * @param err if not NULL, a status code will be stored at the pointer
     *  target, see Errors
     * @return a list of unquoted words or an empty list if an error occurred
     */
    KDECORE_EXPORT QStringList splitArgs( const QString &cmd, int flags = 0, int *err = 0 );

    /**
     * Quotes and joins @p args together according to POSIX shell rules.
     *
     * @param args a list of strings to quote and join
     * @return a command suitable for shell execution
     */
    KDECORE_EXPORT QString joinArgs( const QStringList &args );

    /**
     * Same as above, but $'' is used instead of '' for the quoting.
     * The output is suitable for splitArgs(), bash, zsh and possibly
     * other bourne-compatible shells, but not for plain sh. The advantage
     * is, that control characters (ASCII less than 32) are escaped into
     * human-readable strings.
     *
     * @param args a list of strings to quote and join
     * @return a command suitable for shell execution
     */
    KDECORE_EXPORT QString joinArgsDQ( const QStringList &args );

    /**
     * Quotes and joins @p argv together according to POSIX shell rules.
     *
     * @param argv an array of c strings to quote and join.
     *  The strings are expected to be in local-8-bit encoding.
     * @param argc maximal number of strings in @p argv. if not supplied,
     *  @p argv must be null-terminated.
     * @return a command suitable for shell execution
     */
    KDECORE_EXPORT QString joinArgs( const char * const *argv, int argc = -1 );

    /**
     * Performs tilde expansion on @p path. Interprets "~/path" and
     * "~user/path".
     *
     * @param path the path to tilde-expand
     * @return the expanded path
     */
    KDECORE_EXPORT QString tildeExpand( const QString &path );

    /**
     * Obtain a @p user's home directory.
     *
     * @param user The name of the user whose home dir should be obtained.
     *  An empty string denotes the current user.
     * @return The user's home directory.
     */
    KDECORE_EXPORT QString homeDir( const QString &user );

}


#endif /* _KSHELL_H */
