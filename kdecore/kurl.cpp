// -*-C++-*-
// KURL
//
// Mon Nov 25 10:07:17 1996 -- Steffen Hansen

// Reference: RFC 1738 Uniform Resource Locators
// Syntax?
// TODO: user, password, port

#include "kurl.h"
#include <iostream.h>

void
KURL::detach()
{
  protocol_part.detach();
  host_part.detach();
  path_part.detach();
  ref_part.detach();
  /* temporarily removed */
  // dir_part.detach();
  user_part.detach();
  passwd_part.detach();
}

static void split1(QString _in, char* sep, QString* _out1, QString* _out2)
  /* <_in> -> [<_out1><sep>]<out2> */
{
}

static void split2(QString _in, char* sep, QString* _out1, QString* _out2)
  /* <_in> -> <_out1>[<sep><out2>] */
{
}

KURL::KURL( KURL & _base_url, const char * _rel_url )
{
  malformed = _base_url.malformed;
  protocol_part = _base_url.protocol_part;
  host_part = _base_url.host_part;
  path_part = _base_url.path_part;
  ref_part = _base_url.ref_part;
  dir_part = _base_url.dir_part;
  user_part = _base_url.user_part;
  passwd_part = _base_url.passwd_part;
  detach();
    
  if ( strstr( _rel_url, ":/" ) == 0 )
	cdPath( _rel_url );
  else
	parse( _rel_url );
}

KURL::KURL( const char* _url)
{
  parse( _url );
}

void KURL::parse( const char * _url )
{
  QString url(_url);
  url.detach();
  malformed = FALSE;

  if ( _url[0] == '/' )
      url.sprintf( "file:%s", _url );
  
  // We need a : somewhere to determine the protocol
  int pos = url.find( ":" );
  if ( pos == -1 )
    {
	  malformed = TRUE;
	  return;
    }
  protocol_part = url.left( pos );

  if ( protocol_part == "info" || protocol_part == "mailto" || protocol_part == "man" )
    {
	  path_part = url.mid( pos + 1, url.length() );
	  detach();
	  return;
    }
    
  if ( (int)url.length() < pos + 2 )
    {
	  malformed = TRUE;
	  return;
    }

  if ( strncmp( url.data() + pos, ":/", 2 ) != 0 )
	{
	  malformed = TRUE;
	  return;
	}
  pos += 2;
  int pos2;
  // Is it a local file or what
  if( url.data()[pos] == '/')
  {
    pos2 = url.find( '/', pos + 1);
    if ( pos2 == -1 )
    {
      host_part = url.mid( pos + 1, url.length() );
      pos2 = url.length();
    }
    else
      host_part = url.mid( pos + 1, (( pos2 == -1)?url.length():pos2) - pos - 1);      
  }
  else
  {
	host_part = "";
	// Go back to the '/'
	pos2 = pos - 1;
  }

  if ( host_part.length() > 0 )
  {    
    int j = host_part.find( "@" );
    if ( j != -1 )
    {	
      int i = host_part.find( ":" );
      if ( i != -1 && i < j )
      {
	user_part = host_part.left( i );
	passwd_part = host_part.mid( i + 1, j - i - 1 );
	host_part = host_part.mid( j + 1, host_part.length() );
      }
      else
      {
	user_part = host_part.left( j );
	passwd_part = "";
	host_part = host_part.mid( j + 1, host_part.length() );
      }
    }
    else
    {
      passwd_part = "";
      user_part = "";
    }
  }
  else
  {
    passwd_part = "";
    user_part = "";
  }
  
  // Find the path
  if( pos2 < (int)url.length() && pos2 != -1)
  {
    int pos3 = url.find( '#', pos2 );
    // Is there a reference ?
    if ( pos3 == -1 )
      path_part = url.mid( pos2, url.length() );
    else 
    {
      path_part = url.mid( pos2, pos3 - pos2 );
      ref_part = url.mid( pos3 + 1, url.length() );
    }
  }
  else
  {
	path_part = "/";
	ref_part = "";
  } 
  
  /* ip-schemepart, login, see RFC1738                   */
  /* Syntax [<user>[":"<password>]"@"]<host>[":"<port>]] */
  /* Note that both user and password may be encoded.    */
  // login    -> [userpart@]hostpart 
  // hostpart -> Host[:Port]
  // userpart -> User[:Pass]
  detach();
}

KURL::KURL( const char* _protocol, const char* _host, 
			const char* _path, const char* _ref)
{
  protocol_part = _protocol;
  host_part = _host;
  path_part = _path;
  ref_part  = _ref;
  malformed = FALSE;
  detach();
  cleanURL();
}     

