/*
    This file is part of the KDE libraries

    Copyright (c) 2003,2007 Oswald Buddenhagen <ossi@kde.org>

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
#ifndef KSHELL_H
#define KSHELL_H

#include <kdecore_export.h>

class QStringList;
class QString;

/**
 * \namespace KShell
 * Provides some basic POSIX shell and bash functionality.
 * @see KStringHandler
 */
namespace KShell {

    /**
     * Flags for splitArgs().
     */
    enum Option {
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
    Q_DECLARE_FLAGS(Options, Option)

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
    KDECORE_EXPORT QStringList splitArgs( const QString &cmd, Options flags = NoOptions, Errors *err = 0 );

    /**
     * Quotes and joins @p args together according to POSIX shell rules.
     *
     * @param args a list of strings to quote and join
     * @return a command suitable for shell execution
     */
    KDECORE_EXPORT QString joinArgs( const QStringList &args );

    /**
     * Quotes @p arg according to POSIX shell rules.
     *
     * This function can be used to quote an argument string such that
     * the shell processes it properly. This is e.g. necessary for
     * user-provided file names which may contain spaces or quotes.
     * It also prevents expansion of wild cards and environment variables.
     *
     * @param arg the argument to quote
     * @return the quoted argument
     */
    KDECORE_EXPORT QString quoteArg( const QString &arg );

    /**
     * Performs tilde expansion on @p path. Interprets "~/path" and
     * "~user/path". If the path starts with an escaped tilde ("\~" on UNIX,
     * "^~" on Windows), the escape char is removed and the path is returned
     * as is.
     *
     * @param path the path to tilde-expand
     * @return the expanded path
     */
    KDECORE_EXPORT QString tildeExpand( const QString &path );
}

Q_DECLARE_OPERATORS_FOR_FLAGS(KShell::Options)

#endif /* KSHELL_H */
