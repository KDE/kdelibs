#include <kapp.h>
#include <dcopclient.h>

int main( int argc, char** argv )
{
    KApplication app( argc, argv, "KIDLTestClient", false );

    kapp->dcopClient()->attach();
    // kapp->dcopClient()->registerAs( "kidlclienttest" );

debug("sending configure to object KonquerorIface in konqueror");
       QByteArray snd;
       QByteArray rcv;
       QCString _type_;
       kapp->dcopClient()->call( "konqueror", "KonquerorIface", "configure()", snd, _type_, rcv );
       debug(_type_);
       if( _type_ != "void" ) debug("void expected, %s returned",_type_.data());

    QByteArray data;
debug("sending configure to object KonquerorIface in konqueror");
    if (kapp->dcopClient()->send( "konqueror", "KonquerorIface", "configure()", data ))
      debug("ok");
    else
      debug("ko");

debug("sending databaseChanged to object ksycoca in konqueror");
    if (kapp->dcopClient()->send( "konqueror", "ksycoca", "databaseChanged()", data ))
      debug("ok");
    else
      debug("ko");

}