const char* KURL::directory( bool _trailing )
{
    // Calculate only on demand
    if ( path_part.right( 1 ) == "/" )
	dir_part = path_part.data();
    else
    {
	QString p = path_part;
	if ( !_trailing )
	    if ( p.right( 1 ) == "/" )
		p = p.left( p.length() - 1 );
	int i = p.findRev( "/" );
	if ( i == -1 )
	    // Should never happen
	    dir_part = "/";
	else
	    dir_part = p.left( i + 1 );
    }

    return dir_part.data();
}

const char* KURL::directoryURL( bool _trailing )
{
    QString u = url();
    
    // Calculate only on demand
    if ( u.right( 1 ) == "/" )
	dir_part = u.data();
    else
    {
	if ( !_trailing )
	    if ( u.right( 1 ) == "/" )
		u = u.left( u.length() - 1 );
	int i = u.findRev( "/" );
	if ( i == -1 )
	    // Should never happen
	    dir_part = "/";
	else
	    dir_part = u.left( i + 1 );
    }

    return dir_part.data();
}

QString
KURL::url() const
{
  QString url( protocol_part.data());
  url.detach();
  if( !host_part.isNull() && host_part.data()[0] != 0) 
  {
    url += "://";   
    if ( !user_part.isNull() && user_part.data()[0] != 0 )
    {
      url += user_part.data();
      if ( !passwd_part.isNull() && passwd_part.data()[0] != 0 )
      {
	url += ":";
	url += passwd_part.data();
      }      
      url += "@";
    }    
    url += host_part;
  }
  else
    url += ":";
	
  if( !path_part.isNull() && path_part.data()[0] != 0)
	url += path_part; 
  if( !ref_part.isNull() && ref_part.data()[0] != 0)
	url += "#" + ref_part;

  return url;
}

static inline QString cleanPath( const char* filePath)
{
  QString tmp( QDir::cleanDirPath( filePath) );
  if( tmp == "." || tmp == "/")
	tmp = "";
  return tmp;
}

void
KURL::cleanURL()
{
  path_part = cleanPath( path_part);
  if( path_part.data()[0] != '/')
      path_part.prepend("/");
  protocol_part = cleanPath( protocol_part);
  host_part     = cleanPath( host_part);
  ref_part      = cleanPath( ref_part);
}

const char* KURL::filename( bool _isReference )
{
  if ( _isReference )
    {
	  int pos = ref_part.findRev( "/" );
	  return ref_part.data() + pos + 1;
    }
    
  if ( path_part.data()[0] == 0 )
	return "";

  if ( path_part.data()[0] == '/' && path_part.data()[1] == 0 )
	return "";
    
  int pos = path_part.findRev( "/" );
  return path_part.data() + pos + 1;
}
    
bool
KURL::cdRef( const char* _ref, bool acceptAbsPath)
{
  // We cant have a referece if we have no path (other than /)
  if( path_part.isNull() || path_part.data()[0] == 0
	  || path_part.data()[0] == '/')
	return false;
  if( _ref[0] == '/' && acceptAbsPath) {
	ref_part  = _ref + 1;
  } else {
	if( ref_part.data()[0] != 0 
		&& ref_part.data()[ref_part.length()] != '/')
	  ref_part += "/";
	ref_part += _ref;
  }
  cleanURL();
  return true;
}

bool KURL::cdPath( const char* _dir, bool acceptAbsPath, bool zapRef)
{
  if ( !_dir )
      return FALSE;
    
  if( _dir[0] == '/' && acceptAbsPath )
  {
      path_part = _dir;
  }
  else if ( _dir[0] == '~' )
  {
      path_part = QDir::homeDirPath();
      path_part.detach();
      path_part += _dir + 1;
  }
  else
  {
      path_part += "/";
      path_part += _dir;
  }
  if( zapRef )
	setReference( "" );
  cleanURL();
  return true;
}

bool
KURL::setReference( const char* _ref)
{
  // We cant have a referece if we have no path (other than /)
  if( path_part.isNull() || path_part.data()[0] == 0 )
	return false;
  ref_part = _ref;
  return true;
}
bool
KURL::cd( const char* dir, bool acceptAbsPath, bool isReference)
{
  if( isReference)
	return cdRef( dir, acceptAbsPath);
  else
	return cdPath( dir, acceptAbsPath);
}

KURL&
KURL::operator=( const KURL &u)
{
  malformed = u.malformed;
  protocol_part = u.protocol_part;
  host_part = u.host_part;
  path_part = u.path_part;
  ref_part = u.ref_part;
  detach();
  return *this;
}

KURL& KURL::operator=( const char *_url )
{
  parse( _url );
  return *this;
}
