/* This file is part of the KDE libraries
   Copyright (c) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __kregexp_h__
#define __kregexp_h__

class KRegExpPrivate;

/**
 * Regular expression (regexp) matching with back-references.
 *
 * This was implemented
 * because @ref QRegExp does not support back-references.
 *
 * Back-references are parts of a regexp grouped with parentheses. If a
 * string matches the regexp, you can access the text that matched each
 * group with the @ref group method. This is similar to regular expressions 
 * in Perl.
 *
 * Example:
 * <pre>
 *  KRegExp ex( "([A-Za-z]+) (.+)" );
 *  ex.match( "42 Torben Weis" );
 *  kdDebug() << ex.group(0) << endl;
 *  kdDebug() << ex.group(1) << endl;
 *  kdDebug() << ex.group(2) << endl;
 * </pre>
 * Output:
 * <pre>
 *  Torben Weis
 *  Torben
 *  Weis
 * </pre>
 *
 * Please notice that KRegExp does @em not support unicode.
 *
 * @author Torben Weis <weis@kde.org>
 * @version $Id$
 */
class KRegExp
{
public:

  /**
   * Creates a KRegExp object without a default pattern.
   */
  KRegExp();

  /**
   * Creates a KRegExp object.
   * @param _pattern	The regular expression to use for matches.
   * @param _mode	If this is "i", case-insensitive matches will be
   * 			performed.
   */
  KRegExp( const char *_pattern, const char *_mode = "" );
  ~KRegExp();

  /**
   * Prepare a regular expression for subsequent matches.
   * @param _pattern	The regular expression to use for matches.
   * @param _mode	If this is "i", case-insensitive matches will be
   * 			performed.
   * @return bool if successful.
   */
  bool compile( const char *_pattern, const char *_mode = "" );

  /**
   * Match a string to the last supplied regexp.
   * @param _string the string to match
   * @return @p true on match, false otherwise.
   */
  bool match( const char *_string );


  /**
   * Returns a group from the match.
   *
   * @param _grp May be in the range [0..9]. If @p _grp is 0 then the complete
   *             matched string is returned.
   * @return a grouped substring. A substring may be empty.
   *         In this case 0 is returned. Otherwise you may @em not
   *         delete or modify the returned value. In addition the
   *         returned value becomes invalid after the KRegExp instance
   *         is deleted or after @ref match() was called again.
   */
  const char *group( int _grp );

  /**
   * The offset of the given group in the string.
   * @param _grp May be in the range [0..9]. If @p _grp is 0 then the start offset
   *             of the complete matched string is returned.
   * @return The start offset of the grouped substring.
   */
  int groupStart( int _grp );
  /**
   * The offset of the given group's end in the string.
   * @param _grp May be in the range [0..9]. If @p _grp is 0 then the end offset
   *             of the complete matched string is returned.
   * @return The end offset of the grouped substring. The "end offset" is the first
   *         character after the string.
   */
  int groupEnd( int _grp );

private:
  KRegExpPrivate *m_pPrivate;
};


#endif
