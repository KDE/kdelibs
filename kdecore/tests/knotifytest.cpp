#include <knotifyclient.h>
#include <kapp.h>

int main( int argc, char **argv )
{
	KApplication app( argc, argv, "knotifytest" );
    KNotifyClient::userEvent( "This is a notification to notify you :)", 
                              KNotifyClient::Messagebox,
                              KNotifyClient::Error );
}
