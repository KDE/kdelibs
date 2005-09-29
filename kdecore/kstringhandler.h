/* This file is part of the KDE libraries
   Copyright (C) 1999 Ian Zepp (icszepp@islc.net)
   Copyright (C) 2000 Rik Hemsley (rikkus) <rik@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KSTRINGHANDLER_H
#define KSTRINGHANDLER_H

#include <qstring.h>
#include <qstringlist.h>
#include <qregexp.h>            // for the word ranges
#include <qfontmetrics.h>
#include "kdelibs_export.h"

/**
 * This class contains utility functions for handling strings.
 *
 * This class is @em not a substitute for the QString class. What
 * I tried to do with this class is provide an easy way to
 * cut/slice/splice words inside sentences in whatever order desired.
 * While the main focus of this class are words (ie characters
 * separated by spaces/tabs), the two core functions here ( split()
 * and join() ) will function given any char to use as a separator.
 * This will make it easy to redefine what a 'word' means in the
 * future if needed.
 *
 * I freely stole some of the function names from python. I also think
 * some of these were influenced by mIRC (yes, believe it if you will, I
 * used to write a LOT of scripts in mIRC).
 *
 * The ranges are a fairly powerful way of getting/stripping words from
 * a string. These ranges function, for the large part, as they would in
 * python. See the word(const QString&, const char *) and remword(const QString&, uint) functions for more detail.
 *
 * This class contains no data members of its own. All strings are cut
 * on the fly and returned as new qstrings/qstringlists.
 *
 * Quick example on how to use:
 *
 * \code
 * KStringHandler kstr;
 * QString line = "This is a test of the strings";
 *
 * cout << "1> " << kstr.word( line , "4:" ) << "\n";
 * cout << "2> " << kstr.remrange( line , "2:5" ) << "\n";
 * cout << "2> " << kstr.reverse( line ) << "\n";
 * cout << "2> " << kstr.center( kstr.word( line , 4 ) , 15 ) << "\n";
 * \endcode
 *
 * and so forth.
 *
 * @short Class for manipulating words and sentences in strings
 * @author Ian Zepp <icszepp@islc.net>
 * @see KShell
 */
class KDECORE_EXPORT KStringHandler
{
public:
    /** Returns the nth word in the string if found
      * Returns a EMPTY (not null) string otherwise.
      * Note that the FIRST index is 0.
      * @param text the string to search for the words
      * @param pos the position of the word to search
      * @return the word, or an empty string if not found
      * @deprecated use QString::section instead
      */
    static QString        word( const QString &text , uint pos ) KDE_DEPRECATED;

    /** Returns a range of words from that string.
      * Ie:
      * @li "0" returns the very first word
      * @li "0:" returns the first to the last word
      * @li "0:3" returns the first to fourth words
      * @li ":3" returns everything up to the fourth word
      *
      * If you grok python, you're set.
      * @param text the string to search for the words
      * @param range the words to return (see description)
      * @return the words, or an empty string if not found
      */
    static QString        word( const QString &text , const char *range );

    /** Inserts a word into the string, and returns
      * a new string with the word included. the first
      * index is zero (0). If there are not @p pos words in the original
      * string, the new word will be appended to the end.
      * @param text the original text
      * @param word the word to insert
      * @param pos the position (in words) for the new word
      * @return the resulting string
      */
    static QString        insword( const QString &text , const QString &word , uint pos );

    /** Replaces a word in the string, and returns
      * a new string with the word included. the first
      * index is zero (0). If there are not @p pos words in the original
      * string, the new word will be appended to the end.
      * @param text the original text
      * @param word the word to insert
      * @param pos the position (in words) for the new word
      * @return the resulting string
      */
    static QString        setword( const QString &text , const QString &word , uint pos );

    /** Removes a word or ranges of words from the string,
      * and returns a new string. The ranges definitions
      * follow the definitions for the word() function.
      *
      * @li "0"        removes the very first word
      * @li "0:"    removes the first the the last word
      * @li "0:3"    removes the first to fourth words
      * @li ":3"    removes everything up to the fourth word
      * @param text the original text
      * @param range the words to remove (see description)
      * @return the resulting string
      */
    static QString        remrange( const QString &text , const char *range );


    /** Removes a word at the given index, and returns a
      * new string. The first index is zero (0).
      * @param text the original text
      * @param pos the position (in words) of thw word to delete
      * @return the resulting string
      */
    static QString        remword( const QString &text , uint pos );

    /** Removes a matching word from the string, and returns
      * a new string. Note that only ONE match is removed.
      * @param text the original text
      * @param word the word to remove
      * @return the resulting string
      */
    static QString        remword( const QString &text , const QString &word );

    /** Capitalizes each word in the string
      * "hello there" becomes "Hello There"        (string)
      * @param text the text to capitalize
      * @return the resulting string
      */
    static QString        capwords( const QString &text );

