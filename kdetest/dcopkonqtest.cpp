#include <kapp.h>
#include <dcopclient.h>

int main( int argc, char** argv )
{
    KApplication app( argc, argv, "KIDLTestClient", false );

    kapp->dcopClient()->attach();
    // kapp->dcopClient()->registerAs( "kidlclienttest" );

    QByteArray data;

/*
debug("sending configure to object KonquerorIface in konqueror");
       QByteArray snd;
       QByteArray rcv;
       QCString _type_;
       kapp->dcopClient()->call( "konqueror", "KonquerorIface", "configure()", snd, _type_, rcv );
       debug(_type_);
       if( _type_ != "void" ) debug("void expected, %s returned",_type_.data());

debug("sending configure to object KonquerorIface in konqueror");
    if (kapp->dcopClient()->send( "konqueror", "KonquerorIface", "configure()", data ))
      debug("ok");
    else
      debug("ko");
*/

debug("sending databaseChanged to object ksycoca in kdesktop");
    if (kapp->dcopClient()->send( "kdesktop", "ksycoca", "databaseChanged()", data ))
      debug("ok");
    else
      debug("ko");

/// BROADCAST (currently broken)
debug("sending databaseChanged to object ksycoca in all apps");
    if (kapp->dcopClient()->send( "*", "ksycoca", "databaseChanged()", data ))
      debug("ok");
    else
      debug("ko");
}
