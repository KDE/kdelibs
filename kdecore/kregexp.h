#ifndef __kregexp_h__
#define __kregexp_h__

class KRegExpPrivate;

/**
 * Regular expression matching with back-references. This was implemented
 * because @ref QRegExp does not support backrefs.
 *
 * Back-references are parts of a regexp grouped with parentheses. If a
 * string matches the regexp, you can access the text that matched each
 * group with the @ref group method. This is similar to regexps in perl.
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
   * @return true on match, false otherwise.
   */
  bool match( const char *_string );


  /**
   * Return a grouped substring.
   */
  const char *group( int _grp );
  
protected:
  KRegExpPrivate *m_pPrivate;
};


#endif