    /** Capitalizes each word in the list
      * [hello, there] becomes [Hello, There]    (list)
      * @param list the list to capitalize
      * @return the resulting list
      */
    static QStringList    capwords( const QStringList &list );

    /** Reverses the order of the words in a string
      * "hello there" becomes "there hello"        (string)
      * @param text the text to reverse
      * @return the resulting string
      */
    static QString        reverse( const QString &text );

    /** Reverses the order of the words in a list
      * [hello, there] becomes [there, hello]    (list)
      * @param list the list to reverse
      * @return the resulting list
      */
    static QStringList    reverse( const QStringList &list );

    /** Left-justifies a string and returns a string at least 'width' characters
      * wide.
      * If the string is longer than the @p width, the original
      * string is returned. It is never truncated.
      * @param text the text to justify
      * @param width the desired width of the new string
      * @return the resulting string
      * @deprecated use QString::leftJustify instead
      */
    static QString        ljust( const QString &text , uint width ) KDE_DEPRECATED;

    /** Right-justifies a string and returns a string at least 'width' characters
      * wide.
      * If the string is longer than the @p width, the original
      * string is returned. It is never truncated.
      * @param text the text to justify
      * @param width the desired width of the new string
      * @return the resulting string
      * @deprecated use QString::rightJustify instead
      */
    static QString        rjust( const QString &text , uint width ) KDE_DEPRECATED;

    /** Centers a string and returns a string at least 'width' characters
      * wide.
      * If the string is longer than the @p width, the original
      * string is returned. It is never truncated.
      * @param text the text to justify
      * @param width the desired width of the new string
      * @return the resulting string
      */
    static QString        center( const QString &text , uint width );

    /** Substitute characters at the beginning of a string by "...".
     * @param str is the string to modify
     * @param maxlen is the maximum length the modified string will have
     * If the original string is shorter than "maxlen", it is returned verbatim
     * @return the modified string
     */
    static QString        lsqueeze( const QString & str, uint maxlen = 40 );

    /** Substitute characters at the beginning of a string by "...". Similar to
     * method above, except that it truncates based on pixel width rather than
     * the number of characters
     * @param name is the string to modify
     * @param fontMetrics is the font metrics to use to calculate character sizes
     * @param maxlen is the maximum length in ems the modified string will have
     * If the original string is shorter than "maxlen", it is returned verbatim
     * @return the modified string
     * @since 3.2
     */
    static QString lEmSqueeze( const QString & name,
                               const QFontMetrics& fontMetrics,
                               uint maxlen = 30 );

    /** Substitute characters at the beginning of a string by "...". Similar to
     * method above, except that maxlen is the width in pixels to truncate to
     * @param name is the string to modify
     * @param fontMetrics is the font metrics to use to calculate character sizes
     * @param maxPixels is the maximum pixel length the modified string will have
     * If the original string is shorter than "maxlen", it is returned verbatim
     * @return the modified string
     * @since 3.2
     */
    static QString lPixelSqueeze( const QString & name,
                                  const QFontMetrics& fontMetrics,
                                  uint maxPixels );

    /** Substitute characters at the middle of a string by "...".
     * @param str is the string to modify
     * @param maxlen is the maximum length the modified string will have
     * If the original string is shorter than "maxlen", it is returned verbatim
     * @return the modified string
     */
    static QString        csqueeze( const QString & str, uint maxlen = 40 );

    /** Substitute characters in the middle of a string by "...". Similar to
     * method above, except that it truncates based on pixel width rather than
     * the number of characters
     * @param name is the string to modify
     * @param fontMetrics is the font metrics to use to calculate character sizes
     * @param maxlen is the maximum length in ems the modified string will have
     * If the original string is shorter than "maxlen", it is returned verbatim
     * @return the modified string
     * @since 3.2
     */
    static QString cEmSqueeze( const QString & name,
                               const QFontMetrics& fontMetrics,
                               uint maxlen = 30 );

    /** Substitute characters in the middle of a string by "...". Similar to
     * method above, except that maxlen is the width in pixels to truncate to
     * @param name is the string to modify
     * @param fontMetrics is the font metrics to use to calculate character sizes
     * @param maxPixels is the maximum pixel length the modified string will have
     * If the original string is shorter than "maxlen", it is returned verbatim
     * @return the modified string
     * @since 3.2
     */
    static QString cPixelSqueeze( const QString & name,
                                  const QFontMetrics& fontMetrics,
                                  uint maxPixels );

    /** Substitute characters at the end of a string by "...".
     * @param str is the string to modify
     * @param maxlen is the maximum length the modified string will have
     * If the original string is shorter than "maxlen", it is returned verbatim
     * @return the modified string
     */
    static QString        rsqueeze( const QString & str, uint maxlen = 40 );

