#ifndef __parse_h__
#define __parse_h__

void kurl_setProtocol( char* );
void kurl_setUser( char * );
void kurl_setPass( char* );
void kurl_setHost( char* );
void kurl_setPort( char* );
void kurl_setPath( char* );
void kurl_setRef( char* );

void kurl_setError();

#endif
