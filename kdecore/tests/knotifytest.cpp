#include <knotifyclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

int main( int argc, char **argv )
{
    KAboutData about("knotifytest", "knotifytest", "version");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication app;

    KNotifyClient::userEvent( "This is a notification to notify you :)", 
                              KNotifyClient::Messagebox,
                              KNotifyClient::Error );
}
