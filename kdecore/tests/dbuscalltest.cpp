#include <QDebug>
#include <QCoreApplication>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

int main( int argc, char** argv )
{
    QCoreApplication app(argc, argv);

    QDBusConnectionInterface *bus = 0;
    if (!QDBusConnection::sessionBus().isConnected() || !(bus = QDBusConnection::sessionBus().interface())) {
        qCritical() << "Session bus not found";
        return 125;
    }

    qDebug() << "sending reparseConfiguration to object Konqueror in konqueror";
    QDBusMessage message = QDBusMessage::createSignal("/Konqueror", "org.kde.Konqueror", "reparseConfiguration");
    if (!QDBusConnection::sessionBus().send(message))
        qDebug() << "void expected, " << QDBusConnection::sessionBus().lastError().name() << " returned";

    return 0;
}
