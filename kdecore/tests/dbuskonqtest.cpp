#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <qdbusinterface.h>
#include <qdbusreply.h>

int main( int argc, char** argv )
{
   KAboutData about("DBusKonqTest", "DBusKonqTest", "version");
   KCmdLineArgs::init(argc, argv, &about);
   KApplication app(false);

    kDebug() << "sending reparseConfiguration to object Konqueror in konqueror" << endl;
    QDBusMessage message = QDBusMessage::signal("/Konqueror", "org.kde.Konqueror", "reparseConfiguration", QDBus::sessionBus());
    if(!QDBus::sessionBus().send(message))
		kDebug() << "void expected, " << QDBus::sessionBus().lastError().name() << " returned" << endl;

    /*
       QByteArray snd;
       QByteArray rcv;
       DCOPCString _type_;
       KApplication::dcopClient()->call( "konqueror", "KonquerorIface", "reparseConfiguration()", snd, _type_, rcv );
       kDebug() << _type_ << endl;
       if( _type_ != "void" ) kDebug() << "void expected, " << _type_.data() << " returned" << endl;
    */
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
