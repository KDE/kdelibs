#ifndef __kreg_private_h__
#define __kreg_private_h__

extern "C" { // bug with some libc5 distributions
#include <regex.h>
}

class KRegExpPrivate
{
public:
  KRegExpPrivate();
  KRegExpPrivate( const char *_pattern, const char *_mode = "" );
  ~KRegExpPrivate();
  
  bool compile( const char *_pattern, const char *_mode = "" );

  bool match( const char *_string );
  const char *group( int _grp );
  
protected:
  regex_t m_pattern;
  regmatch_t (m_matches[ 10 ]);
  char* (m_strMatches[10]);
  bool m_bInit;
};

#endif
