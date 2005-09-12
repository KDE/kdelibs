#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <dcopclient.h>

int main( int argc, char** argv )
{
   KAboutData about("KIDLTestClient", "KIDLTestClient", "version");
   KCmdLineArgs::init(argc, argv, &about);
   KApplication app(false);

    KApplication::dcopClient()->attach();
    // KApplication::dcopClient()->registerAs( "kidlclienttest" );

    QByteArray data;

    kdDebug() << "sending reparseConfiguration to object KonquerorIface in konqueror" << endl;
       QByteArray snd;
       QByteArray rcv;
       DCOPCString _type_;
       KApplication::dcopClient()->call( "konqueror", "KonquerorIface", "reparseConfiguration()", snd, _type_, rcv );
       kdDebug() << _type_ << endl;
       if( _type_ != "void" ) kdDebug() << "void expected, " << _type_.data() << " returned" << endl;

/*
debug("sending configure to object KonquerorIface in konqueror");
    if (KApplication::dcopClient()->send( "konqueror", "KonquerorIface", "configure()", data ))
      debug("ok");
    else
      debug("ko");

/// BROADCAST
debug("sending databaseChanged to object ksycoca in all apps");
    if (KApplication::dcopClient()->send( "*", "ksycoca", "databaseChanged()", data ))
      debug("ok");
    else
      debug("ko");
*/
}
