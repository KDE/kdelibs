#ifndef __kregexp_h__
#define __kregexp_h__

class KRegExpPrivate;

/**
 * Regular expression (regexp) matching with back-references. 
 * This was implemented
 * because @ref QRegExp does not support back-references.
 *
 * Back-references are parts of a regexp grouped with parentheses. If a
 * string matches the regexp, you can access the text that matched each
 * group with the @ref group method. This is similar to regexps in perl.
 *
 * Example:
 * <pre>
 *  KRegExp ex( "([A-Za-z]+) (.+)" );
 *  ex.match( "42 Torben Weis" );
 *  qDebug( ex.group(0) );
 *  qDebug( ex.group(1) );
 *  qDebug( ex.group(2) );
 * </pre>
 * Output:
 * <pre>
 *  Torben Weis
 *  Torben
 *  Weis
 * </pre>
 *
 * Please notice that KRegExp does @bf not support unicode.
 *
 * @author Torben Weis <weis@kde.org>
 * @version $Id$
 */
class KRegExp
{
public:

  /**
   * Create a KRegExp object without a default pattern.
   */
  KRegExp();

  /**
   * Create a KRegExp object.
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
   */
  bool compile( const char *_pattern, const char *_mode = "" );

  /**
   * Match a string to the last supplied regexp.
   * @return @p true on match, false otherwise.
   */
  bool match( const char *_string );


  /**
   * @return a grouped substring. A substring may be empty.
   *         In this case 0 is returned. Otherwise you may @bf not
   *         delete or modify the returned value. In addition the
   *         returned value becomes invalid after the KRegExp instance
   *         is deleted or after @ref @match() was called again.
   *
   * @param _grp May be in the range [0..9]. If @p _grp is 0 then the complete
   *             matched string is returned.
   */
  const char *group( int _grp );

  /**
   * @return The start offset of the grouped substring.
   *
   * @param _grp May be in the range [0..9]. If @p _grp is 0 then the start offset
   *             of the complete matched string is returned.
   */
  int groupStart( int _grp );
  /**
   * @return The end offset of the grouped substring. The "end offset" is the first
   *         character after the string.
   *
   * @param _grp May be in the range [0..9]. If @p _grp is 0 then the end offset
   *             of the complete matched string is returned.
   */
  int groupEnd( int _grp );

protected:
  KRegExpPrivate *m_pPrivate;
};


#endif
