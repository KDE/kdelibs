#ifndef __kregexp_h__
#define __kregexp_h__

class KRegExpPrivate;

class KRegExp
{
public:
  KRegExp();
  KRegExp( const char *_pattern, const char *_mode = "" );
  ~KRegExp();
  
  bool compile( const char *_pattern, const char *_mode = "" );

  bool match( const char *_string );
  const char *group( int _grp );
  
protected:
  KRegExpPrivate *m_pPrivate;
};


#endif
