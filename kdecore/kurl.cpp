// -*-C++-*-
/* This file is part of the KDE libraries
    Copyright (C) 1997 Steffen Hansen (stefh@dit.ou.dk)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
// KURL
// Reference: RFC 1738 Uniform Resource Locators

#include <qdir.h>
#include "kurl.h"
#include <qregexp.h>
#include <stdlib.h>

void KURL::encodeURL( QString& _url ) {

    int old_length = _url.length();

    
    if (!old_length)
	return;
   
    // a worst case approximation
    char *new_url = new char[ old_length * 3 + 1 ];
    int new_length = 0;
     
    for (int i = 0; i < old_length; i++) 
    {
        static char *safe = "$-._!*(),/"; /* RFC 1738 */
        // '/' added by David, fix found by Michael Reiher

        char t = _url[i];

        if ( (( t >= 'A') && ( t <= 'Z')) ||
             (( t >= 'a') && ( t <= 'z')) ||
             (( t >= '0') && ( t <= '9')) ||
             (strchr(safe, t))
           )
	{
	    new_url[ new_length++ ] = _url[i];
	}
	else
	{
	    new_url[ new_length++ ] = '%';

	    unsigned char c = ((unsigned char)_url[ i ]) / 16;
	    c += (c > 9) ? ('A' - 10) : '0';
	    new_url[ new_length++ ] = c;

	    c = ((unsigned char)_url[ i ]) % 16;
	    c += (c > 9) ? ('A' - 10) : '0';
	    new_url[ new_length++ ] = c;
	    
	} 
    }

    new_url[new_length]=0;
    _url = new_url;
    delete [] new_url;
}

static uchar hex2int( char _char ) {
    if ( _char >= 'A' && _char <='F')
	return _char - 'A' + 10;
    if ( _char >= 'a' && _char <='f')
	return _char - 'a' + 10;
    if ( _char >= '0' && _char <='9')
	return _char - '0';
    return 0;
}

void KURL::decodeURL( QString& _url ) {

    int old_length = _url.length();
    if (!old_length)
	return;
    
    int new_length = 0;

    // make a copy of the old one
    char *new_url = new char[ old_length + 1];
        
    for (int i = 0; i < old_length; i++) 
    {
	uchar character = _url[ i ];
	if ( character == '%' ) 
	{
	    character = hex2int( _url[i+1] ) * 16 + hex2int( _url[i+2] );
	    i += 2;
	}
	new_url [ new_length++ ] = character;
    }
    new_url [ new_length ] = 0;
    _url = new_url;
    delete [] new_url;
}

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
    path_part_decoded.detach();
    search_part.detach();
}

KURL::KURL() 
{ 
    malformed = true;
    protocol_part = "";
    host_part = ""; 
    path_part = ""; 
    ref_part = ""; 
    bNoPath = false;
}


KURL::KURL( KURL & _base_url, const char * _rel_url )
{
    char * pos1 = strchr( _rel_url, ':');
    char * pos2 = strchr( _rel_url, '/');
    
    // A full URL has a ':' and no '/' in front of the ':'

    if ( (pos1 != 0) && 
         ( (pos2 == 0) || (pos2 > pos1) ) 
       )
    {
    	// Full URL
        parse( _rel_url );
    }
    else
    {
	// Relative URL

        malformed = _base_url.malformed;
        protocol_part = _base_url.protocol_part;
        host_part = _base_url.host_part;
        port_number = _base_url.port_number;
        path_part = _base_url.path_part;
        path_part_decoded = _base_url.path_part_decoded;
        ref_part = _base_url.ref_part;
        dir_part = _base_url.dir_part;
        user_part = _base_url.user_part;
        passwd_part = _base_url.passwd_part;
        bNoPath = _base_url.bNoPath;
        detach();

	cd( _rel_url );
    } 
}

KURL::KURL( const char* _url)
{
    parse( _url );
}

