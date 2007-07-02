#include <QtCore/QTextStream>
#include <QtGui/QApplication>
#include <QtCore/QTimer>

#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kwallet.h>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusReply>

static QTextStream _out( stdout, QIODevice::WriteOnly );

void openWallet()
{
	_out << "About to ask for wallet sync" << endl;

	KWallet::Wallet *w = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(), 0, KWallet::Wallet::Synchronous );

	_out << "Got sync wallet: " << (w != 0) << endl;
}

int main( int argc, char *argv[] )
{
	KAboutData aboutData("kwalletsync", 0, ki18n("kwalletsync"), "version");
	KComponentData componentData(&aboutData);
	QApplication app( argc, argv );

	// force name with D-BUS
        QDBusReply<QDBusConnectionInterface::RegisterServiceReply> reply
            = QDBusConnection::sessionBus().interface()->registerService( "org.kde.kwalletsync",
                                                                QDBusConnectionInterface::ReplaceExistingService );

        if ( !reply.isValid() )
        {
                _out << "D-BUS name request returned " << reply.error().name() << endl;
        }

	openWallet();

	return 0;
}

// vim: set noet ts=4 sts=4 sw=4:

