// File: ftp_proxy.cpp by Martin Zumkley (uc2n@rzstud1.rz.uni-karlsruhe.de)

#ifndef _ftp_proxy_h
#define _ftp_proxy_h

#include "../http/http.h" // HACK

class ProxyFtp :public HTTPProtocol
{
 public:
    ProxyFtp( const QCString &protocol, const QCString &pool, const QCString & app);
    virtual ~ProxyFtp();

    // Slave interface ... is the same as HTTPProtocol's

    //virtual void get( const QString& path, const QString& query, bool reload );

    // Overloaded from HTTPProtocol (need to create it there)
    virtual QString proxyProtocol();

protected:
    int secondtry;
    //virtual int OpenProxy(KURL *url, int mode, bool reload);

/*
    virtual bool isHTML(){
	    if ( strcmp(url.right(1),"/") == 0 )
		return 1;
	    else
		return 0;
	};
*/
};

#endif
