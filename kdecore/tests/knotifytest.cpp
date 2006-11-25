#include <knotifyclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kinstance.h>

int main( int argc, char **argv )
{
    KAboutData about("knotifytest", "knotifytest", "version");
    //KCmdLineArgs::init(argc, argv, &about);
    //KApplication app;
    KInstance instance(&about);

    KNotifyClient::userEvent( "This is a notification to notify you :)",
                              KNotifyClient::Messagebox,
                              KNotifyClient::Error );
}
