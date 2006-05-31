#include <qtextstream.h>
#include <qtimer.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kwallet.h>
#include <dbus/qdbusbus.h>
#include <dbus/qdbusconnection.h>
#include <dbus/qdbusreply.h>

static QTextStream _out( stdout, QIODevice::WriteOnly );

void openWallet()
{
	_out << "About to ask for wallet sync" << endl;

	KWallet::Wallet *w = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(), 0, KWallet::Wallet::Synchronous );

	_out << "Got sync wallet: " << (w != 0) << endl;
}

int main( int argc, char *argv[] )
{
	KAboutData aboutData( "kwalletsync", "kwalletsync", "version" );
	KCmdLineArgs::init( argc, argv, &aboutData );
	KApplication app( "kwalletsync" );

	// force name with D-BUS
        QDBusReply<QDBusBusService::RequestNameReply> reply
            = QDBus::sessionBus().busService()->requestName( app.objectName(),
                                                             QDBusBusService::ReplaceExistingName );

        if ( reply.isError() )
        {
                _out << "D-BUS name request returned " << reply.error().name() << endl;
        }

	openWallet();

	return 0;
}

// vim: set noet ts=4 sts=4 sw=4:

