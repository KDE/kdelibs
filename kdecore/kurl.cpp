// -*-C++-*-
// KURL
//
// Mon Nov 25 10:07:17 1996 -- Steffen Hansen

// Reference: RFC 1738 Uniform Resource Locators
// Syntax?

#include "kurl.h"
#include <iostream.h>
#include <qregexp.h>

void KURL::encodeURL( QString& _url ) {
    
    int old_length = _url.length();
   
    // a worst case approximation
    char *new_url = new char[ old_length * 3 + 1 ];
    int new_length = 0;
     
    for (int i = 0; i < old_length; i++) {
	if ( strchr("#@\"&%$:,;?={}|^~[]\'", _url[i]) ) {
	    new_url[ new_length++ ] = '%';

	    char c = _url[ i ] / 16;
	    c += (c > 9) ? ('A' - 10) : '0';
	    new_url[ new_length++ ] = c;

	    c = _url[ i ] % 16;
	    c += (c > 9) ? ('A' - 10) : '0';
	    new_url[ new_length++ ] = c;
	    
	} else
	    new_url[ new_length++ ] = _url[i];
    };

    new_url[new_length]=0;
    _url = new_url;
    delete new_url;
};

static uchar hex2int( char _char ) {
    if ( _char >= 'A' && _char <='F')
	return _char - 'A' + 10;
    if ( _char >= 'a' && _char <='f')
	return _char - 'a' + 10;
    if ( _char >= '0' && _char <='9')
	return _char - '0';
    return 0;
};

void KURL::decodeURL( QString& _url ) {
    int old_length = _url.length();
    int new_length = 0;

    // make a copy of the old one
    char *new_url = new char[ old_length + 1];
        
    for (int i = 0; i < old_length; i++) 
	{
	    uchar character = _url[ i ];
	    if ( character == '%' ) {
		character = hex2int( _url[i+1] ) * 16 + hex2int( _url[i+2] );
		i += 2;
	    };
	    new_url [ new_length++ ] = character;
	};
    new_url [ new_length ] = 0;
    _url = new_url;
    delete new_url;
};

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

KURL::KURL() 
{ 
  malformed = true;
  protocol_part = "";
  host_part = ""; 
  path_part = ""; 
  ref_part = ""; 
}


KURL::KURL( KURL & _base_url, const char * _rel_url )
{
  malformed = _base_url.malformed;
  protocol_part = _base_url.protocol_part;
  host_part = _base_url.host_part;
  port_number = _base_url.port_number;
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

  if ( _url[0] == '/' ) {
      // encodeURL(url);
      url.insert( 0, "file:");
  };
  
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
  
  // find a possible port number
  int p = host_part.find(":");
  if ( p != -1 )
  {
      port_number = host_part.right( host_part.length() - (p + 1) ).toInt();
      host_part = host_part.left( p );
  }
  else
      port_number = 0;

  // Find the path
  if( pos2 < (int)url.length() && pos2 != -1)
  {
      QRegExp exp( "[a-zA-Z]+:" );
      int pos3 = url.findRev( '#' );
      // Is there a) no reference or b) only a subprotocol like file:/tmp/arch.tgz#tar:/usr/
      if ( pos3 == -1 || exp.match( url, pos3 + 1 ) != -1 )
	  path_part = url.mid( pos2, url.length() );
      else 
      {
	  path_part = url.mid( pos2, pos3 - pos2 );
	  ref_part = url.mid( pos3 + 1, url.length() );
	  if (path_part.right(1) == "/") // no filename and a reference
	      malformed = true;
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

bool KURL::hasSubProtocol()
{
    return ( strchr( path(), '#' ) != 0L );
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

char* KURL::host() const 
{
  if (host_part.isNull()) 
    return "";
  else 
    return host_part.data();
}

 
char* KURL::path() const 
{ 
  if (path_part.isNull()) 
    return "";
  else 
    return path_part.data(); 
}

char* KURL::reference() const 
{ 
  if (ref_part.isNull()) 
    return "";
  else 
    return ref_part.data(); 
}

const char* KURL::user() 
{ 
  if (user_part.isNull()) 
    return "";
  else 
    return user_part.data(); 
}

unsigned int KURL::port() const 
{
  return port_number;
}

const char* KURL::passwd() 
{ 
  if (passwd_part.isNull()) 
    return "";
  else 
    return passwd_part.data(); 
}

void KURL::setPassword( const char *password )
{
   passwd_part = password;
}

bool KURL::cdUp( bool zapRef ) 
{
  if( zapRef) 
    setReference("");
  return cd( "..");
}

bool KURL::operator==( const KURL &_url) const
{
   return _url.url() == url();
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
    
    if ( port_number != 0 ) 
	url.sprintf("%s:%d",url.data(),port_number);
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
    QString tmp = QDir::cleanDirPath( filePath).copy();
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
    
  if ( path_part.isEmpty() )
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

QString KURL::parentURL()
{
    QRegExp exp( "[a-zA-Z]+:" );
    QString str = url();
    
    int i = str.length();
    while( ( i = str.findRev( "#", i) ) != -1 )
    {
	if ( exp.match( str.data(), i + 1 ) != -1 )
	    return QString( str.left( i ) );
	i--;
    }
        
    return QString( str.data() );
}

QString KURL::childURL()
{
    QRegExp exp( "[a-zA-Z]+:" );
    QString str = url();
    
    int i = str.length();
    while( ( i = str.findRev( "#", i) ) != -1 )
    {
	if ( exp.match( str.data(), i + 1 ) != -1 )
	    return QString( str.data() + i + 1 );
	i--;
    }

    return QString();
}
    
QString KURL::nestedURL()
{
    QString s = childURL().data();
    if ( s.isEmpty() )
	return url();
    return s;
}