void KURL::parse( const char * _url )
{
    QString url(_url);
    // defaults
    malformed = false;
    path_part_decoded = 0;
    search_part = 0;
    ref_part = "";
    bNoPath = false;

    if ( _url[0] == '/' )
    {
	// Create a light weight URL with protocol
	path_part_decoded = _url;
	path_part = path_part_decoded.data();
	KURL::encodeURL( path_part );
	protocol_part = "file";
	return;
    }
    
    // We need a : somewhere to determine the protocol
    int pos = url.find( ":" );
    if ( pos == -1 )
    {
	malformed = true;
	return;
    }
    protocol_part = url.left( pos ).lower();

    if ( protocol_part == "info" || protocol_part == "mailto" || 
	 protocol_part == "man" || protocol_part == "news" )
    {
	path_part = url.mid( pos + 1, url.length() );
	detach();
	return;
    }
    
    // Is there something behind "protocol:" ?
    // The minimal valid URL is "file:/"
    if ( static_cast<int>(url.length()) < pos + 2 )
    {
	malformed = true;
	return;
    }

    if ( strncmp( url.data() + pos, ":/", 2 ) != 0 )
    {
	malformed = true;
	return;
    }

    pos += 2;
    int pos2;
    // Do we have a host part ?
    if ( url.data()[pos] == '/' )
    {
	// Find end of host string
	pos2 = url.find( '/', pos + 1);
	// We dont have a path ?
	if ( pos2 == -1 )
	{
	    host_part = url.mid( pos + 1, url.length() );
	    pos2 = url.length();
	}
	else
	{
	    host_part = url.mid( pos + 1, 
				 (( pos2 == -1) ? url.length() : pos2) 
				 - pos - 1);
	}
				 
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
    {
	port_number = 0;
    }
    
    // Find the path
    if( pos2 < static_cast<int>(url.length()) && pos2 != -1)
    {
	QRegExp exp( "[a-zA-Z]+:" );
	int pos3 = url.findRev( '#' );
	// Is there a) no reference or b) only a subprotocol like file:/tmp/arch.tgz#tar:/usr/
	if ( pos3 == -1 || exp.match( url, pos3 + 1 ) != -1 )
	{
	    path_part = url.mid( pos2, url.length() );
        }
	else if ( pos3 > pos2 ) 
	{
	    path_part = url.mid( pos2, pos3 - pos2 );
	    ref_part = url.mid( pos3 + 1, url.length() );
	    // if (path_part.right(1) == "/")  no filename and a reference
	    // malformed = true;
	}
	else
	{
	    malformed = true;
	    return;
	}
	bNoPath = false;
    }
    else
    {
	path_part = "/";
	// indicate that we did not see a path originally
	bNoPath = true;
	ref_part = "";
    } 

    if ((protocol_part == "http") || (protocol_part == "imap4") || (protocol_part == "pop3"))
    {
        p = path_part.find('?');
        if (p != -1)
        {
            search_part = path_part.mid( p + 1, path_part.length() );
            path_part = path_part.left( p);
        }
    }
    else
    {
      cleanPath();
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
    malformed = false;
}     

bool KURL::hasSubProtocol()
{
    return ( !path_part.isNull() && 
	     strchr( path_part, '#' ) != 0L );
}

const char* KURL::directory( bool _trailing )
{
    // Calculate only on demand
    if ( path_part.right( 1 )[0] == '/' )
	dir_part = path_part.copy();
    else
    {
	QString p = path_part;
	if ( !_trailing )
	    if ( p.right( 1 )[0] == '/' )
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

const char* KURL::host() const 
{
    if (host_part.isNull()) 
	return "";
    else 
	return host_part.data();
}

KURL::~KURL() {

}

const char* KURL::path() const
{ 
    if (path_part.isNull()) 
	return "";
    else {
        KURL *that = const_cast<KURL*>(this);
        if (that->path_part_decoded.isNull()) {
	    that->path_part_decoded = path_part.copy();
	    KURL::decodeURL(that->path_part_decoded);
	}
	return path_part_decoded.data();
    }
}

const char* KURL::httpPath() const
{ 
    if (path_part.isNull()) 
	return "";
    else {
	return path_part.data();
    }
}

const char* KURL::searchPart() const
{ 
    if (search_part.isNull()) 
	return 0L;
    else {
	return search_part.data();
    }
}

const char* KURL::protocol() const 
{ 
    if (protocol_part.isNull()) 
	return ""; 
    else 
	return protocol_part.data(); 
}

void KURL::setProtocol( const char* newProto) 
{ 
    protocol_part = newProto; 
}

void KURL::setSearchPart( const char* _searchPart) 
{ 
    search_part = _searchPart; 
}

const char* KURL::reference() const 
{ 
    if (ref_part.isNull()) 
	return "";
    else 
	return ref_part.data(); 
}

const char* KURL::user() const
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

const char* KURL::passwd() const
{ 
    if (passwd_part.isNull()) 
	return "";
    else 
	return passwd_part.data(); 
}

void KURL::setPath( const char *newPath )
{
	path_part = newPath;
}

void KURL::setHost( const char *newHost )
{
	host_part = newHost;
}

void KURL::setPassword( const char *password )
{
    passwd_part = password;
}

void KURL::setUser( const char *newUser )
{
    user_part = newUser;
}

void KURL::setPort( const unsigned int newPort )
{
    port_number = newPort;
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
    if ( u.right( 1 )[0] == '/' && ( _trailing || u.right(2) == ":/" ) )
	dir_part = u.data();
    else
    {
	if ( !_trailing && u.right( 1 ) == "/" && u.right(2) != ":/" )
	    u.truncate( u.length() -1 );
	int i = u.findRev( "/" );
	if ( i == -1 )
	    // Should never happen
	    dir_part = "/";
	else
	    dir_part = u.left( i + 1 );
    }

    return dir_part.data();
}

QString KURL::url() const
{

    QString url = protocol_part.copy();

    if( !host_part.isEmpty() ) 
    {
	url += "://";   
	if ( !user_part.isEmpty() )
	{
	    url += user_part.data();
	    if ( !passwd_part.isEmpty() )
	    {
		url += ":";
		url += passwd_part.data();
	    }      
	    url += "@";
	}    
	url += host_part;
	
	if ( port_number != 0 )
	{
	    QString tmp(url.data());
	    url.sprintf("%s:%d",tmp.data(),port_number);
	}
    }
    else
	url += ":";
    
    if( !path_part.isEmpty() && hasPath() )
	url += path_part; 

    if( !search_part.isNull())
    {
	if(path_part.isEmpty() || !hasPath() )
	    url += "/";
    	url += "?" + search_part;
    }

    if( !ref_part.isEmpty() )
    {
	if(path_part.isEmpty() || !hasPath() )
	    url += "/";
	url += "#" + ref_part;
    }
    
    return url;
}

const char* KURL::filename()
{
    if ( path_part.isEmpty() )
	return "";
    
    if ( path_part.data() == "/")
	return "";
    
    if (path_part_decoded.isNull()) {
	path_part_decoded = path_part.copy();
	KURL::decodeURL(path_part_decoded);
    }
    int pos = path_part_decoded.findRev( "/" );
    return path_part_decoded.data() + pos + 1;
}
    
bool KURL::cd( const char* _dir, bool zapRef)
{
    if ( !_dir )
	return false;
    
    path_part_decoded = 0;

    // Now we have a path for shure
    bNoPath = ( _dir[0] == 0);

    if( _dir[0] == '/' )
    {
	path_part = _dir;
    }
    else if (( _dir[0] == '~' ) && ( protocol_part == "file" ))
    {
	path_part = getenv( "HOME" );
	path_part += "/";
	path_part += _dir + 1;
    }
    else
    {
	if ( path_part.right(1)[0] != '/' && _dir[0] != '/' )
	    path_part += "/";
	path_part += _dir;
    }

    if ( zapRef )
	setReference( "" );

    cleanPath();
    
    return true;
}

bool KURL::setReference( const char* _ref)
{
    // We cant have a referece if we have no path (other than /)
    // if( path_part.isNull() || path_part.data()[0] == 0 )
    // return false;
    ref_part = _ref;
    return true;
}

KURL& KURL::operator=( const KURL &u)
{
  port_number = u.port_number;
  malformed = u.malformed;
  protocol_part = u.protocol_part;
  host_part = u.host_part;
  path_part = u.path_part;
  search_part = u.search_part;
  ref_part = u.ref_part;
  bNoPath = u.bNoPath;
  path_part_decoded = u.path_part_decoded;
  dir_part = u.dir_part;
  user_part = u.user_part;
  passwd_part = u.passwd_part;
  
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
    QString s = childURL();
    if ( s.isEmpty() )
	return url();
    return s;
}

void KURL::cleanPath()
{
    if ( path_part.isEmpty() )
	return;

    // Did we have a trailing '/'
    int len = path_part.length();
    bool slash = false;
    if ( len > 0 && path_part.right(1)[0] == '/' )
	slash = true;
    
    path_part = QDir::cleanDirPath( path_part );

    // Restore the trailing '/'
    len = path_part.length();
    if ( len > 0 && path_part.right(1)[0] != '/' && slash )
	path_part += "/";
}

bool KURL::isLocalFile() 
{
    if (protocol_part != "file")
	return false;

    if (hasSubProtocol())
	return false;

    return host_part.isEmpty();
}
