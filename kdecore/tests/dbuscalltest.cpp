#include <kcmdlineargs.h>
#include <kaboutdata.h>
//#include <kapplication.h>
#include <kdebug.h>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

int main( int argc, char** argv )
{
    KAboutData about("DBusCallTest", 0, ki18n("DBusCallTest"), "version");
    KCmdLineArgs::init(argc, argv, &about);
//   KApplication app(false);

    QDBusConnectionInterface *bus = 0;
    if (!QDBusConnection::sessionBus().isConnected() || !(bus = QDBusConnection::sessionBus().interface())) {
        kFatal(101) << "Session bus not found";
        return 125;
    }

    kDebug() << "sending reparseConfiguration to object Konqueror in konqueror";
    QDBusMessage message = QDBusMessage::createSignal("/Konqueror", "org.kde.Konqueror", "reparseConfiguration");
    if(!QDBusConnection::sessionBus().send(message))
        kDebug() << "void expected, " << QDBusConnection::sessionBus().lastError().name() << " returned";

    return 0;
}