    /** Substitute characters at the end of a string by "...". Similar to
     * method above, except that it truncates based on pixel width rather than
     * the number of characters
     * @param name is the string to modify
     * @param fontMetrics is the font metrics to use to calculate character sizes
     * @param maxlen is the maximum length in ems the modified string will have
     * If the original string is shorter than "maxlen", it is returned verbatim
     * @return the modified string
     * @since 3.2
     */
    static QString rEmSqueeze( const QString & name,
                               const QFontMetrics& fontMetrics,
                               uint maxlen = 30 );

    /** Substitute characters at the end of a string by "...". Similar to
     * method above, except that maxlen is the width in pixels to truncate to
     * @param name is the string to modify
     * @param fontMetrics is the font metrics to use to calculate character sizes
     * @param maxPixels is the maximum pixel length the modified string will have
     * If the original string is shorter than "maxlen", it is returned verbatim
     * @return the modified string
     * @since 3.2
     */
    static QString rPixelSqueeze( const QString & name,
                                  const QFontMetrics& fontMetrics,
                                  uint maxPixels );

    /**
     * Match a filename.
     * @param filename is the real decoded filename (or dirname
     *        without trailing '/').
     * @param pattern is a pattern like *.txt, *.tar.gz, Makefile.*, *README*, etc.
     * Patterns with two asterisks like "*.*pk" are not supported.
     * @return true if the given filename matches the given pattern
     */
    static bool matchFileName( const QString& filename, const QString& pattern );
    // KDE4: move to KShell

    /**
     * Split a QString into a QStringList in a similar fashion to the static
     * QStringList function in Qt, except you can specify a maximum number
     * of tokens. If max is specified (!= 0) then only that number of tokens
     * will be extracted. The final token will be the remainder of the string.
     *
     * Example:
     * \code
     * perlSplit("__", "some__string__for__you__here", 4)
     * QStringList contains: "some", "string", "for", "you__here"
     * \endcode
     *
     * @param sep is the string to use to delimit s.
     * @param s is the input string
     * @param max is the maximum number of extractions to perform, or 0.
     * @return A QStringList containing tokens extracted from s.
     */
    static QStringList perlSplit
      (const QString & sep, const QString & s, uint max = 0);

    /**
     * Split a QString into a QStringList in a similar fashion to the static
     * QStringList function in Qt, except you can specify a maximum number
     * of tokens. If max is specified (!= 0) then only that number of tokens
     * will be extracted. The final token will be the remainder of the string.
     *
     * Example:
     * \code
     * perlSplit(' ', "kparts reaches the parts other parts can't", 3)
     * QStringList contains: "kparts", "reaches", "the parts other parts can't"
     * \endcode
     *
     * @param sep is the character to use to delimit s.
     * @param s is the input string
     * @param max is the maximum number of extractions to perform, or 0.
     * @return A QStringList containing tokens extracted from s.
     */
    static QStringList perlSplit
      (const QChar & sep, const QString & s, uint max = 0);

    /**
     * Split a QString into a QStringList in a similar fashion to the static
     * QStringList function in Qt, except you can specify a maximum number
     * of tokens. If max is specified (!= 0) then only that number of tokens
     * will be extracted. The final token will be the remainder of the string.
     *
     * Example:
     * \code
     * perlSplit(QRegExp("[! ]", "Split me up ! I'm bored ! OK ?", 3)
     * QStringList contains: "Split", "me", "up ! I'm bored, OK ?"
     * \endcode
     *
     * @param sep is the regular expression to use to delimit s.
     * @param s is the input string
     * @param max is the maximum number of extractions to perform, or 0.
     * @return A QStringList containing tokens extracted from s.
     */
    static QStringList perlSplit
      (const QRegExp & sep, const QString & s, uint max = 0);

    /**
     * This method auto-detects URLs in strings, and adds HTML markup to them
     * so that richtext or HTML-enabled widgets (such as KActiveLabel)
     * will display the URL correctly.
     * @param text the string which may contain URLs
     * @return the resulting text
     * @since 3.1
     */
    static QString tagURLs( const QString& text );

    /**
      Obscure string by using a simple symmetric encryption. Applying the
      function to a string obscured by this function will result in the original
      string.

      The function can be used to obscure passwords stored to configuration
      files. Note that this won't give you any more security than preventing
      that the password is directly copied and pasted.

      @param str string to be obscured
      @return obscured string
      @since 3.2
    */
    static QString obscure( const QString &str );

    /**
      Guess whether a string is UTF8 encoded.

      @param str the string to check
      @return true if UTF8. If false, the string is probably in Local8Bit.
      @since 3.2
     */
    static bool isUtf8( const char *str );

    /**
      Construct QString from a c string, guessing whether it is UTF8- or
      Local8Bit-encoded.

      @param str the input string
      @return the (hopefully correctly guessed) QString representation of @p str
      @since 3.2
     */
    static QString from8Bit( const char *str );

#ifdef KDE_NO_COMPAT
private:
#endif
    /**
     * @deprecated Use matchFileName () instead.
     */
    static KDE_DEPRECATED bool matchFilename( const QString& filename, const QString& pattern )
    {
        return matchFileName (filename, pattern);
    };

};
#endif
