// File: ftp_proxy.cpp by Martin Zumkley (uc2n@rzstud1.rz.uni-karlsruhe.de)
//
// additions to work with KFM Proxy Manager by Lars Hoss ( Lars.Hoss@munich.netsurf.de )
// Ported to KDE-2's KIO architecture by David Faure <faure@kde.org>

#include "ftp_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>
#include <unistd.h>
#include <ksimpleconfig.h>

/*****************************************************************************/

ProxyFtp::ProxyFtp ( const QCString &protocol, const QCString &pool, const QCString & app) : HTTPProtocol( protocol, pool, app ) {

//    use_proxy = 0;

    mDefaultPort = 21;

/*
    QString proxyStr;
    QString tmp;
    KURL proxyURL;
    
    // All right. Now read the proxy settings
    KSimpleConfig prxcnf("kfmrc", true );
    prxcnf.setGroup("Browser Settings/Proxy");

    noProxyForStr = prxcnf.readEntry("NoProxyFor");
    
    tmp = prxcnf.readEntry( "UseProxy" );
    if ( tmp == "Yes" ) { // Do we need proxy?
        proxyStr = prxcnf.readEntry( "FTP-Proxy" );
        proxyURL = proxyStr.data();
        // printf( "Using ftp proxy %s on port %d\n", proxyURL.host(), proxyURL.port() );
        port = proxyURL.port();
	if ( port == 0 )
	    port = 80;

	proxy_user = prxcnf.readEntry( "Proxy-User" );
	proxy_pass = prxcnf.readEntry( "Proxy-Pass" );

	init_sockaddr(&proxy_name, proxyURL.host(), port);
	use_proxy = 1;
    }
*/
}

ProxyFtp::~ProxyFtp() {}

/* EmitData is called when the retrieved Data is of mimetype HTML
   Normaly it's after a Directory request
*/
/*
void ProxyFtp::emitData()
{
    char buffer[ 1024 ];
    while ( fgets( buffer, 1024, fsocket ) )
    {
	int n = strlen(buffer);
	if ( n > 0 )
	{
	    buffer[n] = 0;
	    data( QByteArray( buffer, n ) );
	}
    }
}
*/
/* We don't now if the clicked URL is a Directory or not
   so every URL is first assumed to be a directory.
   If the request fails, the Method Process Headers sets the
   VAR "secondtry" to 1 and calls open again without the trailing "/"
   in the URL.
*/
#if 0
void ProxyFtp::listDir( const QString & path )
{
  secondtry = 0;
  openProxy(path,false);
}

void ProxyFtp::get( const QString& path, const QString& /*query*/, bool reload )
{
  openProxy(path,reload);
}

void ProxyFtp::openProxy( const QString& path, bool reload )
{
  bytesRead = 0;
  startTime.start();
  currentTime.start();
  
    // Save the parameter, we could need it if we get HTTP redirection
    // See ::ProcessHeader
  //  currentMode = mode;
    
	if (connected) Close();

	sock = ::socket(PF_INET,SOCK_STREAM,0);

	if (sock < 0)
	{
	    error( ERR_COULD_NOT_CREATE_SOCKET, "" );
	    return;
	}

	int do_proxy = use_proxy;
	if (do_proxy)
	{
       	    if ( ! noProxyForStr.isEmpty() ) 
	    {
	      // printf( "host: %s\n", _url->host() );
	      // printf( "nplist: %s\n", noProxyForStr.data() );
	        do_proxy = !revmatch( _url->host(), noProxyForStr.data() );    
	    }
	}

	if(do_proxy)
	{
	  // printf("FTP::Open: connecting to proxy %s:%d\n",
	  // inet_ntoa(proxy_name.sin_addr),
	  // ntohs(proxy_name.sin_port));
	  if(::connect(sock,(struct sockaddr*)(&proxy_name),sizeof(proxy_name)))
	  {
            error( ERR_COULD_NOT_CONNECT, "..." );
	    return;
	  }
	}
	else
	{
		struct sockaddr_in server_name;
		unsigned short int port = _url->port();
		if ( port == 0 )
			port = 80;

		if(init_sockaddr(&server_name, _url->host(), port) == FAIL)
		{
    		Error(KIO_ERROR_UnknownHost, "Unknown host", errno );
			return(FAIL);
		}

                // printf("ProxyFTP::Open: connecting to %s:%d\n",
		// inet_ntoa(server_name.sin_addr),
		// ntohs(server_name.sin_port));
		if(::connect(sock,(struct sockaddr*)(&server_name),sizeof(server_name)))
		{
	    	Error(KIO_ERROR_CouldNotConnect, "Could not connect host", errno);
			return(FAIL);
		}
 	}
	connected = 1;

	fsocket = fdopen(sock,"r+");
	if(!fsocket)
	{
	    Error(KIO_ERROR_CouldNotConnect, "Could not fdopen socket", errno);
	    return(FAIL);
	}

	QString command;

	if(do_proxy)
	{
		/** add hostname when using proxy **/
		unsigned short int port = _url->port();
		if (! port &&  strcmp(_url->protocol(),"ftp") == 0)  // use default one
			port = 21;

                command.sprintf("GET ftp://%s:%d", _url->host(), port);

	} else {
		command = "GET ";
	}
	
	if ( _url->path()[0] != '/' ) command += "/";
	command += _url->path();

	command += " HTTP/1.0\n"; /* start header */
	command += "User-Agent: Konqueror/1.0"; /* User agent */

	if ( _reload ){ /* No caching for reload */
	  command += "Pragma: no-cache\r\n"; /* for HTTP/1.0 caches */
	  command += "Cache-control: no-cache\r\n"; /* for HTTP/>=1.1 caches */
        }
	
	command += "Host: "; /* support for virtual hosts */
	command += _url->host();
	command += "\n";
 
	if( strlen(_url->user()) != 0 )
	{
		char *www_auth = create_www_auth(_url->user(),_url->passwd());
		command += www_auth;
		free(www_auth);
	}

	if( do_proxy )
	{
	  if( proxy_user != "" && proxy_pass != "" )
          {
	    char *www_auth = create_generic_auth("Proxy-authorization", proxy_user, proxy_pass);
	    command += www_auth;
	    free(www_auth);
	  }
	}

	command += "\n";  /* end header */
	// fprintf(stderr,"Command=%s\n",command.data());

	// write(0, command.data(), command.length());
	write(sock, command.data(), command.length());

	return(ProcessHeader());
}

int ProxyFtp::ProcessHeader()
{
  char buffer[1024];
  int len = 1;
  
  size = 0xFFFFFFF;
  
  while( len && fgets( buffer, 1024, fsocket ) )
    {
      len = strlen(buffer);
      while( len && (buffer[len-1] == '\n' || buffer[len-1] == '\r'))
	buffer[--len] = 0;
      
      if ( strncmp( buffer, "Content-length: ", 16 ) == 0 
	   ||   strncmp( buffer, "Content-Length: ", 16 ) == 0 )
	size = atol( buffer + 16 );
      else if ( strncmp( buffer, "Content-Type: ", 14 ) == 0 
		||   strncmp( buffer, "Content-type: ", 14 ) == 0 )
	{
	  emit mimeType( buffer + 14 );
	}
      else if ( strncmp( buffer, "HTTP/1.0 ", 9 ) == 0 )
	{
	  if ( (buffer[9] == '4') || (buffer[9] == '5'))
	    {
	      KURL u( url );
	      QString path = u.path();
	      /* When the requested URL is not a Directory and it 
		 was the first attempt (secondtry==0) delete the 
		 trailing "/" from URL and try again
	      */
	      if ( (strcmp(path.right(1),"/") == 0)
		   && (path.length() != 1 ) 
		   && !secondtry ) {
		secondtry = 1;
		Close();
		path.truncate(path.length() - 1);
		KURL u2( u, path.data());
		return Open( &u2, currentMode );
	      } else { // A real error occured
		if ( (buffer[9] == '4')) {
		  Close();
		  Error(KIO_ERROR_CouldNotRead,buffer+9,errno);
		  return FAIL;
		}
	      }
	    }
	}      
      else if ( strncmp( buffer, "Location: ", 10 ) == 0 )
	{
	  Close();
	  KURL u( url );
	  KURL u2( u, buffer + 10 );
	  emit redirection( u2.url() );
	  return Open( &u2, currentMode );
	}
      
    }
  bytesleft = size;
  return(SUCCESS);
}
#endif

QString ProxyFtp::proxyProtocol()
{
    return QString::fromLatin1("ftp://");
}

