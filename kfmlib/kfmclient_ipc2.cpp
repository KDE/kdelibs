// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de
#include "kfmclient_ipc.h"

void KfmIpc::refreshDesktop()
{
	int len = 0;
	len += len_string("refreshDesktop");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "refreshDesktop" );
}

void KfmIpc::refreshDirectory(const char* _url)
{
	int len = 0;
	len += len_string( _url );
	len += len_string("refreshDirectory");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "refreshDirectory" );
	write_string( sock->socket(), _url );
}

void KfmIpc::openURL(const char* _url)
{
	int len = 0;
	len += len_string( _url );
	len += len_string("openURL");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "openURL" );
	write_string( sock->socket(), _url );
}

void KfmIpc::openProperties(const char* _url)
{
	int len = 0;
	len += len_string( _url );
	len += len_string("openProperties");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "openProperties" );
	write_string( sock->socket(), _url );
}

void KfmIpc::exec(const char* _url, const char* _binding)
{
	int len = 0;
	len += len_string( _url );
	len += len_string( _binding );
	len += len_string("exec");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "exec" );
	write_string( sock->socket(), _url );
	write_string( sock->socket(), _binding );
}

void KfmIpc::copy(const char* _src, const char* _dest)
{
	int len = 0;
	len += len_string( _src );
	len += len_string( _dest );
	len += len_string("copy");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "copy" );
	write_string( sock->socket(), _src );
	write_string( sock->socket(), _dest );
}

void KfmIpc::move(const char* _src, const char* _dest)
{
	int len = 0;
	len += len_string( _src );
	len += len_string( _dest );
	len += len_string("move");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "move" );
	write_string( sock->socket(), _src );
	write_string( sock->socket(), _dest );
}

void KfmIpc::moveClient(const char* _src, const char* _dest)
{
	int len = 0;
	len += len_string( _src );
	len += len_string( _dest );
	len += len_string("moveClient");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "moveClient" );
	write_string( sock->socket(), _src );
	write_string( sock->socket(), _dest );
}

void KfmIpc::copyClient(const char* _src, const char* _dest)
{
	int len = 0;
	len += len_string( _src );
	len += len_string( _dest );
	len += len_string("copyClient");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "copyClient" );
	write_string( sock->socket(), _src );
	write_string( sock->socket(), _dest );
}

void KfmIpc::sortDesktop()
{
	int len = 0;
	len += len_string("sortDesktop");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "sortDesktop" );
}

void KfmIpc::auth(const char* _password)
{
	int len = 0;
	len += len_string( _password );
	len += len_string("auth");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "auth" );
	write_string( sock->socket(), _password );
}

void KfmIpc::selectRootIcons(int _x, int _y, int _w, int _h, bool _add)
{
	int len = 0;
	len += len_int( _x );
	len += len_int( _y );
	len += len_int( _w );
	len += len_int( _h );
	len += len_bool( _add );
	len += len_string("selectRootIcons");
	write_int( sock->socket(), len );
	write_string( sock->socket(), "selectRootIcons" );
	write_int( sock->socket(), _x );
	write_int( sock->socket(), _y );
	write_int( sock->socket(), _w );
	write_int( sock->socket(), _h );
	write_bool( sock->socket(), _add );
}

void KfmIpc::parse_finished( char *_data, int _len )
{
	int pos = 0;

	// Calling function
	emit finished(  );
}

