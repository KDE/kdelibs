extern "C" 
{
  #include "kurl.parse.h"

  void kurl_mainParse( char *_code );  
};

#include "kurl.h"

#include <assert.h>
#include <stdlib.h>

KURL* g_pKURL = 0L;
bool g_error = false;

bool kurl_parse( KURL *_url, const char *_txt )
{
  g_pKURL = _url;
  g_error = false;

  kurl_mainParse( (char*)_txt );

  return !g_error;
}

void kurl_setError()
{
  g_error = true;
}

// HACK decode stuff here!

void kurl_setProtocol( char* _txt )
{
  assert( g_pKURL );
  
  g_pKURL->setProtocol( _txt );
} 

void kurl_setUser( char * _txt )
{
  assert( g_pKURL );

  g_pKURL->setUser( _txt );
}

void kurl_setPass( char* _txt )
{
  assert( g_pKURL );

  g_pKURL->setPass( _txt );
}

void kurl_setHost( char* _txt )
{
  assert( g_pKURL );

  g_pKURL->setHost( _txt );
}

void kurl_setPort( char* _txt )
{
  assert( g_pKURL );

  g_pKURL->setPort( atoi( _txt ) );
}

void kurl_setPath( char* _txt )
{
  assert( g_pKURL );

  g_pKURL->setEncodedPathAndQuery( _txt );
}

void kurl_setRef( char* _txt )
{
  assert( g_pKURL );

  g_pKURL->setRef( _txt );
}

