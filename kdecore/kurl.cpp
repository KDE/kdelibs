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

#if 0
#define debugC debug
#else
inline void debugC(const char *,...) {};
#endif

struct KURL_intern {
    QString protocol_part;
    QString host_part;
    QString path_part;
    QString path_part_decoded;
    QString ref_part;
    // This variable is only valid after calling 'directory'.
    QString dir_part;
    QString user_part;
    QString passwd_part;
    int paths;
};

void KURL::encodeURL( QString& _url ) {

    static int count = 0;
    debugC("encode %d",++count);

    int old_length = _url.length();

    
    if (!old_length)
	return;
   
    debugC("encode %s", _url.data());
   
    // a worst case approximation
    char *new_url = new char[ old_length * 3 + 1 ];
    int new_length = 0;
     
    for (int i = 0; i < old_length; i++) {
       // 'unsave' and 'reserved' characters
       // according to RFC 1738,
       // 2.2. URL Character Encoding Issues (pp. 3-4)
	if ( strchr("<>#@\"&%$:,;?={}|^~[]\'`\\", _url[i]) ) {
	    new_url[ new_length++ ] = '%';

	    char c = _url[ i ] / 16;
	    c += (c > 9) ? ('A' - 10) : '0';
	    new_url[ new_length++ ] = c;

	    c = _url[ i ] % 16;
	    c += (c > 9) ? ('A' - 10) : '0';
	    new_url[ new_length++ ] = c;
	    
	} else
	    new_url[ new_length++ ] = _url[i];
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
    static int count = 0;
    
    int old_length = _url.length();
    if (!old_length)
	return;
    
    debugC("decode %s %d", _url.data(), ++count);
    int new_length = 0;

    // make a copy of the old one
    char *new_url = new char[ old_length + 1];
        
    for (int i = 0; i < old_length; i++) 
	{
	    uchar character = _url[ i ];
	    if ( character == '%' ) {
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
    data->protocol_part.detach();
    data->host_part.detach();
    data->path_part.detach();
    data->ref_part.detach();
    /* temporarily removed */
    // data->dir_part.detach();
    data->user_part.detach();
    data->passwd_part.detach();
    data->path_part_decoded.detach();
}

KURL::KURL() 
{ 
    data = new KURL_intern;
    debugC("c1 %p %p",data, this);
    malformed = true;
    data->protocol_part = "";
    data->host_part = ""; 
    data->path_part = ""; 
    data->ref_part = ""; 
    data->paths = 0;
    bNoPath = false;
}


KURL::KURL( KURL & _base_url, const char * _rel_url )
{
    data = new KURL_intern;
    debugC("c2 %p %p",data, this);
    malformed = _base_url.malformed;
    data->protocol_part = _base_url.data->protocol_part;
    data->host_part = _base_url.data->host_part;
    port_number = _base_url.port_number;
    data->path_part = _base_url.data->path_part;
    data->ref_part = _base_url.data->ref_part;
    data->dir_part = _base_url.data->dir_part;
    data->user_part = _base_url.data->user_part;
    data->passwd_part = _base_url.data->passwd_part;
    data->paths = 0;
    detach();
    
    if ( strstr( _rel_url, ":/" ) == 0 )
	cd( _rel_url );
    else
	parse( _rel_url );
}

KURL::KURL( const char* _url)
{
    data = new KURL_intern;
    debugC("c3 %p %p",data, this);
    data->paths = 0;
    parse( _url );
}

void KURL::parse( const char * _url )
{
    static uint count = 0;
    debugC("parse() %d %s",++count, _url);

    QString url(_url);
    malformed = false;

    if ( _url[0] == '/' )
    {
	// Create a light weight URL with protocol
	data->path_part = url;
	data->path_part.detach();
	data->protocol_part = "file";
	return;
    }
    
    // We need a : somewhere to determine the protocol
    int pos = url.find( ":" );
    if ( pos == -1 )
    {
	malformed = true;
	return;
    }
    data->protocol_part = url.left( pos );

    if ( data->protocol_part == "info" || data->protocol_part == "mailto" || 
	 data->protocol_part == "man" || data->protocol_part == "news" )
    {
	data->path_part = url.mid( pos + 1, url.length() );
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
	    data->host_part = url.mid( pos + 1, url.length() );
	    pos2 = url.length();
	}
	else
	    data->host_part = url.mid( pos + 1, 
				 (( pos2 == -1) ? url.length() : pos2) 
				 - pos - 1);      
    }
    else
    {
	data->host_part = "";
	// Go back to the '/'
	pos2 = pos - 1;
    }

    if ( data->host_part.length() > 0 )
    {    
	int j = data->host_part.find( "@" );
	if ( j != -1 )
	{	
	    int i = data->host_part.find( ":" );
	    if ( i != -1 && i < j )
	    {
		data->user_part = data->host_part.left( i );
		data->passwd_part = data->host_part.mid( i + 1, j - i - 1 );
		data->host_part = data->host_part.mid( j + 1, data->host_part.length() );
	    }
	    else
	    {
		data->user_part = data->host_part.left( j );
		data->passwd_part = "";
		data->host_part = data->host_part.mid( j + 1, data->host_part.length() );
	    }
	}
	else
	{
	    data->passwd_part = "";
	    data->user_part = "";
	}
    }
    else
    {
	data->passwd_part = "";
	data->user_part = "";
    }
  
    // find a possible port number
    int p = data->host_part.find(":");
    if ( p != -1 )
    {
	port_number = data->host_part.right( data->host_part.length() - (p + 1) ).toInt();
	data->host_part = data->host_part.left( p );
    }
    else
	port_number = 0;
    
    // Find the path
    if( pos2 < static_cast<int>(url.length()) && pos2 != -1)
    {
	QRegExp exp( "[a-zA-Z]+:" );
	int pos3 = url.findRev( '#' );
	// Is there a) no reference or b) only a subprotocol like file:/tmp/arch.tgz#tar:/usr/
	if ( pos3 == -1 || exp.match( url, pos3 + 1 ) != -1 )
	    data->path_part = url.mid( pos2, url.length() );
	else 
	{
	    data->path_part = url.mid( pos2, pos3 - pos2 );
	    data->ref_part = url.mid( pos3 + 1, url.length() );
	    // if (data->path_part.right(1) == "/")  no filename and a reference
	    // malformed = true;
	}
	bNoPath = false;
    }
    else
    {
	data->path_part = "/";
	// Indicate that we did not see a path originally
	bNoPath = true;
	data->ref_part = "";
    } 

    cleanPath();
    
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
    data = new KURL_intern;
    debugC("c4 %p %p",data, this);
    data->protocol_part = _protocol;
    data->host_part = _host;
    data->path_part = _path;
    data->ref_part  = _ref;
    data->paths = 0;
    malformed = false;
}     

bool KURL::hasSubProtocol()
{
    return ( data->path_part.isNull() && 
	     strchr( data->path_part, '#' ) != 0L );
}

const char* KURL::directory( bool _trailing )
{
    // Calculate only on demand
    if ( data->path_part.right( 1 )[0] == '/' )
	data->dir_part = data->path_part.copy();
    else
    {
	QString p = data->path_part;
	if ( !_trailing )
	    if ( p.right( 1 )[0] == '/' )
		p = p.left( p.length() - 1 );
	int i = p.findRev( "/" );
	if ( i == -1 )
	    // Should never happen
	    data->dir_part = "/";
	else
	    data->dir_part = p.left( i + 1 );
    }

    return data->dir_part.data();
}

const char* KURL::host() const 
{
    if (data->host_part.isNull()) 
	return "";
    else 
	return data->host_part.data();
}

KURL::~KURL() {
    debugC("~ %p %p %d",this, data, data->paths);
    delete data;
}

const char* KURL::path() const 
{ 
    data->paths++;
    static uint count = 0;
    debugC("path() %d",++count);

    if (data->path_part.isNull()) 
	return "";
    else {
	if (data->path_part_decoded.isNull()) {
	    data->path_part_decoded = data->path_part.copy();
	    KURL::decodeURL(data->path_part_decoded);
	}
	debugC("path return \"%s\"",data->path_part_decoded.data());
	return data->path_part_decoded.data();
    }
}

const char* KURL::protocol() const 
{ 
    if (data->protocol_part.isNull()) 
	return ""; 
    else 
	return data->protocol_part.data(); 
}

void KURL::setProtocol( const char* newProto) 
{ 
    data->protocol_part = newProto; 
}

const char* KURL::reference() const 
{ 
    if (data->ref_part.isNull()) 
	return "";
    else 
	return data->ref_part.data(); 
}

const char* KURL::user() 
{ 
    if (data->user_part.isNull()) 
	return "";
    else 
	return data->user_part.data(); 
}

unsigned int KURL::port() const 
{
    return port_number;
}

const char* KURL::passwd() 
{ 
    if (data->passwd_part.isNull()) 
	return "";
    else 
	return data->passwd_part.data(); 
}

void KURL::setPassword( const char *password )
{
    data->passwd_part = password;
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
	data->dir_part = u.data();
    else
    {
	if ( !_trailing && u.right( 1 ) == "/" && u.right(2) != ":/" )
	    u.truncate( u.length() -1 );
	int i = u.findRev( "/" );
	if ( i == -1 )
	    // Should never happen
	    data->dir_part = "/";
	else
	    data->dir_part = u.left( i + 1 );
    }

    return data->dir_part.data();
}

QString KURL::url() const
{
    static int count = 0;
    debugC("url() %d",++count);

    QString url = data->protocol_part.copy();

    if( !data->host_part.isEmpty() ) 
    {
	url += "://";   
	if ( !data->user_part.isEmpty() )
	{
	    url += data->user_part.data();
	    if ( !data->passwd_part.isEmpty() )
	    {
		url += ":";
		url += data->passwd_part.data();
	    }      
	    url += "@";
	}    
	url += data->host_part;
	
	if ( port_number != 0 )
	{
	    QString tmp(url.data());
	    url.sprintf("%s:%d",tmp.data(),port_number);
	}
    }
    else
	url += ":";
    
    if( !data->path_part.isEmpty() && hasPath() )
	url += data->path_part; 
    
    if( !data->ref_part.isEmpty() )
	url += "#" + data->ref_part;
    
    return url;
}

const char* KURL::filename()
{
    if ( data->path_part.isEmpty() )
	return "";
    
    if ( data->path_part.data() == "/")
	return "";
    
    if (data->path_part_decoded.isNull()) {
	data->path_part_decoded = data->path_part.copy();
	KURL::decodeURL(data->path_part_decoded);
    }
    int pos = data->path_part_decoded.findRev( "/" );
    return data->path_part_decoded.data() + pos + 1;
}
    
bool KURL::cd( const char* _dir, bool zapRef)
{
    if ( !_dir )
      return false;
    
    // Now we have a path for shure
    if ( _dir[0] == 0 )
	bNoPath = true;
    else
	bNoPath = false;

    if( _dir[0] == '/' )
    {
	data->path_part = _dir;
    }
    else if ( _dir[0] == '~' )
    {
	if ( data->protocol_part != "file" )
	    return false;
	
	data->path_part = getenv( "HOME" );
	data->path_part += "/";
	data->path_part += _dir + 1;
    }
    else
    {
	if ( data->path_part.right(1)[0] != '/' && _dir[0] != '/' )
	    data->path_part += "/";
	data->path_part += _dir;
    }

    if ( zapRef )
	setReference( "" );

    cleanPath();
    
    return true;
}

bool KURL::setReference( const char* _ref)
{
    // We cant have a referece if we have no path (other than /)
    // if( data->path_part.isNull() || data->path_part.data()[0] == 0 )
    // return false;
    data->ref_part = _ref;
    return true;
}

KURL& KURL::operator=( const KURL &u)
{
  malformed = u.malformed;
  data->protocol_part = u.data->protocol_part;
  data->host_part = u.data->host_part;
  data->path_part = u.data->path_part;
  data->ref_part = u.data->ref_part;
  bNoPath = u.bNoPath;
  
  detach();
  return *this;
}

KURL& KURL::operator=( const char *_url )
{
    debugC("= %p %p",data, this);

    // this is of course a big fat memory leak, but
    // if I remove it, it will end up in a FMW and I
    // have no idea why. if someone knows, please try
    // to remove it
    data = new KURL_intern();
    data->paths = 0;
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
    if ( data->path_part.isEmpty() )
	return;

    // Did we have a trailing '/'
    int len = data->path_part.length();
    bool slash = false;
    if ( len > 0 && data->path_part.right(1)[0] == '/' )
	slash = true;
    
    data->path_part = QDir::cleanDirPath( data->path_part );

    // Restore the trailing '/'
    len = data->path_part.length();
    if ( len > 0 && data->path_part.right(1)[0] != '/' && slash )
	data->path_part += "/";
}

bool KURL::isLocalFile() 
{
    if (data->protocol_part != "file")
	return false;

    if (hasSubProtocol())
	return false;

    return data->host_part.isEmpty();
}
